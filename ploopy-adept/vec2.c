#include <math.h>
#include "vec2.h"

const vec2_t vec2_zero = { .x = 0, .y = 0 };

inline vec2_t vec2(float x, float y) {
    return (vec2_t){ .x = x, .y = y };
}

vec2_t vec2_add(vec2_t a, vec2_t b) {
    return (vec2_t){ .x = a.x + b.x, .y = a.y + b.y };
}

vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return (vec2_t){ .x = a.x - b.x, .y = a.y - b.y };
}

vec2_t vec2_mul(vec2_t vec, float factor) {
    return (vec2_t){ .x = vec.x * factor, vec.y * factor };
}

vec2_t vec2_div(vec2_t vec, float divisor) {
    return (vec2_t){ .x = vec.x / divisor, vec.y / divisor };
}

vec2_t vec2_trunc(vec2_t vec) {
    return (vec2_t){ .x = truncf(vec.x), .y = truncf(vec.y) };
}

float vec2_dot(vec2_t a, vec2_t b) {
    return (a.x * b.x) + (a.y * b.y);
}

float vec2_length_sq(vec2_t vec) {
    return vec2_dot(vec, vec);
}

float vec2_length(vec2_t vec) {
    return sqrt(vec2_length_sq(vec));
}

vec2_t vec2_normalized(vec2_t vec) {
    const float length = vec2_length(vec);
    vec2_t result = vec2_zero;
    if (length > 0) {
        result = vec2_div(vec, length);
    }
    return result;
}
