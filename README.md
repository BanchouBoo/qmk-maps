## About

This is my personal repo for storing qmk keymaps, along with a script to clone qmk and compile and flash the firmware with support for multiple qmk forks for different keymaps. I was initially going to use [qmk_userspace](https://github.com/qmk/qmk_userspace) but that seemed like more of a headache than rolling my own setup like this instead.

## Iris

[Link](https://keeb.io/products/iris-keyboard-split-ergonomic-keyboard)

Dvorak default layout with a qwerty layer, a modified qwerty layer with spacebar on the left for games that have subpar/no key remapping (ideally though I remap any game I can to dvorak). The symbols layer contains easy access to things like parens, math operators, etc. and a steno layer for doing [stenography](https://www.youtube.com/watch?v=nRp_1S7cj6A).

## Ploopy Classic

[Link](https://ploopy.co/classic-trackball)

Swapped buttons around and wrote some kinda janky code for mouse acceleration but I'm happy with how it feels. Also using 2000 DPI but then using math to get an effective DPI of 400, allegedly 2000 DPI is the highest the sensor can go before smoothing starts getting applied and a higher DPI scaled down theoretically means a greater degree of accuracy in movement. In practice I haven't conciously noticed a difference, but it's certainly not hurting anything so I chose to keep it.
