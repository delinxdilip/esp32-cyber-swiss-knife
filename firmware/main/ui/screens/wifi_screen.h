#pragma once

#include <cstdint>

#include "ui/ui_model.h"

// ------------------------------------------------------------
// WIFI SCREEN
// ------------------------------------------------------------

class WiFiScreen
{
public:
    /**
     * Render the Wi-Fi main screen.
     */
    static void render_main();

    /**
     * Render the Wi-Fi submenu.
     *
     * @param selected_item Currently selected menu item.
     */
    static void render_menu(
        uint8_t selected_item);

    /**
     * Render Wi-Fi status.
     *
     * @param model Current UI data model.
     */
    static void render_status(
        const UIModel &model);

    /**
     * Render Wi-Fi scan screen.
     */
    static void render_scan();

    /**
     * Render Wi-Fi networks screen.
     */
    static void render_networks();

    /**
     * Render Wi-Fi access point screen.
     *
     * @param model Current UI data model.
     */
    static void render_ap(
        const UIModel &model);
};