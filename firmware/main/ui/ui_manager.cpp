#include "ui_manager.h"

#include <ctime>
#include <cstdio>

#include "wifi/status/wifi_status.h"

#include "core/monitoring/temperature/temperature_monitor.h"
#include "core/logging/logger.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "hardware/input/joystick/joystick_manager.h"

#include "ui/display/display_manager.h"
#include "ui/screens/home_screen.h"
#include "ui/screens/wifi/wifi_screen.h"
#include "ui/screens/wifi/wifi_status_screen.h"

#include "network/ap/ap_manager/ap_manager.h"

// ------------------------------------------------------------
// UI CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr uint32_t UI_UPDATE_DELAY_MS =
        20;

    constexpr uint32_t UI_DATA_UPDATE_INTERVAL_MS =
        1000;

    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    void update_time()
    {
        time_t now;

        time(&now);

        struct tm time_info;

        localtime_r(
            &now,
            &time_info);

        std::snprintf(
            UIManager::get_model().time,
            sizeof(UIManager::get_model().time),
            "%02d:%02d",
            time_info.tm_hour,
            time_info.tm_min);
    }

    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    void update_temperature()
    {
        float temperature =
            0.0f;

        if (TemperatureMonitor::get_celsius(
                temperature))
        {
            UIManager::get_model().temperature_celsius =
                static_cast<uint8_t>(
                    temperature);
        }
    }

    // --------------------------------------------------------
    // ACCESS POINT
    // --------------------------------------------------------

    void update_ap()
    {
        UIManager::get_model().ap_running =
            APManager::is_running();

        UIManager::get_model().ap_clients =
            APManager::get_client_count();

        UIManager::get_model().ap_max_connections =
            ConfigManager::get_ap_config().max_connections;
    }

    // --------------------------------------------------------
    // WIFI STATION STATUS
    // --------------------------------------------------------

    void update_wifi_status()
    {
        WiFiStatus::update();

        const WiFiStatus::Info &info =
            WiFiStatus::get_info();

        UIModel &model =
            UIManager::get_model();

        model.wifi_enabled =
            info.enabled;

        model.wifi_connected =
            info.connected;

        model.wifi_rssi =
            info.rssi;

        model.wifi_channel =
            info.channel;

        // ----------------------------------------------------
        // Convert RSSI to approximate signal percentage
        // ----------------------------------------------------

        model.wifi_signal_percent =
            0;

        if (info.enabled &&
            info.connected)
        {
            int signal_percent =
                2 * (
                    static_cast<int>(
                        info.rssi) + 100);

            if (signal_percent < 0)
            {
                signal_percent =
                    0;
            }

            if (signal_percent > 100)
            {
                signal_percent =
                    100;
            }

            model.wifi_signal_percent =
                static_cast<uint8_t>(
                    signal_percent);
        }

        // ----------------------------------------------------
        // SSID
        // ----------------------------------------------------

        std::snprintf(
            model.wifi_ssid,
            sizeof(model.wifi_ssid),
            "%s",
            info.ssid);

        // ----------------------------------------------------
        // IP address
        // ----------------------------------------------------

        std::snprintf(
            model.wifi_ip_address,
            sizeof(model.wifi_ip_address),
            "%s",
            info.ip_address);

        // ----------------------------------------------------
        // Security
        // ----------------------------------------------------

        std::snprintf(
            model.wifi_security,
            sizeof(model.wifi_security),
            "%s",
            info.security);
    }

    // --------------------------------------------------------
    // UPTIME
    // --------------------------------------------------------

    void update_uptime()
    {
        UIManager::get_model().uptime_seconds =
            static_cast<uint32_t>(
                esp_timer_get_time() /
                1000000ULL);
    }

    // --------------------------------------------------------
    // UI TASK
    // --------------------------------------------------------

    void ui_task(
        void *parameter)
    {
        (void)parameter;

        int64_t last_data_update =
            0;

        while (true)
        {
            UIManager::update();

            const int64_t now =
                esp_timer_get_time();

            if ((now - last_data_update) >=
                (UI_DATA_UPDATE_INTERVAL_MS * 1000LL))
            {
                last_data_update =
                    now;

                update_time();
                update_temperature();
                update_ap();
                update_wifi_status();
                update_uptime();

                /*
                 * The Home screen is refreshed periodically
                 * because it displays live system information.
                 *
                 * Other screens are rendered when navigation
                 * changes the active screen.
                 */
                if (UIManager::get_screen() ==
                        UIScreen::HOME ||
                    UIManager::get_screen() ==
                        UIScreen::WIFI_STATUS)
                {
                    UIManager::render();
                }
            }

            vTaskDelay(
                pdMS_TO_TICKS(
                    UI_UPDATE_DELAY_MS));
        }
    }
}

