/**
 * @file bitmap.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief BMP loading/saving/rendering incl. font rendering
 *
 * @copyright SuperIlu
 */
#include <stdlib.h>
#include <stdio.h>
#include <mem.h>

#include "error.h"
#include "bitmap.h"

/* ======================================================================
** defines
** ====================================================================== */
#define BMP_INFO_HEADER_SIZE 40  //!< size of the info header
#define BMP_NUM_PLANES 1         //!< single plane only
#define BMP_BPP 8                //!< eight bit/pixel only
#define BMP_COMPRESSION_NONE 0   //!< uncompressed images only
#define BMP_COLORS 256           //!< palette must have 256 entries
#define BMP_NUM_CHARS 95         //!< number of characters rendered by font_convert.py (SPACE..TILDE)
#define BMP_SCANLINE_PADDING 4   //!< scanlines in BMP files are a multiple of 4

/* ======================================================================
** typedefs
** ====================================================================== */
//! BMP image header (http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm)
typedef struct __bmp_header {
    uint8_t B;
    uint8_t M;
    uint32_t file_size;
    uint32_t reserved01;
    uint32_t data_offset;

    uint32_t info_header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bit_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_pixels_per_m;
    uint32_t y_pixels_per_m;
    uint32_t num_colors;
    uint32_t important_colors;
} bmp_header_t;

//! BMP image color table entry (order of the colors is wrong in the HTML mentioned above!)
typedef struct __bmp_color {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved02;
} bmp_color_t;

/**
 * @brief load an uncompressed, 8bit BMP from disk.
 *
 * @param fname file name
 * @param palette true to also load the palette, false to just load the image data.
 * @return a bitmap_t or NULL if loading fails.
 */
bitmap_t *bitmap_load(char *fname, bool palette) {
    uint16_t offset;
    int i;
    FILE *f;
    bitmap_t *bm = NULL;
    bmp_header_t header;
    bmp_color_t color;

    f = fopen(fname, "rb");
    if (!f) {
        ERR_NOENT();
        return NULL;
    }

    // read header
    if ((i = fread(&header, sizeof(bmp_header_t), 1, f)) != 1) {
        ERR_IOERR();
        fclose(f);
        return NULL;
    }

    // check for "BM" and right format
    if ((header.B != 'B') || (header.M != 'M') || (header.info_header_size != BMP_INFO_HEADER_SIZE) || (header.planes != BMP_NUM_PLANES) || (header.bit_per_pixel != BMP_BPP) ||
        (header.compression != BMP_COMPRESSION_NONE)) {
        ERR_PARAM();
        fclose(f);
        return NULL;
    }

    // create bitmap
    bm = bitmap_create(header.width, header.height, palette ? header.num_colors : 0);
    if (!bm) {
        ERR_NOMEM();
        fclose(f);
        return NULL;
    }

    // load palette (if wanted), if not skip data
    if (palette) {
        for (i = 0; i < header.num_colors; i++) {
            if (fread(&color, sizeof(bmp_color_t), 1, f) != 1) {
                ERR_IOERR();
                bitmap_free(bm);
                fclose(f);
                return NULL;
            }
            bm->palette[i].red = color.red;
            bm->palette[i].green = color.green;
            bm->palette[i].blue = color.blue;
        }
    } else {
        fseek(f, sizeof(bmp_color_t) * header.num_colors, SEEK_CUR);
    }

    // load image data
    offset = bm->width * (bm->height - 1);
    for (i = 0; i < bm->height; i++, offset -= bm->width) {
        if (fread(&bm->data[offset], bm->width, 1, f) != 1) {
            ERR_IOERR();
            bitmap_free(bm);
            fclose(f);
            return NULL;
        }
        // scanlines are padded to multiples of 4
        if (bm->width % BMP_SCANLINE_PADDING) {
            fseek(f, BMP_SCANLINE_PADDING - (bm->width % BMP_SCANLINE_PADDING), SEEK_CUR);
        }
    }

    // all done and ok
    fclose(f);
    ERR_OK();
    return bm;
}

/**
 * @brief save an uncompressed, 8bit BMP to disk.
 *
 * @param bm pointer to the bitmap. It must contain a pallete with 256 colors!
 * @param fname file name
 *
 * @return true if the image could be saved, else false.
 */
