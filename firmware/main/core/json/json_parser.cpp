#include "json_parser.h"

#include <climits>
#include <cstdio>
#include <cstring>

JsonParser::JsonParser()
    : json_data(nullptr),
      json_length(0),
      parsed(false)
{
}

bool JsonParser::parse(
    const char *json,
    size_t length)
{
    json_data = json;
    json_length = length;
    parsed = false;

    if (json_data == nullptr ||
        json_length == 0)
    {
        return false;
    }

    const char *cursor = json_data;
    const char *end = json_data + json_length;

    skip_whitespace(
        cursor,
        end);

    if (cursor >= end ||
        *cursor != '{')
    {
        return false;
    }

    int depth = 0;
    bool in_string = false;
    bool escaped = false;

    for (; cursor < end; ++cursor)
    {
        const char character = *cursor;

        if (in_string)
        {
            if (escaped)
            {
                escaped = false;
                continue;
            }

            if (character == '\\')
            {
                escaped = true;
                continue;
            }

            if (character == '"')
            {
                in_string = false;
            }

            continue;
        }

        if (character == '"')
        {
            in_string = true;
            continue;
        }

        if (character == '{')
        {
            ++depth;
        }
        else if (character == '}')
        {
            --depth;

            if (depth < 0)
            {
                return false;
            }
        }
    }

    if (in_string ||
        escaped ||
        depth != 0)
    {
        return false;
    }

    cursor = json_data + json_length;

    while (cursor > json_data &&
           (cursor[-1] == ' ' ||
            cursor[-1] == '\t' ||
            cursor[-1] == '\r' ||
            cursor[-1] == '\n'))
    {
        --cursor;
    }

    if (cursor <= json_data ||
        cursor[-1] != '}')
    {
        return false;
    }

    parsed = true;

    return true;
}

bool JsonParser::parse(
    const char *json)
{
    if (json == nullptr)
    {
        return false;
    }

    return parse(
        json,
        std::strlen(json));
}

bool JsonParser::get_string(
    const char *key,
    char *output,
    size_t output_size) const
{
    if (!parsed ||
        key == nullptr ||
        output == nullptr ||
        output_size == 0)
    {
        return false;
    }

    const char *value =
        find_key(key);

    if (value == nullptr)
    {
        return false;
    }

    const char *end =
        json_data + json_length;

    skip_whitespace(
        value,
        end);

    if (value >= end ||
        *value != '"')
    {
        return false;
    }

    ++value;

    const char *cursor = value;

    return decode_string(
        value,
        end,
        output,
        output_size,
        cursor);
}

bool JsonParser::get_uint(
    const char *key,
    uint32_t &value) const
{
    if (!parsed ||
        key == nullptr)
    {
        return false;
    }

    const char *start =
        find_key(key);

    if (start == nullptr)
    {
        return false;
    }

    const char *end =
        json_data + json_length;

    skip_whitespace(
        start,
        end);

    return parse_uint(
        start,
        end,
        value,
        start);
}

bool JsonParser::get_int(
    const char *key,
    int32_t &value) const
{
    if (!parsed ||
        key == nullptr)
    {
        return false;
    }

    const char *start =
        find_key(key);

    if (start == nullptr)
    {
        return false;
    }

    const char *end =
        json_data + json_length;

    skip_whitespace(
        start,
        end);

    return parse_int(
        start,
        end,
        value,
        start);
}

bool JsonParser::get_bool(
    const char *key,
    bool &value) const
{
    if (!parsed ||
        key == nullptr)
    {
        return false;
    }

    const char *start =
        find_key(key);

    if (start == nullptr)
    {
        return false;
    }

    const char *end =
        json_data + json_length;

    skip_whitespace(
        start,
        end);

    return parse_bool(
        start,
        end,
        value,
        start);
}

bool JsonParser::has(
    const char *key) const
{
    if (!parsed ||
        key == nullptr)
    {
        return false;
    }

    return find_key(key) != nullptr;
}

bool JsonParser::valid() const
{
    return parsed;
}

const char *JsonParser::find_key(
    const char *key) const
{
    if (!parsed ||
        key == nullptr)
    {
        return nullptr;
    }

    const char *cursor = json_data;
    const char *end =
        json_data + json_length;

    while (cursor < end)
    {
        skip_whitespace(
            cursor,
            end);

        if (cursor >= end)
        {
            break;
        }

        if (*cursor == '"')
        {
            const char *string_start =
                cursor;

            ++cursor;

            bool escaped = false;

            while (cursor < end)
            {
                if (escaped)
                {
                    escaped = false;
                }
                else if (*cursor == '\\')
                {
                    escaped = true;
                }
                else if (*cursor == '"')
                {
                    break;
                }

                ++cursor;
            }

            if (cursor >= end)
            {
                return nullptr;
            }

            const size_t string_length =
                static_cast<size_t>(
                    cursor - string_start + 1);

            char candidate[128];

            if (string_length <
                sizeof(candidate))
            {
                std::memcpy(
                    candidate,
                    string_start,
                    string_length);

                candidate[string_length] =
                    '\0';

                char expected[128];

                std::snprintf(
                    expected,
                    sizeof(expected),
                    "\"%s\"",
                    key);

                if (std::strcmp(
                        candidate,
                        expected) == 0)
                {
                    ++cursor;

                    skip_whitespace(
                        cursor,
                        end);

                    if (cursor < end &&
                        *cursor == ':')
                    {
                        ++cursor;

                        skip_whitespace(
                            cursor,
                            end);

                        return cursor;
                    }
                }
            }

            ++cursor;
            continue;
        }

        ++cursor;
    }

    return nullptr;
}

