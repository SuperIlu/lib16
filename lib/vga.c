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
#include <stddef.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <mem.h>
#include <math.h>

#include "vga.h"
#include "mouse.h"
#include "error.h"

/* ======================================================================
** defines
** ====================================================================== */
#define INT_VBIOS 0x10                    //!< video BIOS INT
#define INT_VBIOS_SET_MODE 0x00           //!< set mode FUNC
#define INT_VBIOS_GET_COMBINATION 0x1A00  //!< get video combination FUNC

#define TEXT_80 0x03  //!< 80 column text mode
#define VGA_256 0x13  //!< 320x240 VGA mode

#define VGA_MONOCHROME 0x07  //!< VGA w/ monochrome analog display
#define VGA_COLOR 0x08       //!< VGA w/ color analog display

#define VGA_READ_PALETTE_INDEX 0x03C7   //!< VGA palette index read register
#define VGA_WRITE_PALETTE_INDEX 0x03C8  //!< VGA palette index write register
#define VGA_PALETTE_DATA 0x03C9         //!< VGA palette data register

#define VGA_INPUT_STATUS 0x03da  //!< VGA status register
#define VGA_VRETRACE 0x08        //!< VGA status 'retrace'

#define VGA_COLOR_SHIFT 2  //!< shift 'normal' 8bit colors to VGA 6bit colors

#define VGA_SINACOS_TABLE_SIZE 1024      //!< size of the sin/acos lookup table
#define VGA_FIXED_POINT_FACTOR 0x10000L  //!< factor for the fixed point implementation

//! extract sign of a number
#define VGA_SIGN(x) ((x < 0) ? -1 : ((x > 0) ? 1 : 0))

/* ======================================================================
** typedefs
** ====================================================================== */
typedef long fixed16_16;  //!< used for fixed-point math

/* ======================================================================
** global variables
** ====================================================================== */
//! pointer to VGA memory
uint8_t *VGA_MEMORY = (uint8_t *)0xA0000000L;

/* ======================================================================
** local variables
** ====================================================================== */
//! pre calculated sin(acos()) table
static fixed16_16 SIN_ACOS[VGA_SINACOS_TABLE_SIZE];

/* ======================================================================
** private functions
** ====================================================================== */
/**
 * @brief call BIOS set mode.
 *
 * @param mode the wanted mode.
 */
static void vga_set_mode(int mode) {
    union REGS regs;
    regs.h.al = mode;

    regs.h.ah = INT_VBIOS_SET_MODE;
    int86(INT_VBIOS, &regs, &regs);
}

void vga_wait_for_retrace(void) {
    /* wait until done with vertical retrace */
    while ((inp(VGA_INPUT_STATUS) & VGA_VRETRACE)) {
        ;
    }
    /* wait until done refreshing */
    while (!(inp(VGA_INPUT_STATUS) & VGA_VRETRACE)) {
        ;
    }
}
/* ======================================================================
** public functions
** ====================================================================== */
/**
 * @brief switch to VGA 320x240 256 colors.
 *
 * @return true if VGA was available and activated, else false.
 */
bool vga_init(void) {
    uint16_t i;
    union REGS regs;

    regs.x.ax = INT_VBIOS_GET_COMBINATION;
    int86(INT_VBIOS, &regs, &regs);

    // check of VGA is available
    if ((regs.h.bl != VGA_MONOCHROME) && (regs.h.bl != VGA_COLOR)) {
        ERR_AVAIL();
        return false;
    }

    vga_set_mode(VGA_256);

    // create the sin(arccos(x)) table.
    for (i = 0; i < VGA_SINACOS_TABLE_SIZE; i++) {
        SIN_ACOS[i] = sin(acos((float)i / VGA_SINACOS_TABLE_SIZE)) * VGA_FIXED_POINT_FACTOR;
    }

    ERR_OK();
    return true;
}

/**
 * @brief switch back to 80 column text mode.
 */
void vga_exit(void) { vga_set_mode(TEXT_80); }

/**
 * @brief set the whole VGA palette.
 *
 * @param palette pointer to an array of colors.
 * @param size number of colors in the palette.
 */
