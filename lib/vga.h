/**
 * @file vga.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief simple VGA access.
 *
 * @copyright David Brackeen
 * @copyright SuperIlu
 *
 * Most code was copied from http://www.brackeen.com/home/vga/
 */
#ifndef __VGA_H_
#define __VGA_H_

#include <stdbool.h>
#include <stdint.h>

#include "mouse.h"

/* ======================================================================
** defines
** ====================================================================== */
#define VGA_MAX_COLORS 256     //!< max number of colors
#define VGA_SCREEN_WIDTH 320   //< screen width
#define VGA_SCREEN_HEIGHT 200  //!< screen height

/* ======================================================================
** typedefs
** ====================================================================== */
//! color definition
typedef struct __color {
    uint8_t red;    //!< red
    uint8_t green;  //!< green
    uint8_t blue;   //!< blue
} palette_color_t;

//! a vertex for a polygon
typedef struct __vertex {
    uint16_t x;  //!< x coordinate
    uint16_t y;  //!< y coordinate
} vertex_t;

//! color index into the palette
typedef uint8_t color_t;

/* ======================================================================
** prototypes
** ====================================================================== */
extern uint8_t *VGA_MEMORY;

extern bool vga_init(void);
extern void vga_exit(void);
extern void vga_set_palette(palette_color_t *palette, uint16_t size);
extern void vga_set_color(uint16_t idx, palette_color_t *col);
extern void vga_get_palette(palette_color_t *palette, uint16_t size);
extern void vga_get_color(uint16_t idx, palette_color_t *c);
extern void vga_grayscale_palette();
extern void vga_set_pixel(uint16_t x, uint16_t y, color_t c);
extern color_t vga_get_pixel(uint16_t x, uint16_t y);
extern void vga_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t c);
extern void vga_polygon(vertex_t *vertices, uint16_t num_vertices, color_t c);
extern void vga_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, color_t c);
extern void vga_filled_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, color_t c);
extern void vga_hide_mouse(mouse_t *mouse);
extern void vga_show_mouse(mouse_t *mouse);
extern void vga_circle(uint16_t x, uint16_t y, uint16_t radius, color_t color);
extern void vga_filled_circle(uint16_t x, uint16_t y, uint16_t radius, color_t color);

#endif  // __VGA_H_
