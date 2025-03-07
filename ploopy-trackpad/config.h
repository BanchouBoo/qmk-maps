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