void vga_set_palette(palette_color_t *palette, uint16_t size) {
    int i;
    outp(VGA_WRITE_PALETTE_INDEX, 0);
    for (i = 0; (i < VGA_MAX_COLORS) && (i < size); i++) {
        outp(VGA_PALETTE_DATA, palette[i].red >> VGA_COLOR_SHIFT);
        outp(VGA_PALETTE_DATA, palette[i].green >> VGA_COLOR_SHIFT);
        outp(VGA_PALETTE_DATA, palette[i].blue >> VGA_COLOR_SHIFT);
    }
}

/**
 * @brief set a single color in the VGA palette.
 *
 * @param idx color index.
 * @param c the new color.
 */
void vga_set_color(uint16_t idx, palette_color_t *c) {
    outp(VGA_WRITE_PALETTE_INDEX, idx);

    outp(VGA_PALETTE_DATA, c->red >> VGA_COLOR_SHIFT);
    outp(VGA_PALETTE_DATA, c->green >> VGA_COLOR_SHIFT);
    outp(VGA_PALETTE_DATA, c->blue >> VGA_COLOR_SHIFT);
}

/**
 * @brief get the whole VGA palette.
 *
 * @param palette pointer to memory to hold the colors.
 * @param size max number  of colors to copy.
 */
void vga_get_palette(palette_color_t *palette, uint16_t size) {
    int i;
    outp(VGA_READ_PALETTE_INDEX, 0);
    for (i = 0; (i < VGA_MAX_COLORS) && (i < size); i++) {
        palette[i].red = inp(VGA_PALETTE_DATA) << VGA_COLOR_SHIFT;
        palette[i].green = inp(VGA_PALETTE_DATA) << VGA_COLOR_SHIFT;
        palette[i].blue = inp(VGA_PALETTE_DATA) << VGA_COLOR_SHIFT;
    }
}

/**
 * @brief get a single color from the VGA palette.
 *
 * @param idx color index.
 * @param c the memory to read to
 */
void vga_get_color(uint16_t idx, palette_color_t *c) {
    outp(VGA_READ_PALETTE_INDEX, idx);

    c->red = inp(VGA_PALETTE_DATA) << VGA_COLOR_SHIFT;
    c->green = inp(VGA_PALETTE_DATA) << VGA_COLOR_SHIFT;
    c->blue = inp(VGA_PALETTE_DATA) << VGA_COLOR_SHIFT;
}

/**
 * @brief set a grayscale palette.
 * 000..063 := black..gray..white
 * 064..127 := black..red
 * 128..191 := black..green
 * 192..255 := black..blue
 */
void vga_grayscale_palette() {
    int i;
    outp(VGA_WRITE_PALETTE_INDEX, 0);
    for (i = 0; i < VGA_MAX_COLORS / 4; i++) {
        outp(VGA_PALETTE_DATA, i);
        outp(VGA_PALETTE_DATA, i);
        outp(VGA_PALETTE_DATA, i);
    }
    for (i = 0; i < VGA_MAX_COLORS / 4; i++) {
        outp(VGA_PALETTE_DATA, i);
        outp(VGA_PALETTE_DATA, 0);
        outp(VGA_PALETTE_DATA, 0);
    }
    for (i = 0; i < VGA_MAX_COLORS / 4; i++) {
        outp(VGA_PALETTE_DATA, 0);
        outp(VGA_PALETTE_DATA, i);
        outp(VGA_PALETTE_DATA, 0);
    }
    for (i = 0; i < VGA_MAX_COLORS / 4; i++) {
        outp(VGA_PALETTE_DATA, 0);
        outp(VGA_PALETTE_DATA, 0);
        outp(VGA_PALETTE_DATA, i);
    }
}

/**
 * @brief draw a pixel on screen.
 *
 * @param x x position.
 * @param y y position.
 * @param c color index.
 */
void vga_set_pixel(uint16_t x, uint16_t y, color_t c) { VGA_MEMORY[(y << 8) + (y << 6) + x] = c; }

/**
 * @brief get a pixel from screen.
 *
 * @param x x position.
 * @param y y position.
 *
 * @return color index.
 */
color_t vga_get_pixel(uint16_t x, uint16_t y) { return VGA_MEMORY[(y << 8) + (y << 6) + x]; }

