## About

This is my personal repo for storing qmk keymaps, along with a script to clone qmk and compile and flash the firmware with support for multiple qmk forks for different keymaps. I was initially going to use [qmk_userspace](https://github.com/qmk/qmk_userspace) but that seemed like more of a headache than rolling my own setup like this instead.

## Iris

[Link](https://keeb.io/products/iris-keyboard-split-ergonomic-keyboard)

Dvorak default layout with a qwerty layer, a modified qwerty layer with spacebar on the left for games that have subpar/no key remapping (ideally though I remap any game I can to dvorak). The symbols layer contains easy access to things like parens, math operators, etc. and a steno layer for doing [stenography](https://www.youtube.com/watch?v=nRp_1S7cj6A).

## Ploopy Classic

[Link](https://ploopy.co/classic-trackball)

Swapped buttons around and wrote some kinda janky code for mouse acceleration but I'm happy with how it feels. Also using 2000 DPI but then using math to get an effective DPI of 400, allegedly 2000 DPI is the highest the sensor can go before smoothing starts getting applied and a higher DPI scaled down theoretically means a greater degree of accuracy in movement. In practice I haven't conciously noticed a difference, but it's certainly not hurting anything so I chose to keep it.

Eventually I'd like to rework the acceleration code.

## Ploopy Trackpad

[Link](https://ploopy.co/trackpad/)

My most extensively modified keymap. There are three modes set up currently: normal, mouse, and tablet. Different modes have various settings you can change by passing data when calling `change_mode`.

You can switch modes by pressing down a specific number of fingers:
- 6: Normal mode
- 7: Mouse mode
- 8: Tablet mode (if `DIGITIZER_HAS_STYLUS` is defined)
- 10: Puts the trackpad into the bootloader (if `MAXTOUCH_BOOTLOADER_GESTURE` is NOT defined)

- Normal: Standard out-of-the-box trackpad behavior, nothing special.
- Mouse: Emulates mouse input, first contact acts as the pointer, a contact to the left <4000 trackpad units is a left click, a contact to the left >=4000 trackpad units is a mouse side button, a contact to the right <4000 trackpad units is a right click, a contact >=4000 units to the right is a middle click. Currently also has some hardcoded acceleration.
  - Settings:
    - `sensitivity`: a flat multiplier applied to trackpad motion before translating it to mouse units. Defaults to 0.115, which is roughty equivalent to the normal trackpad tracking on Linux via libinput with a flat acceleration profile.
- Tablet: Absolute tracking. Currently only the first contact does anything, and the rest are ignored, but I may add clicking behavior like mouse mode has as well in the future. Currently requires `DIGITIZER_HAS_STYLUS` to be defined in `config.h`.
  - Settings:
    - `mode`: Currently the only supported mode is `MODE_RECTANGLE` but I would like to add support for circular/elliptical tracking in the future (see [here](https://github.com/Kuuuube/Circular_Area/blob/main/wiki/layman_s_guide_to_circular_area.md) for details)
    - `region`: a two-element array defining the width and height of the tablet region. Defaults to the entire trackpad area.
    - `shiftable`: a boolean to determine if smaller tracking regions can move when you push beyond their edge (see video below). Defaults to false.
    - `deadzone_x`: a two-element array ([left, right]) that defines the horizontal outer deadzone for the tracking area. This means that the tracking region will map to a smaller region of the screen instead of the entire screen. Defaults to [0, 0]
    - `deadzone_y`: a two-element array ([top, bottom]) that defines the vertical outer deadzone of the tracking area. Defaults to [0, 0]

https://github.com/user-attachments/assets/69ef7647-d692-40cd-a05a-93a5259300be
