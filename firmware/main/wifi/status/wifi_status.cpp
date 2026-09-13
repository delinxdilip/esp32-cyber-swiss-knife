#include "wifi_status.h"

#include <cstring>
#include <cstdio>

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"

namespace
{
    constexpr char TAG[] = "WiFiStatus";

    WiFiStatus::Info status_info =
    {
        .enabled = false,
        .connected = false,

        .ssid = "",
        .ip_address = "",

        .rssi = 0,
        .channel = 0,

        .security = ""
    };

    void reset_connection_data()
    {
        status_info.connected = false;

        std::memset(status_info.ssid, 0, sizeof(status_info.ssid));
        std::memset(status_info.ip_address, 0, sizeof(status_info.ip_address));
        std::memset(status_info.security, 0, sizeof(status_info.security));

        status_info.rssi = 0;
        status_info.channel = 0;
    }

    const char *get_security_name(wifi_auth_mode_t auth_mode)
    {
        switch (auth_mode)
        {
            case WIFI_AUTH_OPEN:
                return "OPEN";

            case WIFI_AUTH_WEP:
                return "WEP";

            case WIFI_AUTH_WPA_PSK:
                return "WPA";

            case WIFI_AUTH_WPA2_PSK:
                return "WPA2";

            case WIFI_AUTH_WPA_WPA2_PSK:
                return "WPA/WPA2";

            case WIFI_AUTH_WPA2_ENTERPRISE:
                return "WPA2-ENT";

            case WIFI_AUTH_WPA3_PSK:
                return "WPA3";

            case WIFI_AUTH_WPA2_WPA3_PSK:
                return "WPA2/WPA3";

            case WIFI_AUTH_WAPI_PSK:
                return "WAPI";

            default:
                return "UNKNOWN";
        }
    }
}

namespace WiFiStatus
{
    void init()
    {
        status_info.enabled = false;
        reset_connection_data();
    }

    esp_err_t update()
    {
        wifi_mode_t wifi_mode = WIFI_MODE_NULL;

        esp_err_t mode_result = esp_wifi_get_mode(&wifi_mode);

        if (mode_result != ESP_OK)
        {
            status_info.enabled = false;
            reset_connection_data();

            return mode_result;
        }

        status_info.enabled =
            (wifi_mode == WIFI_MODE_STA) ||
            (wifi_mode == WIFI_MODE_APSTA);

        if (!status_info.enabled)
        {
            reset_connection_data();
            return ESP_OK;
        }

        wifi_ap_record_t ap_info;
        std::memset(&ap_info, 0, sizeof(ap_info));

        esp_err_t ap_info_result = esp_wifi_sta_get_ap_info(&ap_info);

        if (ap_info_result != ESP_OK)
        {
            reset_connection_data();

            /*
             * ESP_ERR_WIFI_NOT_CONNECT means the station is enabled
             * but is not currently connected to an access point.
             */
            if (ap_info_result == ESP_ERR_WIFI_NOT_CONNECT)
            {
                return ESP_OK;
            }

            ESP_LOGW(
                TAG,
                "Unable to get connected AP information: %s",
                esp_err_to_name(ap_info_result)
            );

            return ap_info_result;
        }

        status_info.connected = true;

        std::memcpy(
            status_info.ssid,
            ap_info.ssid,
            sizeof(ap_info.ssid)
        );

        status_info.ssid[sizeof(status_info.ssid) - 1] = '\0';

        status_info.rssi = ap_info.rssi;
        status_info.channel = ap_info.primary;

        std::snprintf(
            status_info.security,
            sizeof(status_info.security),
            "%s",
            get_security_name(ap_info.authmode)
        );

        /*
         * The default station interface is normally created using:
         *
         * esp_netif_create_default_wifi_sta();
         *
         * This function retrieves that interface.
         */
        esp_netif_t *sta_netif =
            esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

        if (sta_netif == nullptr)
        {
            std::memset(
                status_info.ip_address,
                0,
                sizeof(status_info.ip_address)
            );

            std::snprintf(
                status_info.ip_address,
                sizeof(status_info.ip_address),
                "--"
            );

            ESP_LOGW(
                TAG,
                "Station network interface was not found"
            );

            return ESP_OK;
        }

        esp_netif_ip_info_t ip_info;
        std::memset(&ip_info, 0, sizeof(ip_info));

        esp_err_t ip_result =
            esp_netif_get_ip_info(sta_netif, &ip_info);

        if (ip_result == ESP_OK)
        {
            esp_ip4addr_ntoa(
                &ip_info.ip,
                status_info.ip_address,
                sizeof(status_info.ip_address)
            );
        }
        else
        {
            std::snprintf(
                status_info.ip_address,
                sizeof(status_info.ip_address),
                "--"
            );

            ESP_LOGW(
                TAG,
                "Unable to get station IP address: %s",
                esp_err_to_name(ip_result)
            );
        }

        return ESP_OK;
    }

    const Info &get_info()
    {
        return status_info;
    }

    bool is_connected()
    {
        return status_info.connected;
    }
}