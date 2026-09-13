#pragma once

#include <cstdint>

// ------------------------------------------------------------
// JOYSTICK DIRECTION
// ------------------------------------------------------------

enum class JoystickDirection : uint8_t
{
    CENTER = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// ------------------------------------------------------------
// JOYSTICK STATE
// ------------------------------------------------------------

struct JoystickState
{
    int raw_x;
    int raw_y;

    JoystickDirection direction;

    bool pressed;

    bool pressed_event;
    bool released_event;
};

// ------------------------------------------------------------
// JOYSTICK MANAGER
// ------------------------------------------------------------

class JoystickManager
{
public:
    static bool init();

    static void update();

    static JoystickState read();

    static int get_x();

    static int get_y();

    static bool is_pressed();

    static bool was_pressed();

    static bool was_released();

    static JoystickDirection get_direction();

    static bool is_up();

    static bool is_down();

    static bool is_left();

    static bool is_right();

private:
    static int read_x();

    static int read_y();

    static JoystickDirection calculate_direction(
        int x,
        int y);
};