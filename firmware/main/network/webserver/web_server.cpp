#include "web_server.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

#include "core/config/config_manager.h"
#include "core/json/json_builder.h"
#include "core/json/json_parser.h"
#include "core/logging/logger.h"
#include "core/system/system_info.h"

#include "network/ap/ap_manager/ap_manager.h"

#include "wifi/data/wifi_data.h"
#include "wifi/scanner/wifi_scanner.h"

namespace
{
    constexpr const char *FILESYSTEM_BASE = "/web";
    constexpr size_t MAX_REQUEST_BODY = 512;
    constexpr size_t MAX_PATH_LENGTH = 256;
    constexpr size_t SYSTEM_RESPONSE_SIZE = 8192;

    httpd_handle_t server = nullptr;
    bool spiffs_initialized = false;

    const char *get_content_type(
        const char *path)
    {
        const char *extension =
            std::strrchr(path, '.');

        if (extension == nullptr)
        {
            return "text/plain";
        }

        if (std::strcmp(extension, ".html") == 0)
        {
            return "text/html";
        }

        if (std::strcmp(extension, ".css") == 0)
        {
            return "text/css";
        }

        if (std::strcmp(extension, ".js") == 0)
        {
            return "application/javascript";
        }

        if (std::strcmp(extension, ".json") == 0)
        {
            return "application/json";
        }

        if (std::strcmp(extension, ".svg") == 0)
        {
            return "image/svg+xml";
        }

        if (std::strcmp(extension, ".png") == 0)
        {
            return "image/png";
        }

        if (std::strcmp(extension, ".jpg") == 0 ||
            std::strcmp(extension, ".jpeg") == 0)
        {
            return "image/jpeg";
        }

        if (std::strcmp(extension, ".ico") == 0)
        {
            return "image/x-icon";
        }

        if (std::strcmp(extension, ".txt") == 0)
        {
            return "text/plain";
        }

        return "application/octet-stream";
    }

    esp_err_t send_json_error(
        httpd_req_t *request,
        const char *status,
        const char *message)
    {
        httpd_resp_set_status(
            request,
            status);

        httpd_resp_set_type(
            request,
            "application/json");

        char response[256];

        std::snprintf(
            response,
            sizeof(response),
            "{\"error\":\"%s\"}",
            message != nullptr
                ? message
                : "Unknown error");

        return httpd_resp_sendstr(
            request,
            response);
    }

    esp_err_t get_config_handler(
        httpd_req_t *request)
    {
        const APConfig &config =
            ConfigManager::get_ap_config();

        char response[512];

        JsonBuilder json(
            response,
            sizeof(response));

        if (!json.begin_object())
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Failed to create response");
        }

        json.add_string(
            "ssid",
            config.ssid);

        json.add_uint(
            "max_connections",
            config.max_connections);

        json.add_uint(
            "channel",
            config.channel);

        json.end_object();

