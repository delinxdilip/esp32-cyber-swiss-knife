#include "json_builder.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

JsonBuilder::JsonBuilder(
    char *buffer,
    size_t capacity)
    : buffer(buffer),
      buffer_capacity(capacity),
      buffer_size(0),
      depth(0),
      builder_valid(true)
{
    for (size_t i = 0; i < MAX_DEPTH; ++i)
    {
        stack[i].type = ContainerType::OBJECT;
        stack[i].has_items = false;
    }

    clear();
}

void JsonBuilder::clear()
{
    buffer_size = 0;
    depth = 0;
    builder_valid = true;

    if (buffer != nullptr && buffer_capacity > 0)
    {
        buffer[0] = '\0';
    }

    for (size_t i = 0; i < MAX_DEPTH; ++i)
    {
        stack[i].type = ContainerType::OBJECT;
        stack[i].has_items = false;
    }
}

bool JsonBuilder::append(
    const char *text)
{
    if (!builder_valid || buffer == nullptr || text == nullptr)
    {
        return false;
    }

    const size_t length = strlen(text);

    if (buffer_size + length + 1 > buffer_capacity)
    {
        builder_valid = false;
        return false;
    }

    memcpy(
        buffer + buffer_size,
        text,
        length);

    buffer_size += length;
    buffer[buffer_size] = '\0';

    return true;
}

bool JsonBuilder::append_char(
    char character)
{
    if (!builder_valid || buffer == nullptr)
    {
        return false;
    }

    if (buffer_size + 2 > buffer_capacity)
    {
        builder_valid = false;
        return false;
    }

    buffer[buffer_size++] = character;
    buffer[buffer_size] = '\0';

    return true;
}

bool JsonBuilder::append_format(
    const char *format,
    ...)
{
    if (!builder_valid || buffer == nullptr || format == nullptr)
    {
        return false;
    }

    if (buffer_size >= buffer_capacity)
    {
        builder_valid = false;
        return false;
    }

    va_list arguments;

    va_start(arguments, format);

    const int written = vsnprintf(
        buffer + buffer_size,
        buffer_capacity - buffer_size,
        format,
        arguments);

    va_end(arguments);

    if (written < 0)
    {
        builder_valid = false;
        return false;
    }

    if (static_cast<size_t>(written) >=
        buffer_capacity - buffer_size)
    {
        builder_valid = false;
        return false;
    }

    buffer_size += static_cast<size_t>(written);

    return true;
}

bool JsonBuilder::append_escaped_string(
    const char *value)
{
    if (!append_char('"'))
    {
        return false;
    }

    if (value == nullptr)
    {
        if (!append_char('"'))
        {
            return false;
        }

        return true;
    }

    for (const char *cursor = value;
         *cursor != '\0';
         ++cursor)
    {
        const unsigned char character =
            static_cast<unsigned char>(*cursor);

        switch (character)
        {
            case '"':
                if (!append("\\\""))
                {
                    return false;
                }
                break;

            case '\\':
                if (!append("\\\\"))
                {
                    return false;
                }
                break;

            case '\b':
                if (!append("\\b"))
                {
                    return false;
                }
                break;

            case '\f':
                if (!append("\\f"))
                {
                    return false;
                }
                break;

            case '\n':
                if (!append("\\n"))
                {
                    return false;
                }
                break;

            case '\r':
                if (!append("\\r"))
                {
                    return false;
                }
                break;

            case '\t':
                if (!append("\\t"))
                {
                    return false;
                }
                break;

            default:
                if (character < 0x20)
                {
                    if (!append_format(
                            "\\u%04x",
                            character))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!append_char(
                            static_cast<char>(character)))
                    {
                        return false;
                    }
                }
                break;
        }
    }

    return append_char('"');
}

bool JsonBuilder::write_key(
    const char *key)
{
    if (key == nullptr)
    {
        builder_valid = false;
        return false;
    }

    if (depth == 0)
    {
        builder_valid = false;
        return false;
    }

    if (stack[depth - 1].type !=
        ContainerType::OBJECT)
    {
        builder_valid = false;
        return false;
    }

    if (stack[depth - 1].has_items)
    {
        if (!append_char(','))
        {
            return false;
        }
    }

    if (!append_escaped_string(key))
    {
        return false;
    }

    if (!append_char(':'))
    {
        return false;
    }

    return true;
}

bool JsonBuilder::prepare_value(
    const char *key)
{
    if (!builder_valid)
    {
        return false;
    }

    if (depth == 0)
    {
        return true;
    }

    if (stack[depth - 1].type ==
        ContainerType::OBJECT)
    {
        return write_key(key);
    }

    if (stack[depth - 1].type ==
        ContainerType::ARRAY)
    {
        if (stack[depth - 1].has_items)
        {
            if (!append_char(','))
            {
                return false;
            }
        }

        return true;
    }

    builder_valid = false;
    return false;
}

bool JsonBuilder::prepare_array_value()
{
    return prepare_value(nullptr);
}

bool JsonBuilder::finish_value()
{
    if (!builder_valid)
    {
        return false;
    }

    if (depth > 0)
    {
        stack[depth - 1].has_items = true;
    }

    return true;
}

