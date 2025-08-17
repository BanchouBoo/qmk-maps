#include QMK_KEYBOARD_H
#include <math.h>
#include "vec2.h"

// safe range starts at `PLOOPY_SAFE_RANGE` instead.

#define SIGN(x) ((x > 0) - (x < 0))

#define SENSITIVITY 0.2 // TARGET_DPI / ACTUAL_DPI = 400 / 2000

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT( KC_BTN4, KC_BTN5, DRAG_SCROLL, KC_BTN2, KC_BTN1, KC_BTN3 )
};

float accel_classic_sensitivity(float x, float offset, float acceleration, float exponent) {
    x = abs(x);
    float sensitivity = 1.0;
    if (x > offset) {
        float raised_acceleration = pow(acceleration, exponent - 1.0);
        sensitivity += (raised_acceleration * pow(x - offset, exponent)) / x;
    }
    return sensitivity;
}

float accel_classic(float x, float offset, float acceleration, float exponent) {
    return x * accel_classic_sensitivity(x, offset, acceleration, exponent);
}

report_mouse_t pointing_device_task_user(report_mouse_t report) {
    static vec2_t accumulated = { .x = 0, .y = 0 };

    vec2_t move_delta = vec2_mul(vec2(report.x, report.y), SENSITIVITY);

    float magnitude = vec2_length(move_delta);
    magnitude = accel_classic(magnitude, 0.0, 0.03, 2.0);
    move_delta = vec2_mul(vec2_normalized(move_delta), magnitude);

    move_delta = vec2_add(move_delta, accumulated);

    report.x = move_delta.x;
    report.y = move_delta.y;

    accumulated = vec2_sub(move_delta, vec2_trunc(move_delta));

    return report;
}
