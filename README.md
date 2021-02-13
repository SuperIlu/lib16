# libsixteen (aka lib16)
## Small helper library for MS-DOS

Libsixteen provides helper functions for VGA mode 0x13 graphics, BMP loading/saving, mouse pointers and IPX networking.
It was written just for fun using [OpenWatcom](https://github.com/open-watcom).

VGA code is based on the [256-Color VGA Programming in C](http://www.brackeen.com/vga/index.html) tutorial by David Brackeen

IPX code was developed with the help of the Cylindrix [source](https://github.com/hyperlogic/cylindrix/blob/master/src/legacy/jonipx.c).

### files/directories
```
|
+- fonts/	example fonts and a Python script to convert TTF to BMP.
+- lib/		library source files
+- prj01/	showcase for all the implemented functions
+- prj02/	multi player drawing canvas using IPX
+- LICENSE	license description for all parts provided
```

I have included the OpenWatcom project files as a reference. You need to change these if you want to compile the examples yourself because OpenWatcom includes absolute paths in these files.

## Fonts
### Converter
font_converter.py can be used to create BMP fonts for `bitmap_render_char()` and `bitmap_render_string()`.
It needs at least Python 3.6 and PyGame.

### "Computer" and "Magic 5"
These font are kindly included with permission of DamienG https://damieng.com/typography/zx-origins/
Make sure to check his site for more awesome fonts.

## Contact
You can find me on [Twitter](https://twitter.com/dec_hl) if you want...
