#pragma once

// use I2C or Serial, not both
#define USE_SERIAL

#define MASTER_LEFT

#define TAPPING_TERM 200

#define FORCE_NKRO

// steno stuff
#define STENO_1UP
#define STENO_REPEAT

// maximum number of strokes in an outline to repeat
#define MAX_REPEAT 8
// window of time within which a stroke is considered a repeat
#define STENO_REPEAT_DELAY_MS 350
// Percent multiplier for the speed of sending repeated strokes
#define STENO_REPEAT_SPEED_PERCENT 400