/**
 * @brief draw a line on screen
 *
 * @param x1 x start
 * @param y1 y start
 * @param x2 x end
 * @param y2 y end
 * @param c color index.
 */
void vga_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t c) {
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1; /* the horizontal distance of the line */
    dy = y2 - y1; /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = VGA_SIGN(dx);
    sdy = VGA_SIGN(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    vga_set_pixel(px, py, c);

    if (dxabs >= dyabs) { /* the line is more horizontal than vertical */
        for (i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            vga_set_pixel(px, py, c);
        }
    } else { /* the line is more vertical than horizontal */
        for (i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            vga_set_pixel(px, py, c);
        }
    }
}

/**
 * @brief draw a polygon onto the screen.
 *
 * @param vertices a array of vertices.
 * @param num_vertices number of vertices in the array.
 * @param c color index.
 */
extern void vga_polygon(vertex_t *vertices, uint16_t num_vertices, color_t c) {
    int i;

    for (i = 0; i < num_vertices - 1; i++) {
        vga_line(vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y, c);
    }
    vga_line(vertices[0].x, vertices[0].y, vertices[num_vertices - 1].x, vertices[num_vertices - 1].y, c);
}

/**
 * @brief draw a rectangle (outline).
 *
 * @param left x start
 * @param top y start
 * @param right x end
 * @param bottom y end
 * @param c color index.
 */
void vga_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, color_t c) {
    uint16_t top_offset, bottom_offset, i, temp;

    if (top > bottom) {
        temp = top;
        top = bottom;
        bottom = temp;
    }
    if (left > right) {
        temp = left;
        left = right;
        right = temp;
    }

    top_offset = (top << 8) + (top << 6);
    bottom_offset = (bottom << 8) + (bottom << 6);

    for (i = left; i <= right; i++) {
        VGA_MEMORY[top_offset + i] = c;
        VGA_MEMORY[bottom_offset + i] = c;
    }
    for (i = top_offset; i <= bottom_offset; i += VGA_SCREEN_WIDTH) {
        VGA_MEMORY[left + i] = c;
        VGA_MEMORY[right + i] = c;
    }
}

/**
 * @brief draw a filled rectangle.
 *
 * @param left x start
 * @param top y start
 * @param right x end
 * @param bottom y end
 * @param c color index.
 */
void vga_filled_rect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, color_t c) {
    uint16_t top_offset, bottom_offset, i, temp, width;

    if (top > bottom) {
        temp = top;
        top = bottom;
        bottom = temp;
    }
    if (left > right) {
        temp = left;
        left = right;
        right = temp;
    }

    top_offset = (top << 8) + (top << 6) + left;
    bottom_offset = (bottom << 8) + (bottom << 6) + left;
    width = right - left + 1;

    for (i = top_offset; i <= bottom_offset; i += VGA_SCREEN_WIDTH) {
        memset(&VGA_MEMORY[i], c, width);
    }
}

/**
 * @brief hide the mouse pointer before updating the screen. restores pixels previously saved by vga_show_mouse().
 *
 * @param mouse the mouse info struct from mouse_init()
 */
void vga_hide_mouse(mouse_t *mouse) {
    int x, y;
    int mx = mouse->x - mouse->cursor->x;
    int my = mouse->y - mouse->cursor->y;
    uint32_t screen_offset = (my << 8) + (my << 6);
    uint16_t bitmap_offset = 0;

    vga_wait_for_retrace();
    for (y = 0; y < MOUSE_CURSOR_HEIGHT; y++) {
        for (x = 0; x < MOUSE_CURSOR_WIDTH; x++, bitmap_offset++) {
            /* check for screen boundries */
            if (mx + x < VGA_SCREEN_WIDTH && mx + x >= 0 && my + y < VGA_SCREEN_HEIGHT && my + y >= 0) {
                VGA_MEMORY[(uint16_t)(screen_offset + mx + x)] = mouse->under->img[bitmap_offset];
            }
        }

        screen_offset += VGA_SCREEN_WIDTH;
    }
}

/**
 * @brief draw the mouse pointer over the current screen content. modified pixels are saved for restauration by vga_hide_mouse().
 *
 * @param mouse the mouse info struct from mouse_init()
 */