        if (!json.valid())
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Configuration response too large");
        }

        httpd_resp_set_type(
            request,
            "application/json");

        return httpd_resp_send(
            request,
            json.data(),
            json.size());
    }

    esp_err_t post_config_handler(
        httpd_req_t *request)
    {
        if (request->content_len == 0 ||
            request->content_len > MAX_REQUEST_BODY)
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Invalid request body size");
        }

        char body[MAX_REQUEST_BODY + 1];

        size_t received = 0;

        while (received < request->content_len)
        {
            const int result =
                httpd_req_recv(
                    request,
                    body + received,
                    request->content_len - received);

            if (result <= 0)
            {
                if (result == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    continue;
                }

                return send_json_error(
                    request,
                    "400 Bad Request",
                    "Failed to receive request body");
            }

            received += static_cast<size_t>(result);
        }

        body[received] = '\0';

        JsonParser parser;

        if (!parser.parse(body, received))
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Invalid JSON");
        }

        APConfig new_config =
            ConfigManager::get_ap_config();

        char ssid[33];

        if (!parser.get_string(
                "ssid",
                ssid,
                sizeof(ssid)))
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "SSID is required");
        }

        std::strncpy(
            new_config.ssid,
            ssid,
            sizeof(new_config.ssid) - 1);

        new_config.ssid[
            sizeof(new_config.ssid) - 1] =
            '\0';

        uint32_t max_connections = 0;

        if (!parser.get_uint(
                "max_connections",
                max_connections))
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Maximum connections is required");
        }

        new_config.max_connections =
            static_cast<uint8_t>(
                max_connections);

        uint32_t channel = 0;

        if (!parser.get_uint(
                "channel",
                channel))
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Channel is required");
        }

        new_config.channel =
            static_cast<uint8_t>(
                channel);

        char password[65];

        if (parser.get_string(
                "password",
                password,
                sizeof(password)))
        {
            if (password[0] != '\0')
            {
                std::strncpy(
                    new_config.password,
                    password,
                    sizeof(new_config.password) - 1);

                new_config.password[
                    sizeof(new_config.password) - 1] =
                    '\0';
            }
        }

        if (!ConfigManager::validate(
                new_config))
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Invalid configuration");
        }

        const APConfig old_config =
            ConfigManager::get_ap_config();

        if (!APManager::apply_config(
                new_config))
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Failed to apply access point configuration");
        }

        if (!ConfigManager::set_ap_config(
                new_config))
        {
            APManager::apply_config(
                old_config);

            return send_json_error(
                request,
                "500 Internal Server Error",
                "Failed to save configuration");
        }

        httpd_resp_set_type(
            request,
            "application/json");

        return httpd_resp_sendstr(
            request,
            "{\"success\":true}");
    }

    esp_err_t get_wifi_handler(
        httpd_req_t *request)
    {
        char response[1024];

        JsonBuilder json(
            response,
            sizeof(response));

        WiFiSnapshot snapshot;

        if (!WiFiData::get_snapshot(snapshot))
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Failed to collect Wi-Fi information");
        }

        json.begin_object();

        json.add_uint(
            "network_count",
            snapshot.network_count);

        json.add_int(
            "strongest_rssi",
            snapshot.strongest_rssi);

        json.add_int(
            "weakest_rssi",
            snapshot.weakest_rssi);

        json.add_uint(
            "open_network_count",
            snapshot.open_network_count);

        json.add_uint(
            "hidden_network_count",
            snapshot.hidden_network_count);

        json.end_object();

        if (!json.valid())
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Wi-Fi response too large");
        }

        httpd_resp_set_type(
            request,
            "application/json");

        return httpd_resp_send(
            request,
            json.data(),
            json.size());
    }

    esp_err_t get_wifi_networks_handler(
        httpd_req_t *request)
    {
        char *response =
            static_cast<char *>(
                std::malloc(8192));

        if (response == nullptr)
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Not enough memory for Wi-Fi network response");
        }

        JsonBuilder json(
            response,
            8192);

        json.begin_object();

        json.begin_array(
            "networks");

        const uint16_t count =
            WiFiData::get_network_count();

        for (uint16_t index = 0;
             index < count;
             ++index)
        {
            const WiFiNetwork *network =
                WiFiData::get_network(index);

            if (network == nullptr)
            {
                continue;
            }

            json.begin_object();

            json.add_string(
                "ssid",
                network->ssid);

            char bssid[18];

            std::snprintf(
                bssid,
                sizeof(bssid),
                "%02X:%02X:%02X:%02X:%02X:%02X",
                network->bssid[0],
                network->bssid[1],
                network->bssid[2],
                network->bssid[3],
                network->bssid[4],
                network->bssid[5]);

            json.add_string(
                "bssid",
                bssid);

            json.add_int(
                "rssi",
                network->rssi);

            json.add_uint(
                "channel",
                network->channel);

            json.add_uint(
                "hidden",
                network->hidden ? 1 : 0);

            json.end_object();

            if (!json.valid())
            {
                break;
            }
        }

        json.end_array();

        json.end_object();

        if (!json.valid())
        {
            std::free(response);

            return send_json_error(
                request,
                "500 Internal Server Error",
                "Wi-Fi network response too large");
        }

        httpd_resp_set_type(
            request,
            "application/json");

        const esp_err_t result =
            httpd_resp_send(
                request,
                json.data(),
                json.size());

        std::free(response);

        return result;
    }

    esp_err_t scan_wifi_handler(
        httpd_req_t *request)
    {
        if (WiFiScanner::is_scanning())
        {
            return send_json_error(
                request,
                "409 Conflict",
                "Wi-Fi scan already in progress");
        }

        if (!WiFiScanner::scan())
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Wi-Fi scan failed");
        }

        return get_wifi_networks_handler(
            request);
    }

    esp_err_t get_system_handler(
        httpd_req_t *request)
    {
        SystemInfoData info{};

        if (!SystemInfo::get(info))
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Failed to collect system information");
        }

        /*
         * Keep the large JSON response off the HTTP
         * server task stack.
         *
         * The HTTP server task stack is configured to
         * 8192 bytes. SystemInfoData plus an 8192-byte
         * local JSON buffer can exhaust/corrupt that stack.
         *
         * Allocate the response buffer from heap instead.
         */
        char *response =
            static_cast<char *>(
                std::malloc(
                    SYSTEM_RESPONSE_SIZE));

        if (response == nullptr)
        {
            return send_json_error(
                request,
                "500 Internal Server Error",
                "Not enough memory for system response");
        }

        JsonBuilder json(
            response,
            SYSTEM_RESPONSE_SIZE);

        char temperature[32];

        std::snprintf(
            temperature,
            sizeof(temperature),
            "%.2f",
            static_cast<double>(
                info.temperature_celsius));

        char cpu_load[32];

        std::snprintf(
            cpu_load,
            sizeof(cpu_load),
            "%.1f",
            static_cast<double>(
                info.cpu_load_percent));

        bool success = true;

        success =
            success &&
            json.begin_object();

        success =
            success &&
            json.begin_object(
                "device");

        success =
            success &&
            json.add_string(
                "name",
                info.device_name);

        success =
            success &&
            json.add_string(
                "chip",
                info.chip_name);

        success =
            success &&
            json.add_uint(
                "chip_revision",
                info.chip_revision);

        success =
            success &&
            json.add_uint(
                "cpu_cores",
                info.cpu_cores);

        success =
            success &&
            json.add_uint(
                "cpu_frequency_mhz",
                info.cpu_frequency_mhz);

        success =
            success &&
            json.add_string(
                "esp_idf_version",
                info.esp_idf_version);

        success =
            success &&
            json.add_string(
                "firmware_version",
                info.firmware_version);

        success =
            success &&
            json.add_string(
                "build_date",
                info.build_date);

        success =
            success &&
            json.add_uint(
                "uptime_seconds",
                static_cast<uint32_t>(
                    info.uptime_seconds));

        success =
            success &&
            json.add_string(
                "reset_reason",
                info.reset_reason);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.begin_object(
                "memory");

        success =
            success &&
            json.add_uint(
                "free_internal_ram",
                info.free_internal_ram);

        success =
            success &&
            json.add_uint(
                "total_internal_ram",
                info.total_internal_ram);

        success =
            success &&
            json.add_uint(
                "used_internal_ram",
                info.used_internal_ram);

        success =
            success &&
            json.add_uint(
                "minimum_free_heap",
                info.minimum_free_heap);

        success =
            success &&
            json.add_uint(
                "psram_total",
                info.psram_total);

        success =
            success &&
            json.add_uint(
                "psram_free",
                info.psram_free);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.begin_object(
                "flash");

        success =
            success &&
            json.add_uint(
                "size",
                info.flash_size);

        success =
            success &&
            json.add_uint(
                "speed_mhz",
                info.flash_speed_mhz);

        success =
            success &&
            json.add_string(
                "mode",
                info.flash_mode);

        success =
            success &&
            json.add_uint(
                "application_partition",
                info.application_partition_size);

        success =
            success &&
            json.add_uint(
                "spiffs_partition",
                info.spiffs_partition_size);

        success =
            success &&
            json.add_uint(
                "spiffs_used",
                info.spiffs_used);

        success =
            success &&
            json.add_uint(
                "spiffs_free",
                info.spiffs_free);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.begin_object(
                "health");

        success =
            success &&
            json.add_raw(
                "temperature_celsius",
                temperature);

        success =
            success &&
            json.add_uint(
                "cpu_cores",
                info.hardware_cpu_cores);

        success =
            success &&
            json.add_uint(
                "chip_revision",
                info.hardware_chip_revision);

        success =
            success &&
            json.add_string(
                "wifi_state",
                info.wifi_state);

        success =
            success &&
            json.add_string(
                "bluetooth_state",
                info.bluetooth_state);

        success =
            success &&
            json.add_string(
                "ap_state",
                info.ap_state);

        success =
            success &&
            json.add_uint(
                "connected_clients",
                info.connected_clients);

        success =
            success &&
            json.add_raw(
                "cpu_load_percent",
                cpu_load);

        success =
            success &&
            json.add_uint(
                "task_count",
                info.task_count);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.begin_object(
                "network");

        success =
            success &&
            json.add_string(
                "ap_status",
                info.ap_status);

        success =
            success &&
            json.add_string(
                "ssid",
                info.ap_ssid);

        success =
            success &&
            json.add_string(
                "ip_address",
                info.ap_ip_address);

        success =
            success &&
            json.add_uint(
                "channel",
                info.ap_channel);

        success =
            success &&
            json.add_uint(
                "clients",
                info.ap_clients);

        success =
            success &&
            json.add_string(
                "mac_address",
                info.mac_address);

        success =
            success &&
            json.add_string(
                "wifi_status",
                info.wifi_status);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.begin_object(
                "storage");

        success =
            success &&
            json.add_uint(
                "total",
                info.storage_total);

        success =
            success &&
            json.add_uint(
                "used",
                info.storage_used);

        success =
            success &&
            json.add_uint(
                "free",
                info.storage_free);

        success =
            success &&
            json.add_uint(
                "files",
                info.storage_files);

        success =
            success &&
            json.add_uint(
                "logs",
                info.logs_bytes);

        success =
            success &&
            json.add_uint(
                "web_ui",
                info.web_ui_bytes);

        success =
            success &&
            json.add_uint(
                "configuration",
                info.configuration_bytes);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.begin_object(
                "firmware");

        success =
            success &&
            json.add_string(
                "name",
                info.firmware_name);

        success =
            success &&
            json.add_string(
                "version",
                info.firmware_version_detail);

        success =
            success &&
            json.add_string(
                "build_date",
                info.firmware_build_date);

        success =
            success &&
            json.add_string(
                "build_time",
                info.firmware_build_time);

        success =
            success &&
            json.add_string(
                "target",
                info.firmware_target);

        success =
            success &&
            json.add_string(
                "framework",
                info.firmware_framework);

        success =
            success &&
            json.add_string(
                "idf_version",
                info.esp_idf_version);

        success =
            success &&
            json.end_object();

        success =
            success &&
            json.end_object();

        if (!success || !json.valid())
        {
            std::free(response);

            return send_json_error(
                request,
                "500 Internal Server Error",
                "System information response too large");
        }

        httpd_resp_set_type(
            request,
            "application/json");

        /*
         * Keep the heap buffer alive until
         * httpd_resp_send() has finished using it.
         */
        const esp_err_t result =
            httpd_resp_send(
                request,
                json.data(),
                json.size());

        std::free(response);

        return result;
    }

    esp_err_t static_file_handler(
        httpd_req_t *request)
    {
        char path[MAX_PATH_LENGTH];

        const char *uri =
            request->uri;

        if (uri == nullptr)
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Invalid URI");
        }

        if (std::strstr(uri, "..") != nullptr)
        {
            return send_json_error(
                request,
                "400 Bad Request",
                "Invalid path");
        }

        if (std::strcmp(uri, "/") == 0)
        {
            uri = "/pages/index.html";
        }

        if (std::strlen(uri) >=
            MAX_PATH_LENGTH - 6)
        {
            return send_json_error(
                request,
                "414 URI Too Long",
                "Path too long");
        }

        int written = 0;

        if (std::strncmp(
                uri,
                "/pages/",
                7) == 0 ||
            std::strncmp(
                uri,
                "/css/",
                5) == 0 ||
            std::strncmp(
                uri,
                "/js/",
                4) == 0)
        {
            written =
                std::snprintf(
                    path,
                    sizeof(path),
                    "%s%s",
                    FILESYSTEM_BASE,
                    uri);
        }
        else
        {
            written =
                std::snprintf(
                    path,
                    sizeof(path),
                    "%s/pages%s",
                    FILESYSTEM_BASE,
                    uri);
        }

        if (written < 0 ||
            static_cast<size_t>(written) >=
                sizeof(path))
        {
            return send_json_error(
                request,
                "414 URI Too Long",
                "Path too long");
        }

        LOG_DEBUG(
            WEB,
            "Static request: %s -> %s",
            request->uri,
            path);

        FILE *file =
            std::fopen(
                path,
                "rb");

        if (file == nullptr)
        {
            httpd_resp_set_status(
                request,
                "404 Not Found");

            return httpd_resp_sendstr(
                request,
                "404 Not Found");
        }

        httpd_resp_set_type(
            request,
            get_content_type(path));

        char buffer[1024];

        while (true)
        {
            const size_t read =
                std::fread(
                    buffer,
                    1,
                    sizeof(buffer),
                    file);

            if (read > 0)
            {
                const esp_err_t result =
                    httpd_resp_send_chunk(
                        request,
                        buffer,
                        read);

                if (result != ESP_OK)
                {
                    std::fclose(file);

                    return result;
                }
            }

            if (read < sizeof(buffer))
            {
                break;
            }
        }

        std::fclose(file);

        return httpd_resp_send_chunk(
            request,
            nullptr,
            0);
    }

    esp_err_t not_found_handler(
        httpd_req_t *request,
        httpd_err_code_t error)
    {
        if (error == HTTPD_404_NOT_FOUND)
        {
            return static_file_handler(
                request);
        }

        return ESP_FAIL;
    }
}

