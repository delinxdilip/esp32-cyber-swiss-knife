#include "core/system/system_manager.h"
#include "ui/display/gc9a01_display.h"

extern "C" void app_main()
{
    SystemManager::init();

    ESP_ERROR_CHECK(gc9a01_display_init());
    ESP_ERROR_CHECK(gc9a01_display_test());
}