bool bitmap_save(bitmap_t *bm, const char *fname) {
    uint16_t data_size, offset;
    int i, p;
    FILE *f;
    bmp_header_t header;
    bmp_color_t color;

    if (!bm->palette || (bm->num_colors != BMP_COLORS)) {
        ERR_PARAM();
        return false;
    }

    f = fopen(fname, "wb");
    if (!f) {
        ERR_CREAT();
        return false;
    }

    // create header
    data_size = bm->width * bm->height;

    header.B = 'B';
    header.M = 'M';
    header.file_size = sizeof(bmp_header_t) + BMP_COLORS * sizeof(bmp_color_t) + data_size;
    header.reserved01 = 0x00;
    header.data_offset = sizeof(bmp_header_t) + BMP_COLORS * sizeof(bmp_color_t);
    header.info_header_size = BMP_INFO_HEADER_SIZE;
    header.width = bm->width;
    header.height = bm->height;
    header.planes = BMP_NUM_PLANES;
    header.bit_per_pixel = BMP_BPP;
    header.compression = BMP_COMPRESSION_NONE;
    header.image_size = bm->width * bm->height;
    header.x_pixels_per_m = 0xB12;  // (0xB12 = 72 dpi)
    header.y_pixels_per_m = 0xB12;  // (0xB12 = 72 dpi)
    header.num_colors = BMP_COLORS;
    header.important_colors = 0;

    // write header
    if (fwrite(&header, sizeof(bmp_header_t), 1, f) != 1) {
        ERR_IOERR();
        fclose(f);
        remove(fname);
        return false;
    }

    // write color
    color.reserved02 = 0x00;
    for (i = 0; i < BMP_COLORS; i++) {
        color.red = bm->palette[i].red;
        color.green = bm->palette[i].green;
        color.blue = bm->palette[i].blue;

        if (fwrite(&color, sizeof(bmp_color_t), 1, f) != 1) {
            ERR_IOERR();
            fclose(f);
            remove(fname);
            return false;
        }
    }

    // write data
    offset = bm->width * (bm->height - 1);
    for (i = 0; i < bm->height; i++, offset -= bm->width) {
        if (fwrite(&bm->data[offset], bm->width, 1, f) != 1) {
            ERR_IOERR();
            fclose(f);
            remove(fname);
            return false;
        }
        // scanlines are padded to multiples of 4
        if (bm->width % BMP_SCANLINE_PADDING) {
            for (p = 0; p < BMP_SCANLINE_PADDING - (bm->width % BMP_SCANLINE_PADDING); p++) {
                fputc(0x00, f);
            }
        }
    }

    // all done and ok
    fclose(f);
    ERR_OK();
    return true;
}

/**
 * @brief create a bitmap of given size with all pixel set to 0 and the given number of colors in the palette.
 *
 * @param width wanted width
 * @param height wanted height
 * @param palette_colors number of color in the palette or 0 for no palette.
 *
 * @return bitmap_t* a new bitmap or NULL if out of memory.
 */
bitmap_t *bitmap_create(uint16_t width, uint16_t height, uint16_t palette_colors) {
    bitmap_t *bm;

    // get bitmap_t
    bm = calloc(sizeof(bitmap_t), 1);
    if (!bm) {
        ERR_NOMEM();
        return NULL;
    }
    bm->width = width;
    bm->height = height;
    bm->ch_width = bm->width / BMP_NUM_CHARS;  // calculate character width

    // alloc pixel data
    bm->data = calloc(width, height);
    if (!bm->data) {
        bitmap_free(bm);
        ERR_NOMEM();
        return NULL;
    }

    // alloc palette if requested
    if (palette_colors) {
        bm->num_colors = palette_colors;
        bm->palette = calloc(sizeof(palette_color_t), palette_colors);
        if (!bm->palette) {
            bitmap_free(bm);
            ERR_NOMEM();
            return NULL;
        }
    }
    ERR_OK();
    return bm;
}

/**
 * @brief free the memory for a bitmap.
 *
 * @param bm the bitmap pointer or NULL.
 */
void bitmap_free(bitmap_t *bm) {
    if (bm) {
        if (bm->data) {
            free(bm->data);
            bm->data = NULL;
        }
        if (bm->palette) {
            free(bm->palette);
            bm->palette = NULL;
        }

        free(bm);
    }
}

/**
 * @brief copy screen data into a new bitmap.
 *
 * @param x screen start x
 * @param y screen start y
 * @param width width of the new image
 * @param height height of the new image
 * @param palette true if the palette should be copied as well.
 *
 * @return a bitmap_t with a copy from the screen or NULL if out of memory or x_width/y+height are out of bounds.
 */