// ------------------------------------------------------------
// UI STATE
// ------------------------------------------------------------
//
// The designated initializers follow the exact field order
// declared in UIModel.
// ------------------------------------------------------------

UIModel UIManager::model =
{
    // --------------------------------------------------------
    // SYSTEM
    // --------------------------------------------------------

    .temperature_celsius = 42,

    .uptime_seconds = 0,

    .time = "23:47",

    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    .wifi_enabled = true,

    .wifi_connected = true,

    .wifi_signal_percent = 100,

    // --------------------------------------------------------
    // ACCESS POINT
    // --------------------------------------------------------

    .ap_running = true,

    .ap_clients = 3,

    .ap_max_connections = 3,

    // --------------------------------------------------------
    // BLUETOOTH
    // --------------------------------------------------------

    .bluetooth_enabled = false,

    .bluetooth_connected = false,

    // --------------------------------------------------------
    // FIRMWARE
    // --------------------------------------------------------

    .firmware_version = "v1.0.0",

    // --------------------------------------------------------
    // WIFI STATION STATUS
    // --------------------------------------------------------

    .wifi_ssid = "",

    .wifi_ip_address = "",

    .wifi_rssi = 0,

    .wifi_channel = 0,

    .wifi_security = ""
};

UIScreen UIManager::current_screen =
    UIScreen::HOME;

WiFiMenuItem UIManager::wifi_menu_item =
    WiFiMenuItem::STATUS;

bool UIManager::initialized =
    false;

// ------------------------------------------------------------
// INITIALIZATION
// ------------------------------------------------------------

bool UIManager::init()
{
    LOG_INFO(SYSTEM, HARDWARE, "UIManager::init() entered");

    if (initialized)
    {
        LOG_INFO(SYSTEM, HARDWARE, "UIManager already initialized");
        return true;
    }

    LOG_INFO(SYSTEM, HARDWARE, "Checking DisplayManager");

    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        LOG_ERROR(SYSTEM, HARDWARE, "DisplayManager is not initialized or has no driver");
        return false;
    }

    LOG_INFO(SYSTEM, HARDWARE, "DisplayManager check passed");

    current_screen =
        UIScreen::HOME;

    wifi_menu_item =
        WiFiMenuItem::STATUS;

    LOG_INFO(SYSTEM, WIFI, "Initializing WiFiStatus");

    WiFiStatus::init();

    LOG_INFO(SYSTEM, WIFI, "WiFiStatus initialized");

    initialized =
        true;

    LOG_INFO(SYSTEM, HARDWARE, "UIManager marked initialized");

    LOG_INFO(SYSTEM, HARDWARE, "Creating UI task");

    BaseType_t task_result =
        xTaskCreate(
            ui_task,
            "ui_task",
            4096,
            nullptr,
            5,
            nullptr);

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "xTaskCreate returned: %d",
        static_cast<int>(task_result));

    if (task_result != pdPASS)
    {
        LOG_ERROR(SYSTEM, HARDWARE, "Failed to create UI task");

        initialized =
            false;

        return false;
    }

    LOG_INFO(SYSTEM, HARDWARE, "UI task created successfully");
    LOG_INFO(SYSTEM, HARDWARE, "UIManager initialization complete");

    return true;
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------

