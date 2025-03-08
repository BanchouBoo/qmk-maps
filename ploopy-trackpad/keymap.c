#include QMK_KEYBOARD_H

#include <stdint.h>
#include <math.h>
#include "digitizer.h"
#include "vec2.h"
#include "print.h" // for debugging

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {{{ KC_NO }}};

#define WIDTH  DIGITIZER_RESOLUTION_X
#define HEIGHT DIGITIZER_RESOLUTION_Y
#define CENTER_X (WIDTH / 2)
#define CENTER_Y (HEIGHT / 2)
#define INVALID_INDEX DIGITIZER_CONTACT_COUNT

// TODO: move to mode data for runtime confiuration
#define FINGER_INDEX 0
#define FINGER_THUMB 3
#define FINGER_RING  1
#define FINGER_PINKY 2

#define ACCEL_EXPONENT 2.0
#define ACCEL_FACTOR (1.0 / 12.0)
#define ACCEL_OFFSET 0.0
#define ACCEL_CAP 200.0

#define CLAMP(value, min, max) (MIN(max, MAX(value, min)))

// (void)contact to suppress unused variable warnings
#define FOR_CONTACTS(state, ...) \
    for (uint8_t i = 0; i < DIGITIZER_CONTACT_COUNT; i++) { \
        digitizer_contact_t *contact = &(state->contacts[i]); \
        (void)contact; \
        do __VA_ARGS__ while (0); \
    }

#define FOR_ACTIVE_CONTACTS(state, ...) FOR_CONTACTS(state, if (contact->tip) { __VA_ARGS__ })

typedef enum {
    MODE_NORMAL,
    MODE_MOUSE,
    MODE_TABLET,
} mode_t;

typedef enum {
    MODE_RECTANGLE,
    MODE_ELLIPSE,
} tablet_mode_t;

typedef union {
    struct mode_mouse_t {
        // ---SETTINGS---
        // 0.115 is about equivalent to normal trackpad speed with a flat
        // acceleration profile on linux via libinput, and is the default
        // value for `mode_mouse_default`
        float sensitivity;

        // ---INTERNAL DATA---
        vec2_t pointer_position;
        uint8_t pointer_index;
        // TODO: use a constant/define for button length in case it changes later
        uint8_t button_index[4];
        uint8_t buttons;
        vec2_t accumulated;

        // for trackball emulation, eventually
        // vec2_t velocity;
    } mouse;

    struct mode_tablet_t {
        // ---SETTINGS---
        tablet_mode_t mode;
        uint16_t region[2];
        bool shiftable;
        // left right
        uint16_t deadzone_x[2];
        // top bottom
        uint16_t deadzone_y[2];

        // ---INTERNAL DATA---
        uint8_t pointer_index;
        uint8_t button_index[4];
        uint8_t buttons;
        uint16_t min[2];
        uint16_t max[2];
    } tablet;
} mode_data_t;
const mode_data_t mode_no_data;
const mode_data_t mode_tablet_default = { .tablet = { .mode = MODE_RECTANGLE,
                                                      .region = { WIDTH, HEIGHT }, 
                                                      .shiftable = false,
                                                      .pointer_index = INVALID_INDEX,
                                                      .button_index = { INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX },
                                                      .deadzone_x = { 0, 0 },
                                                      .deadzone_y = { 0, 0 } } };
const mode_data_t mode_mouse_default = { .mouse = { .pointer_index = INVALID_INDEX,
                                                    .button_index = { INVALID_INDEX, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX },
                                                    .sensitivity = 0.115 } };

mode_t mode = MODE_NORMAL;
mode_data_t mode_data;
uint8_t last_contact_count = 0;

void change_mode(mode_t new_mode, mode_data_t data) {
    mode = new_mode;
    mode_data = data;
    switch (mode) {
        case MODE_TABLET: {
            // TODO: currently tablet region is automatically rooted to the center, make it
            //       possible and easier to set the initial position for smaller regions
            mode_data.tablet.min[0] = CENTER_X - (mode_data.tablet.region[0] / 2);
            mode_data.tablet.max[0] = CENTER_X + (mode_data.tablet.region[0] / 2);
            mode_data.tablet.min[1] = CENTER_Y - (mode_data.tablet.region[1] / 2);
            mode_data.tablet.max[1] = CENTER_Y + (mode_data.tablet.region[1] / 2);
        } break;
        default: break;
    };
}

