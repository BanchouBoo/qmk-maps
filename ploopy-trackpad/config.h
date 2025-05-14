// bigger data types for mouse reports, prevents overflow from acceleration
#define MOUSE_EXTENDED_REPORT

// disable automatic stylus detection
#define MXT_ENABLE_STYLUS false
// enable stylus device reporting, so I can manually send stylus inputs
#define DIGITIZER_HAS_STYLUS true

// number of contacts that the device can read, only the first 5 are
// sent to the OS but the rest can be used internally
#define DIGITIZER_CONTACT_COUNT 16

// for printing floats
#undef PRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1

// high polling rate https://github.com/george-norton/qmk_userspace/blob/multitouch_experiment/keyboards/ploopyco/pavonis/keymaps/high_polling_rate/config.h
#undef MXT_GAIN
#define MXT_GAIN 8
#undef MXT_TOUCH_THRESHOLD
#define MXT_TOUCH_THRESHOLD 32
#undef MXT_TOUCH_HYST
#define MXT_TOUCH_HYST 16
#undef MXT_INTERNAL_TOUCH_THRESHOLD
#define MXT_INTERNAL_TOUCH_THRESHOLD 12
#undef MXT_INTERNAL_TOUCH_HYST
#define MXT_INTERNAL_TOUCH_HYST 6

#undef MXT_IDLE_SYNCS_PER_X
#define MXT_IDLE_SYNCS_PER_X 0
#undef MXT_ACTIVE_SYNCS_PER_X
#define MXT_ACTIVE_SYNCS_PER_X 0
#undef MXT_T56_SHIELDLESS_ENABLE
#undef MXT_T65_LENS_BENDING_ENABLE
#undef MXT_LOW_PASS_FILTER_COEFFICIENT
#define MXT_LOW_PASS_FILTER_COEFFICIENT 0
#undef MXT_CHARGE_TIME
#define MXT_CHARGE_TIME 8
#undef MXT_RETRANSMISSION_COMPENSATION_ENABLE
#define MXT_RETRANSMISSION_COMPENSATION_ENABLE 0

#define MXT_CONFTHR 5 // Increase debounce time

#define MXT_MOVE_HYSTERESIS_NEXT 16
#define MXT_MOVE_HYSTERESIS_INITIAL 50
