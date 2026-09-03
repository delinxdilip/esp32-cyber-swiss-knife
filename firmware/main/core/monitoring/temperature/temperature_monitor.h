#pragma once

class TemperatureMonitor
{
public:
    static bool init();
    static bool get_celsius(float &temperature);
};