uint8_t get_contact_count(digitizer_t *state) {
    uint8_t contact_count = 0;
    FOR_ACTIVE_CONTACTS(state, {
        contact_count += 1;
    });
    return contact_count;
}

vec2_t get_average_position(digitizer_t *state) {
    uint8_t count = 0;
    vec2_t result = vec2_zero;
    FOR_ACTIVE_CONTACTS(state, {
        count += 1;
        result.x += contact->x;
        result.y += contact->y;
    });
    if (count > 0) {
        result.x /= count;
        result.y /= count;
    }
    return result;
}

bool valid_index(uint8_t index) {
    return index < INVALID_INDEX;
}

bool button_index_taken(uint8_t button_index[4], uint8_t index) {
    for (uint8_t i = 0; i < 4; i++) {
        const uint8_t check = button_index[i];
        if (valid_index(index) && index == check) return true;
    }
    return false;
}

// for debugging convenience
void print_contact(digitizer_contact_t contact) {
    uprintf("Type: %d\nTip: %d\nIn Range: %d\nConfidence: %d\nX: %d\nY: %d\n",
            contact.type,
            contact.tip,
            contact.in_range,
            contact.confidence,
            contact.x,
            contact.y);
}

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
    // TODO: better unify these to avoid the duplicate code, maybe make mouse buttons a global instead of mode-specific
    if (mode == MODE_MOUSE) {
        // buttons get reset every report, so we have to store them to set them every time
        report.buttons = mode_data.mouse.buttons;

        if (!valid_index(mode_data.mouse.pointer_index)) {
            // TODO: do inertia stuff here
            //       maybe make inertia a separate mode
        }
    } else if (mode == MODE_TABLET) {
        report.buttons = mode_data.tablet.buttons;
    }
    return report;
}

