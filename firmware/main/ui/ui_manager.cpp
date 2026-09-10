#include "ui_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware/input/joystick/joystick_manager.h"

#include "ui/display/display_manager.h"
#include "ui/screens/home_screen.h"
#include "ui/screens/wifi_screen.h"

// ------------------------------------------------------------
// UI CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr uint32_t UI_UPDATE_DELAY_MS =
        20;

    // --------------------------------------------------------
    // UI TASK
    // --------------------------------------------------------

    void ui_task(
        void *parameter)
    {
        (void)parameter;

        while (true)
        {
            UIManager::update();

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

    42,
    754,

    // --------------------------------------------------------
    // NETWORK
    // --------------------------------------------------------

    true,
    100,

    true,
    3,

    // --------------------------------------------------------
    // BLUETOOTH
    // --------------------------------------------------------

    false
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

    if (!JoystickManager::init())
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