bitmap_t *bitmap_copy(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool palette) {
    uint32_t screen_offset = (y << 8) + (y << 6);
    uint16_t bitmap_offset = 0;
    int copy_x, copy_y;
    bitmap_t *bm;

    // check parameters
    if ((x + width > VGA_SCREEN_WIDTH) || (y + height > VGA_SCREEN_HEIGHT)) {
        ERR_PARAM();
        return NULL;
    }

    // create empty bitmap
    bm = bitmap_create(width, height, palette ? VGA_MAX_COLORS : 0);
    if (!bm) {
        ERR_NOMEM();
        return NULL;
    }

    // copy palette from VGA registers
    if (palette) {
        vga_get_palette(bm->palette, bm->num_colors);
    }

    // copy data
    for (copy_y = 0; copy_y < height; copy_y++) {
        for (copy_x = 0; copy_x < width; copy_x++, bitmap_offset++) {
            bm->data[bitmap_offset] = VGA_MEMORY[(uint16_t)(screen_offset + x + copy_x)];
        }
        screen_offset += VGA_SCREEN_WIDTH;
    }
    ERR_OK();
    return bm;
}

/**
 * @brief draw a bitmap to the screen.
 *
 * @param bm the bitmap to draw.
 * @param x screen x pos
 * @param y screen y pos
 * @param apply_colors true to apply the bitmap palette, false to keep the current.
 *
 * @return true if all ok or false if  x_width/y+height are out of bounds.
 */
bool bitmap_draw(bitmap_t *bm, uint16_t x, uint16_t y, bool apply_colors) {
    uint16_t screen_offset = (y << 8) + (y << 6) + x;
    uint16_t bitmap_offset = 0;
    int j;

    // check parameters
    if ((x + bm->width > VGA_SCREEN_WIDTH) || (y + bm->height > VGA_SCREEN_HEIGHT)) {
        ERR_PARAM();
        return false;
    }

    if (apply_colors && bm->palette) {
        vga_set_palette(bm->palette, bm->num_colors);
    }

    // copy data
    for (j = 0; j < bm->height; j++) {
        memcpy(&VGA_MEMORY[screen_offset], &bm->data[bitmap_offset], bm->width);

        bitmap_offset += bm->width;
        screen_offset += VGA_SCREEN_WIDTH;
    }

    ERR_OK();
    return true;
}

/**
 * @brief change the color in a bitmap. This function is a NOP if the bitmap has no palette or idx is out of range.
 *
 * @param bm the bitmap pointer
 * @param idx the index of the color
 * @param c the new color
 */
void bitmap_set_color(bitmap_t *bm, uint8_t idx, palette_color_t *c) {
    if (bm->palette && idx < bm->num_colors) {
        bm->palette[idx].red = c->red;
        bm->palette[idx].green = c->green;
        bm->palette[idx].blue = c->blue;
    }
}

/**
 * @brief render a single character from a bitmap font to screen
 *
 * @param bm the bitmap to use as font
 * @param x x pos
 * @param y y pos
 * @param ch the character to render.
 * @param c color to use for rendering
 *
 * @return width of the character rendered or 0 if nothing was rendered.
 */
uint16_t bitmap_render_char(bitmap_t *bm, uint16_t x, uint16_t y, char ch, color_t c) {
    uint16_t ch_offset;
    uint16_t screen_offset = (y << 8) + (y << 6) + x;
    uint16_t bitmap_offset;
    int16_t ch_idx = ((uint8_t)ch) - ((uint8_t)' ');  // space is first character
    uint16_t j, w;

    if (bm->ch_width && (ch_idx >= 0) && (ch_idx < BMP_NUM_CHARS) && (x + bm->ch_width < VGA_SCREEN_WIDTH) && (y + bm->height < VGA_SCREEN_HEIGHT)) {  // check bounds
        ch_offset = ch_idx * bm->ch_width;
        bitmap_offset = ch_offset;
        // copy data
        for (j = 0; j < bm->height; j++) {
            for (w = 0; w < bm->ch_width; w++) {
                if (bm->data[bitmap_offset + w]) {
                    VGA_MEMORY[screen_offset + x + w] = c;
                }
            }
            bitmap_offset += bm->width;
            screen_offset += VGA_SCREEN_WIDTH;
        }
        return bm->ch_width;
    } else {
        return 0;  // nothing was rendered
    }
}

/**
 * @brief render a string to screen using a bitmap font. Multi line strings can be rendered using '\n' in the string.
 *
 * @param bm the bitmap to use as font
 * @param x x pos
 * @param y y pos
 * @param str the string to render.
 * @param c color to use for rendering
 *
 * @return width of the rendered string. For multi line string this is the width of the last line.
 */
uint16_t bitmap_render_string(bitmap_t *bm, uint16_t x, uint16_t y, char *str, color_t c) {
    uint16_t xPos = x;
    uint16_t yPos = y;
    while (*str) {
        if (*str == '\n') {
            xPos = x;
            yPos += bm->height;
        } else if (*str != '\r') {
            xPos += bitmap_render_char(bm, xPos, yPos, *str, c);
        }
        str++;
    }
    return xPos - x;
}