bool digitizer_task_user(digitizer_t *state) {
    uint8_t contact_count = get_contact_count(state);

    //if (contact_count != last_contact_count)
    //    uprintf("Contact count: %d / %d\n", contact_count, DIGITIZER_CONTACT_COUNT);

    if (contact_count > 5 && contact_count > last_contact_count) {
        // zero out mouse buttons to hopefully avoid issues
        report_mouse_t mouse_report = {
            .buttons = 0
        };
        pointing_device_set_report(mouse_report);
        pointing_device_send();

        if (contact_count == 6) {
            change_mode(MODE_NORMAL, mode_no_data);
        } else if (contact_count == 7) {
            change_mode(MODE_MOUSE, mode_mouse_default);
        } else if (contact_count == 8) {
            #ifdef DIGITIZER_HAS_STYLUS
                mode_data_t data = mode_tablet_default;

                data.tablet.shiftable = true;

                data.tablet.region[0] = WIDTH * 0.367;
                data.tablet.region[1] = HEIGHT * 0.386;


                change_mode(MODE_TABLET, data);
            #endif
        } else if (contact_count == 10) {
            #ifndef MAXTOUCH_BOOTLOADER_GESTURE
                reset_keyboard();
            #endif
        }
    }

    switch (mode) {
        // TODO: add a way to scroll
        case MODE_MOUSE: {
            if (valid_index(mode_data.mouse.pointer_index)) {
                if (!state->contacts[mode_data.mouse.pointer_index].tip) {
                    mode_data.mouse.pointer_index = INVALID_INDEX;
                }
            }
            if (last_contact_count == 0 || !valid_index(mode_data.mouse.pointer_index)) {
                FOR_ACTIVE_CONTACTS(state, {
                    if (button_index_taken(mode_data.mouse.button_index, i)) {
                        continue;
                    }
                    mode_data.mouse.pointer_index = i;
                    mode_data.mouse.pointer_position = vec2(contact->x, contact->y);
                    break;
                });
            }

            vec2_t move_delta = vec2_zero;
            if (valid_index(mode_data.mouse.pointer_index)) {
                digitizer_contact_t *pointer_contact = &(state->contacts[mode_data.mouse.pointer_index]);

                vec2_t last_pointer_position = mode_data.mouse.pointer_position;
                vec2_t pointer_position = vec2(pointer_contact->x, pointer_contact->y);
                mode_data.mouse.pointer_position = pointer_position;
                move_delta = vec2_sub(pointer_position, last_pointer_position);

                move_delta = vec2_mul(move_delta, mode_data.mouse.sensitivity);

                // TODO: refactor acceleration settings so it can easily be seperately configured or disabled at compile time
                //       also reimplement more acceleration functions from rawaccel (see https://github.com/Kuuuube/rawaccel_convert)
                float speed = vec2_length(move_delta);
                // TODO: why does accel need to be roughtly 1/10th to feel the same? Should same scaling apply? Or scaling from libinput points?
                float sens = accel_classic_sensitivity(speed, 0.0, 0.03, 2.0);
                speed *= sens;
                move_delta = vec2_mul(vec2_normalized(move_delta), speed);

                move_delta = vec2_add(move_delta, mode_data.mouse.accumulated);

                FOR_ACTIVE_CONTACTS(state, {
                    if (contact != pointer_contact) {
                        float distance = vec2_length(vec2_sub(pointer_position, vec2(contact->x, contact->y)));
                        if (contact->x < pointer_contact->x) {
                            if ((distance < 4000.0 || valid_index(mode_data.mouse.button_index[FINGER_THUMB])) && !valid_index(mode_data.mouse.button_index[FINGER_INDEX])) {
                                if (!button_index_taken(mode_data.mouse.button_index, i)) {
                                    mode_data.mouse.button_index[FINGER_INDEX] = i;
                                }
                            } else if (!valid_index(mode_data.mouse.button_index[FINGER_THUMB])) {
                                if (!button_index_taken(mode_data.mouse.button_index, i)) {
                                    mode_data.mouse.button_index[FINGER_THUMB] = i;
                                }
                            }
                        } else if (contact->x > pointer_contact->x) {
                            if ((distance < 4000.0 || valid_index(mode_data.mouse.button_index[FINGER_PINKY])) && !valid_index(mode_data.mouse.button_index[FINGER_RING])) {
                                if (!button_index_taken(mode_data.mouse.button_index, i)) {
                                    mode_data.mouse.button_index[FINGER_RING] = i;
                                }
                            } else if (!valid_index(mode_data.mouse.button_index[FINGER_PINKY])) {
                                if (!button_index_taken(mode_data.mouse.button_index, i)) {
                                    mode_data.mouse.button_index[FINGER_PINKY] = i;
                                }
                            }
                        }
                    }
                });
            }

            mode_data.mouse.buttons = 0;
            for (uint8_t i = 0; i < 4; i++) {
                const uint8_t index = mode_data.mouse.button_index[i];
                if (!valid_index(index)) continue;
                if (state->contacts[index].tip) {
                    mode_data.mouse.buttons |= (1 << i);
                } else {
                    mode_data.mouse.button_index[i] = INVALID_INDEX;
                }
            }

            report_mouse_t mouse_report = {
                .x = move_delta.x,
                .y = move_delta.y,
                .buttons = mode_data.mouse.buttons
            };

            mode_data.mouse.accumulated = vec2_sub(move_delta, vec2_trunc(move_delta));

            FOR_CONTACTS(state, {
                contact->tip = false;
            });

            pointing_device_set_report(mouse_report);
            pointing_device_send();
        } break;
        case MODE_TABLET: {
            if (valid_index(mode_data.tablet.pointer_index)) {
                if (!state->contacts[mode_data.tablet.pointer_index].tip) {
                    mode_data.tablet.pointer_index = INVALID_INDEX;
                }
            }
            if (last_contact_count == 0 || !valid_index(mode_data.tablet.pointer_index)) {
                FOR_ACTIVE_CONTACTS(state, {
                    if (button_index_taken(mode_data.tablet.button_index, i)) {
                        continue;
                    }
                    mode_data.tablet.pointer_index = i;
                    break;
                });
            }


            if (valid_index(mode_data.tablet.pointer_index)) {
                digitizer_contact_t *pointer_contact = &(state->contacts[mode_data.tablet.pointer_index]);

                uint16_t x_original = pointer_contact->x;

                pointer_contact->type = STYLUS;
                pointer_contact->tip = false;
                switch (mode_data.tablet.mode) {
                    case MODE_RECTANGLE: {
                        uint16_t width = mode_data.tablet.region[0];
                        uint16_t height = mode_data.tablet.region[1];

                        float x_percent;
                        if (width != WIDTH) {
                            if (mode_data.tablet.shiftable) {
                                if (pointer_contact->x < mode_data.tablet.min[0]) {
                                    uint16_t diff = mode_data.tablet.min[0] - pointer_contact->x;
                                    mode_data.tablet.min[0] -= diff;
                                    mode_data.tablet.max[0] -= diff;
                                } else if (pointer_contact->x > mode_data.tablet.max[0]) {
                                    uint16_t diff = pointer_contact->x - mode_data.tablet.max[0];
                                    mode_data.tablet.min[0] += diff;
                                    mode_data.tablet.max[0] += diff;
                                }
                            }
                            float x_diff = (float)pointer_contact->x - (float)mode_data.tablet.min[0];
                            x_percent = CLAMP(x_diff / (float)width, 0.0, 1.0);
                        } else {
                            x_percent = (float)pointer_contact->x / WIDTH;
                        }

                        float y_percent;
                        if (height != HEIGHT) {
                            if (mode_data.tablet.shiftable) {
                                if (pointer_contact->y < mode_data.tablet.min[1]) {
                                    uint16_t diff = mode_data.tablet.min[1] - pointer_contact->y;
                                    mode_data.tablet.min[1] -= diff;
                                    mode_data.tablet.max[1] -= diff;
                                } else if (pointer_contact->y > mode_data.tablet.max[1]) {
                                    uint16_t diff = pointer_contact->y - mode_data.tablet.max[1];
                                    mode_data.tablet.min[1] += diff;
                                    mode_data.tablet.max[1] += diff;
                                }
                            }
                            float y_diff = (float)pointer_contact->y - (float)mode_data.tablet.min[1];
                            y_percent = CLAMP(y_diff / (float)height, 0.0, 1.0);
                        } else {
                            y_percent = (float)pointer_contact->y / HEIGHT;
                        }

                        uint16_t scaled_width = WIDTH - mode_data.tablet.deadzone_x[0] - mode_data.tablet.deadzone_x[1];
                        uint16_t x = (x_percent * scaled_width) + mode_data.tablet.deadzone_x[0];

                        uint16_t scaled_height = HEIGHT - mode_data.tablet.deadzone_y[0] - mode_data.tablet.deadzone_y[1];
                        uint16_t y = (y_percent * scaled_height) + mode_data.tablet.deadzone_y[0];

                        pointer_contact->x = x;
                        pointer_contact->y = y;
                    } break;
                    default: uprintf("Tablet mode %d not implemented\n", mode_data.tablet.mode); break;
                }

                FOR_ACTIVE_CONTACTS(state, {
                    if (contact != pointer_contact) {
                        if (contact->x < x_original) {
                            mode_data.tablet.button_index[0] = i;
                        } else if (contact->x > x_original) {
                            mode_data.tablet.button_index[1] = i;
                        }
                    }
                });

                mode_data.tablet.buttons = 0;
                for (uint8_t i = 0; i < 4; i++) {
                    const uint8_t index = mode_data.tablet.button_index[i];
                    if (!valid_index(index)) continue;
                    if (state->contacts[index].tip) {
                        mode_data.tablet.buttons |= (1 << i);
                    } else {
                        mode_data.tablet.button_index[i] = INVALID_INDEX;
                    }
                }

                FOR_CONTACTS(state, {
                    contact->tip = false;
                });
            }
        } break;
        default: break;
    }

    last_contact_count = contact_count;
    return true;
}
