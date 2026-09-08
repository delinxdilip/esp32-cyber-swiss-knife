#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t gc9a01_display_init(void);
esp_err_t gc9a01_display_test(void);

#ifdef __cplusplus
}
#endif