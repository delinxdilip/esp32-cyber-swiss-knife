idf_component_register(
    SRCS
        "main.cpp"

        # ====================================================
        # SYSTEM
        # ====================================================
        "core/system/system_manager.cpp"
        "core/system/system_info.cpp"

        # ====================================================
        # LOGGING
        # ====================================================
        "core/logging/logger.cpp"
        "core/logging/log_manager.cpp"

        # ====================================================
        # CONFIGURATION
        # ====================================================
        "core/config/config_manager.cpp"

        # ====================================================
        # JSON
        # ====================================================
        "core/json/json_builder.cpp"
        "core/json/json_parser.cpp"

        # ====================================================
        # MONITORING
        # ====================================================
        "core/monitoring/temperature/temperature_monitor.cpp"
        "core/monitoring/uptime/uptime_monitor.cpp"

        # ====================================================
        # WIFI
        # ====================================================
        "wifi/wifi_manager/wifi_manager.cpp"
        "wifi/scanner/wifi_scanner.cpp"
        "wifi/analyzer/wifi_analyzer.cpp"
        "wifi/data/wifi_data.cpp"

        # ====================================================
        # NETWORK
        # ====================================================
        "network/ap/ap_manager/ap_manager.cpp"
        "network/webserver/web_server.cpp"

        # ====================================================
        # UI
        # ====================================================
        "ui/display/gc9a01_display.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp_driver_tsens
        esp_timer
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        nvs_flash
        esp_lcd
)
