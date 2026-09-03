#include "web_server.h"

#include "core/logging/logger.h"

#include "esp_err.h"
#include "esp_http_server.h"

namespace
{
    httpd_handle_t server = nullptr;

    constexpr char INDEX_HTML[] =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>CyberSwissKnife</title>"
        "</head>"
        "<body>"
        "<h1>CyberSwissKnife</h1>"
        "<p>ESP32 HTTP server is working.</p>"
        "<p>Device IP: 192.168.4.1</p>"
        "</body>"
        "</html>";

    esp_err_t handle_root(
        httpd_req_t *request)
    {
        LOG_INFO(
            WEB,
            "GET /");

        httpd_resp_set_type(
            request,
            "text/html");

        return httpd_resp_send(
            request,
            INDEX_HTML,
            HTTPD_RESP_USE_STRLEN);
    }
}

bool WebServer::init()
{
    if (server != nullptr)
    {
        LOG_DEBUG(
            WEB,
            "HTTP server already running");

        return true;
    }

    httpd_config_t config =
        HTTPD_DEFAULT_CONFIG();

    config.server_port = 80;

    esp_err_t err =
        httpd_start(
            &server,
            &config);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WEB,
            "Failed to start HTTP server: %s",
            esp_err_to_name(err));

        server = nullptr;

        return false;
    }

    httpd_uri_t root_uri = {};

    root_uri.uri = "/";
    root_uri.method = HTTP_GET;
    root_uri.handler = handle_root;
    root_uri.user_ctx = nullptr;

    err =
        httpd_register_uri_handler(
            server,
            &root_uri);

    if (err != ESP_OK)
    {
        LOG_ERROR(
            WEB,
            "Failed to register root URI: %s",
            esp_err_to_name(err));

        httpd_stop(server);

        server = nullptr;

        return false;
    }

    LOG_INFO(
        WEB,
        "HTTP server started");

    LOG_INFO(
        WEB,
        "Listening on port 80");

    LOG_INFO(
        WEB,
        "Open http://192.168.4.1");

    return true;
}