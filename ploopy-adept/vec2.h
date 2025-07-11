typedef union {
    float elements[2];
    struct {
        float x;
        float y;
    };
} vec2_t;

extern const vec2_t vec2_zero;

vec2_t vec2(float x, float y);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_mul(vec2_t vec, float factor);
vec2_t vec2_div(vec2_t vec, float divisor);
vec2_t vec2_trunc(vec2_t vec);
float vec2_dot(vec2_t a, vec2_t b);
float vec2_length_sq(vec2_t vec);
float vec2_length(vec2_t vec);
vec2_t vec2_normalized(vec2_t vec);
