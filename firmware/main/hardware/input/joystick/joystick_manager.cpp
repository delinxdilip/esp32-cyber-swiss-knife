#include "joystick_manager.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "core/logging/logger.h"

// ------------------------------------------------------------
// JOYSTICK HARDWARE CONFIGURATION
// ------------------------------------------------------------

namespace
{
    constexpr gpio_num_t JOYSTICK_SW_GPIO =
        GPIO_NUM_8;

    constexpr adc_unit_t JOYSTICK_ADC_UNIT =
        ADC_UNIT_2;

    constexpr adc_channel_t JOYSTICK_VRX_CHANNEL =
        ADC_CHANNEL_6;

    constexpr adc_channel_t JOYSTICK_VRY_CHANNEL =
        ADC_CHANNEL_7;

    constexpr uint16_t CALIBRATION_SAMPLES =
        32;

    constexpr uint32_t CALIBRATION_DELAY_MS =
        5;

    constexpr int JOYSTICK_DEADZONE =
        500;

    constexpr uint32_t BUTTON_DEBOUNCE_MS =
        30;

    adc_oneshot_unit_handle_t adc_handle =
        nullptr;

    bool initialized =
        false;

    int center_x =
        2048;

    int center_y =
        2048;

    bool button_state =
        false;

    bool previous_button_state =
        false;

    bool pressed_event =
        false;

    bool released_event =
        false;

    TickType_t last_button_change_tick =
        0;
}

// ------------------------------------------------------------
// INITIALIZATION
// ------------------------------------------------------------

bool JoystickManager::init()
{
    if (initialized)
    {
        LOG_WARN(
            SYSTEM,
            HARDWARE,
            "Joystick is already initialized");

        return true;
    }

    // --------------------------------------------------------
    // ADC UNIT
    // --------------------------------------------------------

    adc_oneshot_unit_init_cfg_t adc_config = {};

    adc_config.unit_id =
        JOYSTICK_ADC_UNIT;

    adc_config.ulp_mode =
        ADC_ULP_MODE_DISABLE;

    esp_err_t error =
        adc_oneshot_new_unit(
            &adc_config,
            &adc_handle);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to initialize joystick ADC: %s",
            esp_err_to_name(error));

        return false;
    }

    // --------------------------------------------------------
    // ADC CHANNEL CONFIGURATION
    // --------------------------------------------------------

    adc_oneshot_chan_cfg_t channel_config = {};

    channel_config.bitwidth =
        ADC_BITWIDTH_DEFAULT;

    channel_config.atten =
        ADC_ATTEN_DB_12;

    // --------------------------------------------------------
    // VRX
    // --------------------------------------------------------

    error =
        adc_oneshot_config_channel(
            adc_handle,
            JOYSTICK_VRX_CHANNEL,
            &channel_config);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure joystick VRX: %s",
            esp_err_to_name(error));

        adc_oneshot_del_unit(
            adc_handle);

        adc_handle =
            nullptr;

        return false;
    }

    // --------------------------------------------------------
    // VRY
    // --------------------------------------------------------

    error =
        adc_oneshot_config_channel(
            adc_handle,
            JOYSTICK_VRY_CHANNEL,
            &channel_config);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure joystick VRY: %s",
            esp_err_to_name(error));

        adc_oneshot_del_unit(
            adc_handle);

        adc_handle =
            nullptr;

        return false;
    }

    // --------------------------------------------------------
    // BUTTON
    // --------------------------------------------------------

    gpio_config_t button_config = {};

    button_config.pin_bit_mask =
        (1ULL << JOYSTICK_SW_GPIO);

    button_config.mode =
        GPIO_MODE_INPUT;

    button_config.pull_up_en =
        GPIO_PULLUP_ENABLE;

    button_config.pull_down_en =
        GPIO_PULLDOWN_DISABLE;

    button_config.intr_type =
        GPIO_INTR_DISABLE;

    error =
        gpio_config(
            &button_config);

    if (error != ESP_OK)
    {
        LOG_ERROR(
            SYSTEM,
            HARDWARE,
            "Failed to configure joystick button: %s",
            esp_err_to_name(error));

        adc_oneshot_del_unit(
            adc_handle);

        adc_handle =
            nullptr;

        return false;
    }

    // --------------------------------------------------------
    // CENTER CALIBRATION
    // --------------------------------------------------------

    int64_t total_x =
        0;

    int64_t total_y =
        0;

    for (uint16_t sample = 0;
         sample < CALIBRATION_SAMPLES;
         sample++)
    {
        int x =
            0;

        int y =
            0;

        adc_oneshot_read(
            adc_handle,
            JOYSTICK_VRX_CHANNEL,
            &x);

        adc_oneshot_read(
            adc_handle,
            JOYSTICK_VRY_CHANNEL,
            &y);

        total_x +=
            x;

        total_y +=
            y;

        vTaskDelay(
            pdMS_TO_TICKS(
                CALIBRATION_DELAY_MS));
    }

    center_x =
        static_cast<int>(
            total_x /
            CALIBRATION_SAMPLES);

    center_y =
        static_cast<int>(
            total_y /
            CALIBRATION_SAMPLES);

    // --------------------------------------------------------
    // INITIAL BUTTON STATE
    // --------------------------------------------------------

    button_state =
        gpio_get_level(
            JOYSTICK_SW_GPIO) == 0;

    previous_button_state =
        button_state;

    last_button_change_tick =
        xTaskGetTickCount();

    initialized =
        true;

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "Joystick initialized");

    LOG_INFO(
        SYSTEM,
        HARDWARE,
        "Joystick center calibrated: X=%d Y=%d",
        center_x,
        center_y);

    return true;
}

