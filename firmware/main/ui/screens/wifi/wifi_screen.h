#pragma once

#include <cstdint>

// ------------------------------------------------------------
// WIFI SCREEN
// ------------------------------------------------------------

class WiFiScreen
{
public:
    /**
     * Render the Wi-Fi landing screen.
     *
     * Displays:
     *     <       WI-FI       >
     *     PRESS TO OPEN
     */
    static void render_main();

    /**
     * Render the Wi-Fi submenu.
     *
     * @param selected_item Currently selected menu item.
     *
     * Menu items:
     *     0 - STATUS
     *     1 - SCAN
     *     2 - NETWORKS
     *     3 - AP
     */
    static void render_menu(
        uint8_t selected_item);
};