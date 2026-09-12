#include "ui_manager.h"

#include <ctime>
#include <cstdio>

#include "core/monitoring/temperature/temperature_monitor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "hardware/input/joystick/joystick_manager.h"

#include "ui/display/display_manager.h"
#include "ui/screens/home_screen.h"
#include "ui/screens/wifi_screen.h"

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

    void update_temperature()
    {
        float temperature = 0.0f;

        if (TemperatureMonitor::get_celsius(
                temperature))
        {
            UIManager::get_model().temperature_celsius =
                static_cast<uint8_t>(temperature);
        }
    }

    void update_ap()
    {
        UIManager::get_model().ap_running =
            APManager::is_running();

        UIManager::get_model().ap_clients =
            APManager::get_client_count();

        UIManager::get_model().ap_max_connections =
            ConfigManager::get_ap_config().max_connections;
    }

    void update_uptime()
    {
        UIManager::get_model().uptime_seconds =
            static_cast<uint32_t>(
                esp_timer_get_time() / 1000000ULL);
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
                update_uptime();

                if (UIManager::get_screen() ==
                    UIScreen::HOME)
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

UIModel UIManager::model =
{
    // --------------------------------------------------------
    // SYSTEM
    // --------------------------------------------------------

    .temperature_celsius = 42,
    .uptime_seconds = 0,
    .time = "23:47",

    // --------------------------------------------------------
    // NETWORK
    // --------------------------------------------------------

    .wifi_enabled = true,
    .wifi_connected = true,
    .wifi_signal_percent = 100,

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

    .firmware_version = "v1.0.0"
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
    if (initialized)
    {
        return true;
    }

    if (!DisplayManager::is_initialized() ||
        !DisplayManager::has_driver())
    {
        return false;
    }

    current_screen =
        UIScreen::HOME;

    wifi_menu_item =
        WiFiMenuItem::STATUS;

    initialized =
        true;

    BaseType_t task_result =
        xTaskCreate(
            ui_task,
            "ui_task",
            4096,
            nullptr,
            5,
            nullptr);

    if (task_result != pdPASS)
    {
        initialized =
            false;

        return false;
    }

    render();

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
        // MAIN SCREEN
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
        // WIFI MAIN
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
    // SELECT
    // --------------------------------------------------------

    if (joystick.pressed_event)
    {
        // ----------------------------------------------------
        // WIFI MAIN
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
        // WIFI SCAN
        // ----------------------------------------------------

        else if (current_screen ==
                 UIScreen::WIFI_SCAN)
        {
            // ------------------------------------------------
            // Scan action will be connected to WiFiManager
            // later.
            // ------------------------------------------------
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

            WiFiScreen::render_status(
                model);

            break;

        case UIScreen::WIFI_SCAN:

            WiFiScreen::render_scan();

            break;

        case UIScreen::WIFI_NETWORKS:

            WiFiScreen::render_networks();

            break;

        case UIScreen::WIFI_AP:

            WiFiScreen::render_ap(
                model);

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