// ------------------------------------------------------------
// READ X
// ------------------------------------------------------------

int JoystickManager::read_x()
{
    if (!initialized ||
        adc_handle == nullptr)
    {
        return 0;
    }

    int value =
        0;

    esp_err_t error =
        adc_oneshot_read(
            adc_handle,
            JOYSTICK_VRX_CHANNEL,
            &value);

    if (error != ESP_OK)
    {
        return 0;
    }

    return value;
}

// ------------------------------------------------------------
// READ Y
// ------------------------------------------------------------

int JoystickManager::read_y()
{
    if (!initialized ||
        adc_handle == nullptr)
    {
        return 0;
    }

    int value =
        0;

    esp_err_t error =
        adc_oneshot_read(
            adc_handle,
            JOYSTICK_VRY_CHANNEL,
            &value);

    if (error != ESP_OK)
    {
        return 0;
    }

    return value;
}

// ------------------------------------------------------------
// CALCULATE DIRECTION
// ------------------------------------------------------------

JoystickDirection JoystickManager::calculate_direction(
    int x,
    int y)
{
    int delta_x =
        x - center_x;

    int delta_y =
        y - center_y;

    if (delta_x > JOYSTICK_DEADZONE)
    {
        return JoystickDirection::RIGHT;
    }

    if (delta_x < -JOYSTICK_DEADZONE)
    {
        return JoystickDirection::LEFT;
    }

    if (delta_y > JOYSTICK_DEADZONE)
    {
        return JoystickDirection::DOWN;
    }

    if (delta_y < -JOYSTICK_DEADZONE)
    {
        return JoystickDirection::UP;
    }

    return JoystickDirection::CENTER;
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------

void JoystickManager::update()
{
    if (!initialized)
    {
        return;
    }

    pressed_event =
        false;

    released_event =
        false;

    bool raw_button_state =
        gpio_get_level(
            JOYSTICK_SW_GPIO) == 0;

    if (raw_button_state != button_state)
    {
        TickType_t current_tick =
            xTaskGetTickCount();

        TickType_t elapsed_ticks =
            current_tick -
            last_button_change_tick;

        if (pdTICKS_TO_MS(elapsed_ticks) >=
            BUTTON_DEBOUNCE_MS)
        {
            previous_button_state =
                button_state;

            button_state =
                raw_button_state;

            last_button_change_tick =
                current_tick;

            if (button_state &&
                !previous_button_state)
            {
                pressed_event =
                    true;
            }

            if (!button_state &&
                previous_button_state)
            {
                released_event =
                    true;
            }
        }
    }
}

// ------------------------------------------------------------
// READ STATE
// ------------------------------------------------------------

JoystickState JoystickManager::read()
{
    JoystickState state = {};

    if (!initialized)
    {
        return state;
    }

    state.raw_x =
        read_x();

    state.raw_y =
        read_y();

    state.direction =
        calculate_direction(
            state.raw_x,
            state.raw_y);

    state.pressed =
        button_state;

    state.pressed_event =
        pressed_event;

    state.released_event =
        released_event;

    return state;
}

// ------------------------------------------------------------
// GET X
// ------------------------------------------------------------

int JoystickManager::get_x()
{
    return read_x();
}

// ------------------------------------------------------------
// GET Y
// ------------------------------------------------------------

int JoystickManager::get_y()
{
    return read_y();
}

// ------------------------------------------------------------
// BUTTON STATE
// ------------------------------------------------------------

bool JoystickManager::is_pressed()
{
    return button_state;
}

// ------------------------------------------------------------
// BUTTON PRESSED EVENT
// ------------------------------------------------------------

bool JoystickManager::was_pressed()
{
    return pressed_event;
}

// ------------------------------------------------------------
// BUTTON RELEASED EVENT
// ------------------------------------------------------------

bool JoystickManager::was_released()
{
    return released_event;
}

// ------------------------------------------------------------
// DIRECTION
// ------------------------------------------------------------

JoystickDirection JoystickManager::get_direction()
{
    int x =
        read_x();

    int y =
        read_y();

    return calculate_direction(
        x,
        y);
}

// ------------------------------------------------------------
// UP
// ------------------------------------------------------------

bool JoystickManager::is_up()
{
    return get_direction() ==
           JoystickDirection::UP;
}

// ------------------------------------------------------------
// DOWN
// ------------------------------------------------------------

bool JoystickManager::is_down()
{
    return get_direction() ==
           JoystickDirection::DOWN;
}

// ------------------------------------------------------------
// LEFT
// ------------------------------------------------------------

bool JoystickManager::is_left()
{
    return get_direction() ==
           JoystickDirection::LEFT;
}

// ------------------------------------------------------------
// RIGHT
// ------------------------------------------------------------

bool JoystickManager::is_right()
{
    return get_direction() ==
           JoystickDirection::RIGHT;
}