void JsonParser::skip_whitespace(
    const char *&cursor,
    const char *end)
{
    while (cursor < end)
    {
        if (*cursor == ' ' ||
            *cursor == '\t' ||
            *cursor == '\r' ||
            *cursor == '\n')
        {
            ++cursor;
        }
        else
        {
            break;
        }
    }
}

bool JsonParser::decode_string(
    const char *start,
    const char *end,
    char *output,
    size_t output_size,
    const char *&cursor)
{
    if (start == nullptr ||
        end == nullptr ||
        output == nullptr ||
        output_size == 0)
    {
        return false;
    }

    size_t output_index = 0;

    cursor = start;

    while (cursor < end)
    {
        const char character = *cursor;

        if (character == '"')
        {
            if (output_index >= output_size)
            {
                return false;
            }

            output[output_index] =
                '\0';

            ++cursor;

            return true;
        }

        if (character == '\\')
        {
            ++cursor;

            if (cursor >= end)
            {
                return false;
            }

            char decoded = '\0';

            switch (*cursor)
            {
                case '"':
                    decoded = '"';
                    break;

                case '\\':
                    decoded = '\\';
                    break;

                case '/':
                    decoded = '/';
                    break;

                case 'b':
                    decoded = '\b';
                    break;

                case 'f':
                    decoded = '\f';
                    break;

                case 'n':
                    decoded = '\n';
                    break;

                case 'r':
                    decoded = '\r';
                    break;

                case 't':
                    decoded = '\t';
                    break;

                case 'u':
                {
                    if (cursor + 4 >= end)
                    {
                        return false;
                    }

                    uint32_t codepoint = 0;

                    for (int i = 1; i <= 4; ++i)
                    {
                        const char hex =
                            cursor[i];

                        codepoint <<= 4;

                        if (hex >= '0' &&
                            hex <= '9')
                        {
                            codepoint |=
                                static_cast<uint32_t>(
                                    hex - '0');
                        }
                        else if (hex >= 'a' &&
                                 hex <= 'f')
                        {
                            codepoint |=
                                static_cast<uint32_t>(
                                    hex - 'a' + 10);
                        }
                        else if (hex >= 'A' &&
                                 hex <= 'F')
                        {
                            codepoint |=
                                static_cast<uint32_t>(
                                    hex - 'A' + 10);
                        }
                        else
                        {
                            return false;
                        }
                    }

                    /*
                     * V1 JSON parser:
                     * support ASCII Unicode codepoints.
                     */
                    if (codepoint > 0x7F)
                    {
                        return false;
                    }

                    decoded =
                        static_cast<char>(
                            codepoint);

                    cursor += 4;

                    break;
                }

                default:
                    return false;
            }

            if (output_index + 1 >=
                output_size)
            {
                return false;
            }

            output[output_index++] =
                decoded;

            ++cursor;

            continue;
        }

        if (output_index + 1 >=
            output_size)
        {
            return false;
        }

        output[output_index++] =
            character;

        ++cursor;
    }

    return false;
}

bool JsonParser::parse_uint(
    const char *start,
    const char *end,
    uint32_t &value,
    const char *&cursor)
{
    if (start == nullptr ||
        end == nullptr ||
        start >= end)
    {
        return false;
    }

    cursor = start;

    if (*cursor < '0' ||
        *cursor > '9')
    {
        return false;
    }

    uint32_t result = 0;

    while (cursor < end &&
           *cursor >= '0' &&
           *cursor <= '9')
    {
        const uint32_t digit =
            static_cast<uint32_t>(
                *cursor - '0');

        if (result >
            (UINT32_MAX - digit) / 10)
        {
            return false;
        }

        result =
            result * 10 + digit;

        ++cursor;
    }

    value = result;

    return true;
}

bool JsonParser::parse_int(
    const char *start,
    const char *end,
    int32_t &value,
    const char *&cursor)
{
    if (start == nullptr ||
        end == nullptr ||
        start >= end)
    {
        return false;
    }

    cursor = start;

    bool negative = false;

    if (*cursor == '-')
    {
        negative = true;
        ++cursor;
    }

    if (cursor >= end ||
        *cursor < '0' ||
        *cursor > '9')
    {
        return false;
    }

    uint32_t result = 0;

    while (cursor < end &&
           *cursor >= '0' &&
           *cursor <= '9')
    {
        const uint32_t digit =
            static_cast<uint32_t>(
                *cursor - '0');

        if (result >
            (UINT32_MAX - digit) / 10)
        {
            return false;
        }

        result =
            result * 10 + digit;

        ++cursor;
    }

    if (negative)
    {
        if (result >
            static_cast<uint32_t>(
                INT32_MAX) + 1U)
        {
            return false;
        }

        if (result ==
            static_cast<uint32_t>(
                INT32_MAX) + 1U)
        {
            value = INT32_MIN;
        }
        else
        {
            value =
                -static_cast<int32_t>(
                    result);
        }
    }
    else
    {
        if (result >
            static_cast<uint32_t>(
                INT32_MAX))
        {
            return false;
        }

        value =
            static_cast<int32_t>(
                result);
    }

    return true;
}

bool JsonParser::parse_bool(
    const char *start,
    const char *end,
    bool &value,
    const char *&cursor)
{
    if (start == nullptr ||
        end == nullptr ||
        start >= end)
    {
        return false;
    }

    cursor = start;

    if ((end - cursor) >= 4 &&
        std::strncmp(
            cursor,
            "true",
            4) == 0)
    {
        value = true;
        cursor += 4;
        return true;
    }

    if ((end - cursor) >= 5 &&
        std::strncmp(
            cursor,
            "false",
            5) == 0)
    {
        value = false;
        cursor += 5;
        return true;
    }

    return false;
}