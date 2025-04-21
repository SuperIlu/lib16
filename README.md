# libsixteen (aka lib16)
## Small helper library for MS-DOS

Libsixteen provides helper functions for VGA mode 0x13 graphics, BMP loading/saving, mouse pointers, OPL2 music, raw disk access and IPX networking.
It was written just for fun using [OpenWatcom](https://github.com/open-watcom).

VGA code is based on the [256-Color VGA Programming in C](http://www.brackeen.com/vga/index.html) tutorial by David Brackeen

IPX code was developed with the help of the Cylindrix [source](https://github.com/hyperlogic/cylindrix/blob/master/src/legacy/jonipx.c).

OPL2 code was ported from [ArduinoOPL2](https://github.com/DhrBaksteen/ArduinoOPL2).

Be warned: the raw disk access code ist mostly untested, use at you own risk!

### files/directories
```
|
+- fonts/		example fonts and a Python script to convert TTF to BMP.
+- lib/			lib16 library source files
+- lua-5.4.7/	lua 5.4.7 source
+- prj01/		showcase for all the implemented functions
+- prj02/		multi player drawing canvas using IPX
+- prj03/		lib16 port of demotune.cpp
+- prj04/		example code using lua-5.4.7
+- LICENSE		license description for all parts provided
```

I have included the OpenWatcom project files as a reference. You need to change these if you want to compile the examples yourself because OpenWatcom includes absolute paths in these files.

The EXE were compiled for i386 w/ i387.

### Tunables/defines
#### VGA_DYNAMIC_TABLE
if defined the SIN/ACOS table for drawing circles is created dynamically when vga_init() is called. If undefined a pre-compiled table is used. Using the pre-compiled table can reduce EXE size when `sin()/acos()` is not used elsewhere.

### NO_ERRORS
If defined the `errno` functionality in `error.h/error.c` clone is disabled. This reduces EXE size.

## Fonts
### Converter
font_converter.py can be used to create BMP fonts for `bitmap_render_char()` and `bitmap_render_string()`.
It needs at least Python 3.6 and PyGame.

### "Computer" and "Magic 5"
These font are kindly included with permission of DamienG https://damieng.com/typography/zx-origins/
Make sure to check his site for more awesome fonts.

## Contact
You can find me on [Mastodon](https://mastodon.social/@dec_hl) if you want...
