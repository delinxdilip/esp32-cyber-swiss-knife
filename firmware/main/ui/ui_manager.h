#pragma once

#include <cstdint>

#include "ui_model.h"

// ------------------------------------------------------------
// UI SCREEN
// ------------------------------------------------------------

enum class UIScreen : uint8_t
{
    HOME = 0,

    WIFI,
    WIFI_MENU,
    WIFI_STATUS,
    WIFI_SCAN,
    WIFI_NETWORKS,
    WIFI_AP,

    BLUETOOTH,
    SYSTEM,
    LOGS,
    CONFIGURATION
};

// ------------------------------------------------------------
// WIFI MENU ITEM
// ------------------------------------------------------------

enum class WiFiMenuItem : uint8_t
{
    STATUS = 0,
    SCAN,
    NETWORKS,
    AP,

    COUNT
};

// ------------------------------------------------------------
// UI MANAGER
// ------------------------------------------------------------

class UIManager
{
public:
    /**
     * Initialize the UI subsystem.
     *
     * @return true if initialization succeeds.
     */
    static bool init();

    /**
     * Update the UI subsystem.
     *
     * Input handling and screen updates will be processed here.
     */
    static void update();

    /**
     * Render the active screen.
     */
    static void render();

    /**
     * Get the active screen.
     *
     * @return Currently active UI screen.
     */
    static UIScreen get_screen();

    /**
     * Change the active screen.
     *
     * @param screen Screen to display.
     */
    static void set_screen(
        UIScreen screen);

    /**
     * Get the current Wi-Fi menu item.
     *
     * @return Currently selected Wi-Fi menu item.
     */
    static WiFiMenuItem get_wifi_menu_item();

    /**
     * Get the current UI model.
     *
     * @return Reference to the current UI model.
     */
    static UIModel &get_model();

private:
    static UIModel model;

    static UIScreen current_screen;

    static WiFiMenuItem wifi_menu_item;

    static bool initialized;
};