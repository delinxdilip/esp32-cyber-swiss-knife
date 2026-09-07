#pragma once

#include <stddef.h>
#include <stdint.h>

class JsonParser
{
public:
    JsonParser();

    bool parse(
        const char *json,
        size_t length);

    bool parse(
        const char *json);

    bool get_string(
        const char *key,
        char *output,
        size_t output_size) const;

    bool get_uint(
        const char *key,
        uint32_t &value) const;

    bool get_int(
        const char *key,
        int32_t &value) const;

    bool get_bool(
        const char *key,
        bool &value) const;

    bool has(
        const char *key) const;

    bool valid() const;

private:
    const char *json_data;
    size_t json_length;

    bool parsed;

    const char *find_key(
        const char *key) const;

    static void skip_whitespace(
        const char *&cursor,
        const char *end);

    static bool decode_string(
        const char *start,
        const char *end,
        char *output,
        size_t output_size,
        const char *&cursor);

    static bool parse_uint(
        const char *start,
        const char *end,
        uint32_t &value,
        const char *&cursor);

    static bool parse_int(
        const char *start,
        const char *end,
        int32_t &value,
        const char *&cursor);

    static bool parse_bool(
        const char *start,
        const char *end,
        bool &value,
        const char *&cursor);
};