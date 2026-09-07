#pragma once

#include <stdint.h>

struct APConfig
{
    char ssid[33];
    char password[65];
    uint8_t max_connections;
    uint8_t channel;
};

class ConfigManager
{
public:
    static bool init();

    static bool load();

    static bool save();

    static const APConfig &get_ap_config();

    static bool validate(
        const APConfig &config);

    static bool set_ap_config(
        const APConfig &config);

private:
    static APConfig ap_config;

    static void set_defaults();
};