bool JsonBuilder::begin_object()
{
    if (!builder_valid)
    {
        return false;
    }

    if (!prepare_value(nullptr))
    {
        return false;
    }

    if (!append_char('{'))
    {
        return false;
    }

    if (depth >= MAX_DEPTH)
    {
        builder_valid = false;
        return false;
    }

    stack[depth].type =
        ContainerType::OBJECT;

    stack[depth].has_items =
        false;

    ++depth;

    return true;
}

bool JsonBuilder::begin_object(
    const char *key)
{
    if (!builder_valid)
    {
        return false;
    }

    if (!prepare_value(key))
    {
        return false;
    }

    if (!append_char('{'))
    {
        return false;
    }

    if (depth >= MAX_DEPTH)
    {
        builder_valid = false;
        return false;
    }

    stack[depth].type =
        ContainerType::OBJECT;

    stack[depth].has_items =
        false;

    ++depth;

    return true;
}

bool JsonBuilder::end_object()
{
    if (!builder_valid || depth == 0)
    {
        return false;
    }

    if (stack[depth - 1].type !=
        ContainerType::OBJECT)
    {
        builder_valid = false;
        return false;
    }

    if (!append_char('}'))
    {
        return false;
    }

    --depth;

    return finish_value();
}

bool JsonBuilder::begin_array(
    const char *key)
{
    if (!builder_valid)
    {
        return false;
    }

    if (!prepare_value(key))
    {
        return false;
    }

    if (!append_char('['))
    {
        return false;
    }

    if (depth >= MAX_DEPTH)
    {
        builder_valid = false;
        return false;
    }

    stack[depth].type =
        ContainerType::ARRAY;

    stack[depth].has_items =
        false;

    ++depth;

    return true;
}

bool JsonBuilder::end_array()
{
    if (!builder_valid || depth == 0)
    {
        return false;
    }

    if (stack[depth - 1].type !=
        ContainerType::ARRAY)
    {
        builder_valid = false;
        return false;
    }

    if (!append_char(']'))
    {
        return false;
    }

    --depth;

    return finish_value();
}

bool JsonBuilder::add_string(
    const char *key,
    const char *value)
{
    if (!prepare_value(key))
    {
        return false;
    }

    if (!append_escaped_string(value))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_int(
    const char *key,
    int32_t value)
{
    if (!prepare_value(key))
    {
        return false;
    }

    if (!append_format(
            "%ld",
            static_cast<long>(value)))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_uint(
    const char *key,
    uint32_t value)
{
    if (!prepare_value(key))
    {
        return false;
    }

    if (!append_format(
            "%lu",
            static_cast<unsigned long>(value)))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_bool(
    const char *key,
    bool value)
{
    if (!prepare_value(key))
    {
        return false;
    }

    if (!append(
            value ? "true" : "false"))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_null(
    const char *key)
{
    if (!prepare_value(key))
    {
        return false;
    }

    if (!append("null"))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_raw(
    const char *key,
    const char *value)
{
    if (!prepare_value(key))
    {
        return false;
    }

    if (value == nullptr)
    {
        if (!append("null"))
        {
            return false;
        }
    }
    else
    {
        if (!append(value))
        {
            return false;
        }
    }

    return finish_value();
}

bool JsonBuilder::add_array_string(
    const char *value)
{
    if (!builder_valid ||
        depth == 0 ||
        stack[depth - 1].type !=
            ContainerType::ARRAY)
    {
        builder_valid = false;
        return false;
    }

    if (!prepare_array_value())
    {
        return false;
    }

    if (!append_escaped_string(value))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_array_int(
    int32_t value)
{
    if (!builder_valid ||
        depth == 0 ||
        stack[depth - 1].type !=
            ContainerType::ARRAY)
    {
        builder_valid = false;
        return false;
    }

    if (!prepare_array_value())
    {
        return false;
    }

    if (!append_format(
            "%ld",
            static_cast<long>(value)))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_array_uint(
    uint32_t value)
{
    if (!builder_valid ||
        depth == 0 ||
        stack[depth - 1].type !=
            ContainerType::ARRAY)
    {
        builder_valid = false;
        return false;
    }

    if (!prepare_array_value())
    {
        return false;
    }

    if (!append_format(
            "%lu",
            static_cast<unsigned long>(value)))
    {
        return false;
    }

    return finish_value();
}

bool JsonBuilder::add_array_bool(
    bool value)
{
    if (!builder_valid ||
        depth == 0 ||
        stack[depth - 1].type !=
            ContainerType::ARRAY)
    {
        builder_valid = false;
        return false;
    }

    if (!prepare_array_value())
    {
        return false;
    }

    if (!append(
            value ? "true" : "false"))
    {
        return false;
    }

    return finish_value();
}

const char *JsonBuilder::data() const
{
    if (buffer == nullptr)
    {
        return "";
    }

    return buffer;
}

size_t JsonBuilder::size() const
{
    return buffer_size;
}

size_t JsonBuilder::capacity() const
{
    return buffer_capacity;
}

bool JsonBuilder::valid() const
{
    return builder_valid &&
           buffer != nullptr &&
           depth == 0;
}