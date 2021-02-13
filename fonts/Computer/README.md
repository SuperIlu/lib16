# A ZX-Origins Font

Thank you for downloading one of the fonts in my ZX Origins collection. You will see a number of files in this download for the various formats this font is supplied in. They are as follows:

## Formats

### Sinclair ZX Spectrum

Inside the **Spectrum** folder you will find each font has a `.ch8` file that is a RAW 768-byte file that can be loaded directly into BASIN, AGD or other Spectrum environments. Alternatively you can load it into any available RAM address and then `POKE 23606` and `23607` with the appropriate RAM address minus 256 bytes.

Each font also has a `.fzx` file containing a proportional conversion of the font for use with the [FZX Proportional Font Renderer](https://github.com/z88dk/z88dk/tree/master/libsrc/_DEVELOPMENT/font/fzx) available for the ZX Spectrum as well as in a z80 assembler compatible format containing `defb` statements in the `.z80.asm` files.

A `.tap` tape image is provided which can be either written to a physical tape, loaded in via a DivMMC or mounted in an emulator. It contains all the fonts for the given typeface complete with a built-in demonstration program that will allow all the fonts to be previewed in any color combination.

### Acorn BBC Micro/Electron 

Inside the **BBC** folder you will find each font has a `.bbc` file that can be copied to your own discs and `*EXEC`'ed once you have set `*FX 20,7` and carved out space for them with `PAGE=&1F00`. It is recommended you `*EXEC` them from your `!BOOT` file. If you need to execute them from your own BASIC then instead you should use the [BFont](http://mdfs.net/Apps/Font/) load routine by J.G.Harston in 0/VDU mode.

Additionally a DFS-formatted `.ssd` disk image is provided for use with emulators contains a demonstration program showing the fonts available for this typeface in any color combination in low, medium or high-resolution modes. 

### Commodore 64

The **C64** folder contains two `.64c` format files (raw in PETSCI order with a 2 byte extra header) - one with upper-case only and more line drawing characters (`.upper`) - and the other containing both upper and lower case (`.both`).

Additionally there is a combined `.bin` file containing both the fonts in a raw format that can be used as a character ROM in an emulator or otherwise imported into your dev system.

### Atari 8-bit

The **Atari8** folder contains a `.fnt` file for each font which is the RAW font data with ATASCII ordering with line-drawing characters from the original machine set.

### PC

### BDF (Glyph Bitmap Distribution Format)

Standard bitmap font format used by many Windows and Mac tools. If you need to extend the character set out I would start here as it allows for large character sets etc and is still purely bitmap.

### PSF (PC Screen Font)

Font file capable of being used by Linux as console fonts.

### TTF (TrueType font)

A Windows, Mac and Linux friendly scalable version of the font. It is very hard to preserve the original bitmap sharply across platforms in a TrueType container but I've done what I can. This looks best at multiples of 8px on Windows. Experiment with other sizes or metrics as you need depending on your OS.

## Further conversion

If you wish to convert this font to other formats the BDF is a good starting point. Alternatively check out John Elliot's PSFTOOLS which can turn the .psf file included into BBC, Wyse, C code, FNT and others formats. I may include additional formats in future updates of ZX Origins.

## Thanks

My sincere thanks to:

- John Elliot for his amazing [psftools](https://www.seasip.info/Unix/PSF/) which helped streamline the process.
- [Paul van der Laan](http://type-invaders.com) for lending his expertise and research into how to make 8x8 bitmap fonts perfect in FontLab Studio 5.
- Paul Dunn for the BASIN editor that is my 8x8 bitmap font designer of choice.
- J.G.Harston for the BBC font-loading BFontLoad routine and the mkimg tool used to automate .ssd files.

## Licence

This font is part of the ZX Origins font collection Copyright (c) 1988-2021 Damien Guard. 

Formal licenses are complicated and burdensome so here's the deal. These are some acceptable use examples:

1. Use it in your game (commercial or non-commercial)
2. Print something on a t-shirt
3. Use the embedded font file on your site
4. Set your favorite terminal or OS to use it

Ideally with a credit like "<fontname> font by DamienG https://damieng.com/zx-origins" *if* you have a credits section. If you don't that's fine. Either way dropping me an email at damieng@gmail.com to let me know what you used it for is appreciated.

The only unacceptable use is redistributing this font as a font. i.e. re-hosting the files on your own site or bundling it with other art assets. I have put a lot of time into these and my only reward is seeing download counts on my site which is ad-free so it seems unfair other people would re-host these files on their site and make $ from Google Adwords etc.

If you need to modify the font for your usage - either to add characters (this collection is pure ASCII + copyright + pound sign right now) or if a few are bothering you just change the credit to "Font based on <fontname> by DamienG" or something.

Thanks and enjoy!

[)amien
https://damieng.com/zx-origins/