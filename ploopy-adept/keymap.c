#include QMK_KEYBOARD_H
#include <math.h>

// safe range starts at `PLOOPY_SAFE_RANGE` instead.

#define SIGN(x) ((x > 0) - (x < 0))

float sensitivity = 400.0 / 2000.0; // TARGET_DPI / ACTUAL_DPI

float x_offset = 1.0;
float y_offset = 1.0;
float exponent = 2.0;

float acceleration = 1.0 / 8.0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT( KC_BTN4, KC_BTN5, DRAG_SCROLL, KC_BTN2, KC_BTN1, KC_BTN3 )
};

report_mouse_t pointing_device_task_user(report_mouse_t report) {
    static float carry_x = 0.0;
    float scaled_x_offset = x_offset * sensitivity;
    float accelerated_x = (float)report.x * sensitivity;

    if (abs(accelerated_x) > scaled_x_offset) {
        accelerated_x -= scaled_x_offset * SIGN(accelerated_x);
        accelerated_x += (pow(abs(accelerated_x), exponent) * acceleration * SIGN(accelerated_x));
    }
    accelerated_x += carry_x;
    carry_x = accelerated_x - trunc(accelerated_x);

    static float carry_y = 0.0;
    float scaled_y_offset = y_offset * sensitivity;
    float accelerated_y = (float)report.y * sensitivity;

    if (abs(accelerated_y) > scaled_y_offset) {
        accelerated_y -= scaled_y_offset * SIGN(accelerated_y);
        accelerated_y += (pow(abs(accelerated_y), exponent) * acceleration * SIGN(accelerated_y));
    }
    accelerated_y += carry_y;
    carry_y = accelerated_y - trunc(accelerated_y);

    // angle snapping, work on more
    // if (abs(accelerated_x) >= 2 && abs(accelerated_y) <= 1) {
    //     carry_y = 0;
    //     accelerated_y = 0;
    // } else if (abs(accelerated_y) >= 2 && abs(accelerated_x) <= 1) {
    //     carry_x = 0;
    //     accelerated_x = 0;
    // }

    report.x = trunc(accelerated_x);
    report.y = trunc(accelerated_y);

    return report;
}
