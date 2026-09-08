#pragma once

#include <cstdint>

class StorageManager
{
public:

    static bool initialize();

    static uint64_t get_total_space();
    static uint64_t get_used_space();
    static uint64_t get_free_space();

    static const char *get_base_path();

    static bool is_initialized();

private:

    static bool initialized;
};