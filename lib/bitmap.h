/**
 * @file bitmap.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief BMP loading/saving/rendering incl. font rendering
 *
 * @copyright SuperIlu
 */
#ifndef __BITMAP_H_
#define __BITMAP_H_

#include <stdint.h>

#include "vga.h"

//! bitmap structure
typedef struct __bitmap {
    uint16_t width;            // bitmap width
    uint16_t height;           // bitmap height
    uint16_t ch_width;         // bm width divided by number of characters in case this is a font bitmap
    uint16_t num_colors;       // number of colors in palette
    palette_color_t *palette;  // pointer to palette or NULL
    uint8_t *data;             // pointer to bitmap data
} bitmap_t;

extern bitmap_t *bitmap_load(char *fname, bool palette);
extern bool bitmap_save(bitmap_t *bm, const char *fname);
extern bitmap_t *bitmap_create(uint16_t width, uint16_t height, uint16_t palette);
extern bitmap_t *bitmap_copy(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool palette);
extern void bitmap_set_color(bitmap_t *bm, uint8_t idx, palette_color_t *color);
extern void bitmap_free(bitmap_t *bm);
extern bool bitmap_draw(bitmap_t *bm, uint16_t x, uint16_t y, bool apply_colors);
extern uint16_t bitmap_render_char(bitmap_t *bm, uint16_t x, uint16_t y, char ch, color_t c);
extern uint16_t bitmap_render_string(bitmap_t *bm, uint16_t x, uint16_t y, char *str, color_t c);

#endif  // __BITMAP_H_
