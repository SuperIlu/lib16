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
#include <stdio.h>
#include <mem.h>
#include <math.h>

#include "vga.h"
#include "mouse.h"
#include "error.h"
#include "fixed.h"

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

#define VGA_SINACOS_TABLE_SIZE 1024  //!< size of the sin/acos lookup table

//! extract sign of a number
#define VGA_SIGN(x) ((x < 0) ? -1 : ((x > 0) ? 1 : 0))

/* ======================================================================
** global variables
** ====================================================================== */
//! pointer to VGA memory
uint8_t *VGA_MEMORY = (uint8_t *)0xA0000000L;

/* ======================================================================
** local variables
** ====================================================================== */
//! pre calculated sin(acos()) table
#ifdef VGA_DYNAMIC_TABLE
static fixed16_16 SIN_ACOS[VGA_SINACOS_TABLE_SIZE];
#else
static fixed16_16 SIN_ACOS[VGA_SINACOS_TABLE_SIZE] = {
    65536, 65535, 65535, 65535, 65535, 65535, 65534, 65534, 65533, 65533, 65532, 65532, 65531, 65530, 65529, 65528, 65527, 65526, 65525, 65524, 65523, 65522, 65520, 65519, 65517,
    65516, 65514, 65513, 65511, 65509, 65507, 65505, 65503, 65501, 65499, 65497, 65495, 65493, 65490, 65488, 65485, 65483, 65480, 65478, 65475, 65472, 65469, 65466, 65463, 65460,
    65457, 65454, 65451, 65448, 65444, 65441, 65437, 65434, 65430, 65427, 65423, 65419, 65415, 65411, 65407, 65403, 65399, 65395, 65391, 65387, 65382, 65378, 65373, 65369, 65364,
    65359, 65355, 65350, 65345, 65340, 65335, 65330, 65325, 65320, 65315, 65309, 65304, 65299, 65293, 65287, 65282, 65276, 65270, 65265, 65259, 65253, 65247, 65241, 65235, 65228,
    65222, 65216, 65210, 65203, 65197, 65190, 65183, 65177, 65170, 65163, 65156, 65149, 65142, 65135, 65128, 65121, 65114, 65106, 65099, 65091, 65084, 65076, 65069, 65061, 65053,
    65045, 65037, 65030, 65021, 65013, 65005, 64997, 64989, 64980, 64972, 64963, 64955, 64946, 64938, 64929, 64920, 64911, 64902, 64893, 64884, 64875, 64866, 64857, 64847, 64838,
    64829, 64819, 64809, 64800, 64790, 64780, 64771, 64761, 64751, 64741, 64731, 64720, 64710, 64700, 64690, 64679, 64669, 64658, 64647, 64637, 64626, 64615, 64604, 64593, 64582,
    64571, 64560, 64549, 64538, 64526, 64515, 64504, 64492, 64480, 64469, 64457, 64445, 64433, 64422, 64410, 64397, 64385, 64373, 64361, 64349, 64336, 64324, 64311, 64299, 64286,
    64273, 64261, 64248, 64235, 64222, 64209, 64196, 64183, 64169, 64156, 64143, 64129, 64116, 64102, 64088, 64075, 64061, 64047, 64033, 64019, 64005, 63991, 63977, 63963, 63948,
    63934, 63919, 63905, 63890, 63876, 63861, 63846, 63831, 63816, 63801, 63786, 63771, 63756, 63741, 63725, 63710, 63695, 63679, 63663, 63648, 63632, 63616, 63600, 63584, 63568,
    63552, 63536, 63520, 63504, 63487, 63471, 63454, 63438, 63421, 63405, 63388, 63371, 63354, 63337, 63320, 63303, 63286, 63269, 63251, 63234, 63216, 63199, 63181, 63164, 63146,
    63128, 63110, 63092, 63074, 63056, 63038, 63020, 63001, 62983, 62965, 62946, 62927, 62909, 62890, 62871, 62852, 62834, 62815, 62795, 62776, 62757, 62738, 62718, 62699, 62679,
    62660, 62640, 62621, 62601, 62581, 62561, 62541, 62521, 62501, 62481, 62460, 62440, 62419, 62399, 62378, 62358, 62337, 62316, 62295, 62274, 62253, 62232, 62211, 62190, 62169,
    62147, 62126, 62104, 62083, 62061, 62039, 62017, 61995, 61973, 61951, 61929, 61907, 61885, 61862, 61840, 61818, 61795, 61772, 61750, 61727, 61704, 61681, 61658, 61635, 61612,
    61589, 61565, 61542, 61518, 61495, 61471, 61447, 61424, 61400, 61376, 61352, 61328, 61304, 61280, 61255, 61231, 61206, 61182, 61157, 61133, 61108, 61083, 61058, 61033, 61008,
    60983, 60958, 60932, 60907, 60881, 60856, 60830, 60805, 60779, 60753, 60727, 60701, 60675, 60649, 60623, 60596, 60570, 60543, 60517, 60490, 60463, 60437, 60410, 60383, 60356,
    60329, 60301, 60274, 60247, 60219, 60192, 60164, 60137, 60109, 60081, 60053, 60025, 59997, 59969, 59941, 59912, 59884, 59855, 59827, 59798, 59769, 59741, 59712, 59683, 59654,
    59624, 59595, 59566, 59536, 59507, 59477, 59448, 59418, 59388, 59358, 59328, 59298, 59268, 59238, 59207, 59177, 59147, 59116, 59085, 59055, 59024, 58993, 58962, 58931, 58899,
    58868, 58837, 58805, 58774, 58742, 58711, 58679, 58647, 58615, 58583, 58551, 58519, 58486, 58454, 58421, 58389, 58356, 58323, 58291, 58258, 58225, 58191, 58158, 58125, 58092,
    58058, 58025, 57991, 57957, 57923, 57889, 57855, 57821, 57787, 57753, 57719, 57684, 57650, 57615, 57580, 57545, 57510, 57475, 57440, 57405, 57370, 57334, 57299, 57263, 57228,
    57192, 57156, 57120, 57084, 57048, 57012, 56975, 56939, 56902, 56866, 56829, 56792, 56755, 56718, 56681, 56644, 56607, 56569, 56532, 56494, 56457, 56419, 56381, 56343, 56305,
    56267, 56229, 56190, 56152, 56113, 56074, 56036, 55997, 55958, 55919, 55880, 55840, 55801, 55762, 55722, 55682, 55643, 55603, 55563, 55523, 55482, 55442, 55402, 55361, 55321,
    55280, 55239, 55198, 55157, 55116, 55075, 55034, 54992, 54951, 54909, 54867, 54825, 54783, 54741, 54699, 54657, 54614, 54572, 54529, 54487, 54444, 54401, 54358, 54315, 54271,
    54228, 54184, 54141, 54097, 54053, 54009, 53965, 53921, 53877, 53833, 53788, 53743, 53699, 53654, 53609, 53564, 53519, 53473, 53428, 53383, 53337, 53291, 53245, 53199, 53153,
    53107, 53061, 53014, 52968, 52921, 52874, 52827, 52780, 52733, 52686, 52638, 52591, 52543, 52495, 52447, 52399, 52351, 52303, 52255, 52206, 52158, 52109, 52060, 52011, 51962,
    51913, 51863, 51814, 51764, 51714, 51664, 51614, 51564, 51514, 51464, 51413, 51362, 51312, 51261, 51210, 51159, 51107, 51056, 51004, 50953, 50901, 50849, 50797, 50744, 50692,
    50639, 50587, 50534, 50481, 50428, 50375, 50322, 50268, 50215, 50161, 50107, 50053, 49999, 49944, 49890, 49835, 49781, 49726, 49671, 49616, 49560, 49505, 49449, 49394, 49338,
    49282, 49225, 49169, 49113, 49056, 48999, 48942, 48885, 48828, 48771, 48713, 48655, 48598, 48540, 48482, 48423, 48365, 48306, 48247, 48189, 48129, 48070, 48011, 47951, 47892,
    47832, 47772, 47712, 47651, 47591, 47530, 47469, 47408, 47347, 47286, 47224, 47163, 47101, 47039, 46977, 46914, 46852, 46789, 46726, 46663, 46600, 46537, 46473, 46409, 46345,
    46281, 46217, 46153, 46088, 46023, 45958, 45893, 45828, 45762, 45697, 45631, 45565, 45498, 45432, 45365, 45298, 45231, 45164, 45097, 45029, 44962, 44894, 44825, 44757, 44689,
    44620, 44551, 44482, 44412, 44343, 44273, 44203, 44133, 44063, 43992, 43921, 43850, 43779, 43708, 43636, 43564, 43492, 43420, 43347, 43275, 43202, 43129, 43055, 42982, 42908,
    42834, 42760, 42685, 42611, 42536, 42461, 42385, 42310, 42234, 42158, 42082, 42005, 41928, 41851, 41774, 41697, 41619, 41541, 41463, 41384, 41306, 41227, 41147, 41068, 40988,
    40908, 40828, 40748, 40667, 40586, 40505, 40423, 40341, 40259, 40177, 40094, 40011, 39928, 39845, 39761, 39677, 39593, 39508, 39423, 39338, 39253, 39167, 39081, 38995, 38908,
    38821, 38734, 38647, 38559, 38471, 38382, 38293, 38204, 38115, 38025, 37935, 37845, 37754, 37663, 37572, 37481, 37389, 37296, 37204, 37111, 37017, 36924, 36830, 36735, 36641,
    36545, 36450, 36354, 36258, 36161, 36065, 35967, 35870, 35772, 35673, 35574, 35475, 35375, 35275, 35175, 35074, 34973, 34871, 34769, 34667, 34564, 34461, 34357, 34253, 34148,
    34043, 33938, 33832, 33725, 33618, 33511, 33403, 33295, 33186, 33077, 32967, 32857, 32746, 32635, 32524, 32411, 32299, 32185, 32072, 31957, 31842, 31727, 31611, 31495, 31377,
    31260, 31142, 31023, 30903, 30783, 30663, 30542, 30420, 30297, 30174, 30051, 29926, 29801, 29676, 29549, 29422, 29294, 29166, 29037, 28907, 28776, 28645, 28513, 28380, 28247,
    28112, 27977, 27841, 27704, 27567, 27428, 27289, 27149, 27008, 26866, 26723, 26579, 26434, 26289, 26142, 25995, 25846, 25696, 25546, 25394, 25241, 25087, 24932, 24776, 24619,
    24460, 24301, 24140, 23977, 23814, 23649, 23483, 23316, 23147, 22977, 22805, 22632, 22457, 22281, 22103, 21924, 21743, 21560, 21375, 21189, 21000, 20810, 20618, 20424, 20228,
    20030, 19829, 19626, 19421, 19214, 19004, 18791, 18576, 18358, 18138, 17914, 17687, 17457, 17224, 16987, 16747, 16503, 16255, 16003, 15747, 15486, 15220, 14950, 14674, 14392,
    14105, 13811, 13511, 13204, 12889, 12566, 12233, 11892, 11539, 11176, 10799, 10409, 10003, 9580,  9136,  8669,  8175,  7649,  7084,  6468,  5786,  5012,  4093,  2895};
#endif  // VGA_DYNAMIC_TABLE

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

#ifdef VGA_DYNAMIC_TABLE
    // create the sin(arccos(x)) table.
    for (i = 0; i < VGA_SINACOS_TABLE_SIZE; i++) {
        SIN_ACOS[i] = TO_FIXED(sin(acos((float)i / VGA_SINACOS_TABLE_SIZE)));
    }
#endif  // VGA_DYNAMIC_TABLE

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
    fixed16_16 n = 0, invradius = TO_FIXED(1 / (float)radius);
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
    fixed16_16 n = 0, invradius = TO_FIXED(1 / (float)radius);
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