void vga_show_mouse(mouse_t *mouse) {
    int x, y;
    int mx = mouse->x - mouse->cursor->x;
    int my = mouse->y - mouse->cursor->y;
    uint32_t screen_offset = (my << 8) + (my << 6);
    uint16_t bitmap_offset = 0;
    uint8_t data;

    for (y = 0; y < MOUSE_CURSOR_HEIGHT; y++) {
        for (x = 0; x < MOUSE_CURSOR_WIDTH; x++, bitmap_offset++) {
            mouse->under->img[bitmap_offset] = VGA_MEMORY[(uint16_t)(screen_offset + mx + x)];
            /* check for screen boundries */
            if (mx + x < VGA_SCREEN_WIDTH && mx + x >= 0 && my + y < VGA_SCREEN_HEIGHT && my + y >= 0) {
                data = mouse->cursor->img[bitmap_offset];
                if (data) {
                    VGA_MEMORY[(uint16_t)(screen_offset + mx + x)] = data;
                }
            }
        }
        screen_offset += VGA_SCREEN_WIDTH;
    }
}

/**
 * @brief draw a circle (outline).
 *
 * @param x center x position.
 * @param y center y position.
 * @param radius radius.
 * @param c color index.
 */
void vga_circle(uint16_t x, uint16_t y, uint16_t radius, color_t color) {
    fixed16_16 n = 0, invradius = (1 / (float)radius) * VGA_FIXED_POINT_FACTOR;
    int dx = 0, dy = radius - 1;
    uint16_t dxoffset, dyoffset, offset = (y << 8) + (y << 6) + x;

    while (dx <= dy) {
        dxoffset = (dx << 8) + (dx << 6);
        dyoffset = (dy << 8) + (dy << 6);
        VGA_MEMORY[offset + dy - dxoffset] = color; /* octant 0 */
        VGA_MEMORY[offset + dx - dyoffset] = color; /* octant 1 */
        VGA_MEMORY[offset - dx - dyoffset] = color; /* octant 2 */
        VGA_MEMORY[offset - dy - dxoffset] = color; /* octant 3 */
        VGA_MEMORY[offset - dy + dxoffset] = color; /* octant 4 */
        VGA_MEMORY[offset - dx + dyoffset] = color; /* octant 5 */
        VGA_MEMORY[offset + dx + dyoffset] = color; /* octant 6 */
        VGA_MEMORY[offset + dy + dxoffset] = color; /* octant 7 */
        dx++;
        n += invradius;
        dy = (int)((radius * SIN_ACOS[(int)(n >> 6)]) >> 16);
    }
}

/**
 * @brief draw a filled circle.
 *
 * @param x center x position.
 * @param y center y position.
 * @param radius radius.
 * @param c color index.
 */
void vga_filled_circle(uint16_t x, uint16_t y, uint16_t radius, color_t color) {
    fixed16_16 n = 0, invradius = (1 / (float)radius) * VGA_FIXED_POINT_FACTOR;
    int dx = 0, dy = radius - 1, i;
    uint16_t dxoffset, dyoffset, offset = (y << 8) + (y << 6) + x;

    while (dx <= dy) {
        dxoffset = (dx << 8) + (dx << 6);
        dyoffset = (dy << 8) + (dy << 6);
        for (i = dy; i >= dx; i--, dyoffset -= VGA_SCREEN_WIDTH) {
            VGA_MEMORY[offset + i - dxoffset] = color;  /* octant 0 */
            VGA_MEMORY[offset + dx - dyoffset] = color; /* octant 1 */
            VGA_MEMORY[offset - dx - dyoffset] = color; /* octant 2 */
            VGA_MEMORY[offset - i - dxoffset] = color;  /* octant 3 */
            VGA_MEMORY[offset - i + dxoffset] = color;  /* octant 4 */
            VGA_MEMORY[offset - dx + dyoffset] = color; /* octant 5 */
            VGA_MEMORY[offset + dx + dyoffset] = color; /* octant 6 */
            VGA_MEMORY[offset + i + dxoffset] = color;  /* octant 7 */
        }
        dx++;
        n += invradius;
        dy = (int)((radius * SIN_ACOS[(int)(n >> 6)]) >> 16);
    }
}