bool WebServer::init()
{
    if (server != nullptr)
    {
        LOG_WARN(
            WEB,
            "Web server already initialized");

        return true;
    }

    if (!spiffs_initialized)
    {
        esp_vfs_spiffs_conf_t config =
        {
            .base_path = FILESYSTEM_BASE,
            .partition_label = nullptr,
            .max_files = 16,
            .format_if_mount_failed = false
        };

        const esp_err_t result =
            esp_vfs_spiffs_register(
                &config);

        if (result != ESP_OK)
        {
            LOG_ERROR(
                WEB,
                "SPIFFS mount failed: %s",
                esp_err_to_name(result));

            return false;
        }

        spiffs_initialized = true;

        size_t total = 0;
        size_t used = 0;

        if (esp_spiffs_info(
                nullptr,
                &total,
                &used) == ESP_OK)
        {
            LOG_INFO(
                WEB,
                "SPIFFS mounted: %u / %u bytes used",
                static_cast<unsigned int>(
                    used),
                static_cast<unsigned int>(
                    total));
        }
    }

    httpd_config_t config =
        HTTPD_DEFAULT_CONFIG();

    config.server_port = 80;
    config.max_uri_handlers = 16;
    config.stack_size = 8192;
    config.lru_purge_enable = true;

    /*
     * ESP-IDF 6.x does not provide uri_match_fn
     * inside httpd_uri_t.
     *
     * Wildcard matching belongs to the HTTP server
     * configuration instead.
     */
    config.uri_match_fn =
        httpd_uri_match_wildcard;

    const esp_err_t start_result =
        httpd_start(
            &server,
            &config);

    if (start_result != ESP_OK)
    {
        LOG_ERROR(
            WEB,
            "HTTP server start failed: %s",
            esp_err_to_name(start_result));

        server = nullptr;

        return false;
    }

    httpd_uri_t config_get =
    {
        .uri = "/api/config",
        .method = HTTP_GET,
        .handler = get_config_handler,
        .user_ctx = nullptr
    };

    httpd_register_uri_handler(
        server,
        &config_get);

    httpd_uri_t config_post =
    {
        .uri = "/api/config",
        .method = HTTP_POST,
        .handler = post_config_handler,
        .user_ctx = nullptr
    };

    httpd_register_uri_handler(
        server,
        &config_post);

    httpd_uri_t wifi_get =
    {
        .uri = "/api/wifi",
        .method = HTTP_GET,
        .handler = get_wifi_handler,
        .user_ctx = nullptr
    };

    httpd_register_uri_handler(
        server,
        &wifi_get);

    httpd_uri_t wifi_networks =
    {
        .uri = "/api/wifi/networks",
        .method = HTTP_GET,
        .handler = get_wifi_networks_handler,
        .user_ctx = nullptr
    };

    httpd_register_uri_handler(
        server,
        &wifi_networks);

    httpd_uri_t wifi_scan =
    {
        .uri = "/api/wifi/scan",
        .method = HTTP_GET,
        .handler = scan_wifi_handler,
        .user_ctx = nullptr
    };

    httpd_register_uri_handler(
        server,
        &wifi_scan);

    httpd_uri_t system_get =
    {
        .uri = "/api/system",
        .method = HTTP_GET,
        .handler = get_system_handler,
        .user_ctx = nullptr
    };

    httpd_register_uri_handler(
        server,
        &system_get);

    httpd_uri_t static_files =
    {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = nullptr
    };

    const esp_err_t static_result =
        httpd_register_uri_handler(
            server,
            &static_files);

    if (static_result != ESP_OK)
    {
        LOG_ERROR(
            WEB,
            "Failed to register static file handler: %s",
            esp_err_to_name(static_result));

        httpd_stop(server);

        server = nullptr;

        return false;
    }

    httpd_register_err_handler(
        server,
        HTTPD_404_NOT_FOUND,
        not_found_handler);

    LOG_INFO(
        WEB,
        "HTTP server started on port 80");

    return true;
}