void UIManager::update()
{
    if (!initialized)
    {
        return;
    }

    JoystickManager::update();

    JoystickState joystick =
        JoystickManager::read();

    static JoystickDirection previous_direction =
        JoystickDirection::CENTER;

    JoystickDirection current_direction =
        joystick.direction;

    bool screen_changed =
        false;

    // --------------------------------------------------------
    // NAVIGATION EDGE
    // --------------------------------------------------------

    bool new_direction =
        current_direction !=
            JoystickDirection::CENTER &&
        previous_direction ==
            JoystickDirection::CENTER;

    if (new_direction)
    {
        // ----------------------------------------------------
        // HOME SCREEN
        // ----------------------------------------------------

        if (current_screen ==
            UIScreen::HOME)
        {
            if (current_direction ==
                JoystickDirection::RIGHT)
            {
                current_screen =
                    UIScreen::WIFI;

                screen_changed =
                    true;
            }
        }

        // ----------------------------------------------------
        // WIFI MAIN SCREEN
        // ----------------------------------------------------

        else if (current_screen ==
                 UIScreen::WIFI)
        {
            if (current_direction ==
                JoystickDirection::LEFT)
            {
                current_screen =
                    UIScreen::HOME;

                screen_changed =
                    true;
            }
        }

        // ----------------------------------------------------
        // WIFI MENU
        // ----------------------------------------------------

        else if (current_screen ==
                 UIScreen::WIFI_MENU)
        {
            if (current_direction ==
                JoystickDirection::UP)
            {
                uint8_t item =
                    static_cast<uint8_t>(
                        wifi_menu_item);

                if (item > 0)
                {
                    item--;

                    wifi_menu_item =
                        static_cast<WiFiMenuItem>(
                            item);

                    screen_changed =
                        true;
                }
            }

            if (current_direction ==
                JoystickDirection::DOWN)
            {
                uint8_t item =
                    static_cast<uint8_t>(
                        wifi_menu_item);

                constexpr uint8_t LAST_ITEM =
                    static_cast<uint8_t>(
                        WiFiMenuItem::COUNT) - 1;

                if (item < LAST_ITEM)
                {
                    item++;

                    wifi_menu_item =
                        static_cast<WiFiMenuItem>(
                            item);

                    screen_changed =
                        true;
                }
            }

            if (current_direction ==
                JoystickDirection::LEFT)
            {
                current_screen =
                    UIScreen::WIFI;

                screen_changed =
                    true;
            }

            if (current_direction ==
                JoystickDirection::RIGHT)
            {
                switch (wifi_menu_item)
                {
                    case WiFiMenuItem::STATUS:

                        current_screen =
                            UIScreen::WIFI_STATUS;

                        break;

                    case WiFiMenuItem::SCAN:

                        current_screen =
                            UIScreen::WIFI_SCAN;

                        break;

                    case WiFiMenuItem::NETWORKS:

                        current_screen =
                            UIScreen::WIFI_NETWORKS;

                        break;

                    case WiFiMenuItem::AP:

                        current_screen =
                            UIScreen::WIFI_AP;

                        break;

                    case WiFiMenuItem::COUNT:

                        break;
                }

                screen_changed =
                    true;
            }
        }

        // ----------------------------------------------------
        // WIFI DETAIL SCREENS
        // ----------------------------------------------------

        else if (current_screen ==
                     UIScreen::WIFI_STATUS ||
                 current_screen ==
                     UIScreen::WIFI_SCAN ||
                 current_screen ==
                     UIScreen::WIFI_NETWORKS ||
                 current_screen ==
                     UIScreen::WIFI_AP)
        {
            if (current_direction ==
                JoystickDirection::LEFT)
            {
                current_screen =
                    UIScreen::WIFI_MENU;

                screen_changed =
                    true;
            }
        }
    }

    previous_direction =
        current_direction;

    // --------------------------------------------------------
    // SELECT BUTTON
    // --------------------------------------------------------

    if (joystick.pressed_event)
    {
        // ----------------------------------------------------
        // WIFI MAIN SCREEN
        // ----------------------------------------------------

        if (current_screen ==
            UIScreen::WIFI)
        {
            current_screen =
                UIScreen::WIFI_MENU;

            wifi_menu_item =
                WiFiMenuItem::STATUS;

            screen_changed =
                true;
        }

        // ----------------------------------------------------
        // WIFI SCAN SCREEN
        // ----------------------------------------------------

        else if (current_screen ==
                 UIScreen::WIFI_SCAN)
        {
            // Wi-Fi scan action will be connected later.
        }
    }

    if (screen_changed)
    {
        render();
    }
}

// ------------------------------------------------------------
// RENDER
// ------------------------------------------------------------

void UIManager::render()
{
    if (!initialized)
    {
        return;
    }

    switch (current_screen)
    {
        case UIScreen::HOME:

            HomeScreen::render(
                model);

            break;

        case UIScreen::WIFI:

            WiFiScreen::render_main();

            break;

        case UIScreen::WIFI_MENU:

            WiFiScreen::render_menu(
                static_cast<uint8_t>(
                    wifi_menu_item));

            break;

        case UIScreen::WIFI_STATUS:

            WiFiStatusScreen::render(
                model);

            break;

        case UIScreen::WIFI_SCAN:
        case UIScreen::WIFI_NETWORKS:
        case UIScreen::WIFI_AP:

            // These screens will be implemented later.
            DisplayManager::clear(
                0x000000);

            DisplayManager::update();

            break;

        case UIScreen::BLUETOOTH:
        case UIScreen::SYSTEM:
        case UIScreen::LOGS:
        case UIScreen::CONFIGURATION:

            DisplayManager::clear(
                0x000000);

            DisplayManager::update();

            break;
    }
}

// ------------------------------------------------------------
// SCREEN
// ------------------------------------------------------------

UIScreen UIManager::get_screen()
{
    return current_screen;
}

void UIManager::set_screen(
    UIScreen screen)
{
    if (!initialized)
    {
        return;
    }

    current_screen =
        screen;

    render();
}

// ------------------------------------------------------------
// WIFI MENU ITEM
// ------------------------------------------------------------

WiFiMenuItem UIManager::get_wifi_menu_item()
{
    return wifi_menu_item;
}

// ------------------------------------------------------------
// MODEL
// ------------------------------------------------------------

UIModel &UIManager::get_model()
{
    return model;
}