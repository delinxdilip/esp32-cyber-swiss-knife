#pragma once

#include "core/config/config_manager.h"

class APManager
{
public:
    static bool init();

    static bool apply_config(
        const APConfig &config);

    static bool is_initialized();
};