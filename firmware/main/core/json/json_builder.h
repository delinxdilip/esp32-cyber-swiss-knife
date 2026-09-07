#pragma once

#include <stddef.h>
#include <stdint.h>

class JsonBuilder
{
public:
    JsonBuilder(
        char *buffer,
        size_t capacity);

    void clear();

    bool begin_object();

    bool begin_object(
        const char *key);

    bool end_object();

    bool begin_array(
        const char *key);

    bool end_array();

    bool add_string(
        const char *key,
        const char *value);

    bool add_int(
        const char *key,
        int32_t value);

    bool add_uint(
        const char *key,
        uint32_t value);

    bool add_bool(
        const char *key,
        bool value);

    bool add_null(
        const char *key);

    bool add_raw(
        const char *key,
        const char *value);

    bool add_array_string(
        const char *value);

    bool add_array_int(
        int32_t value);

    bool add_array_uint(
        uint32_t value);

    bool add_array_bool(
        bool value);

    const char *data() const;

    size_t size() const;

    size_t capacity() const;

    bool valid() const;

private:
    enum class ContainerType
    {
        OBJECT,
        ARRAY
    };

    struct Container
    {
        ContainerType type;
        bool has_items;
    };

    static constexpr size_t MAX_DEPTH = 8;

    char *buffer;
    size_t buffer_capacity;
    size_t buffer_size;

    Container stack[MAX_DEPTH];
    size_t depth;

    bool builder_valid;

    bool append(
        const char *text);

    bool append_char(
        char character);

    bool append_format(
        const char *format,
        ...);

    bool append_escaped_string(
        const char *value);

    bool prepare_value(
        const char *key);

    bool prepare_array_value();

    bool finish_value();

    bool write_key(
        const char *key);
};