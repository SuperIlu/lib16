/**
 * @file mouse.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief simple mouse access
 *
 * @copyright SuperIlu
 */
#include <stddef.h>
#include <dos.h>
#include <mem.h>

#include "vga.h"
#include "mouse.h"
#include "error.h"

/* ======================================================================
** defines
** ====================================================================== */
#define INT_MOUSE 0x33           //!< mouse INT
#define INT_MOUSE_RESET 0x00     //!< reset mouse FUNC
#define INT_MOUSE_UPDATE 0x03    //!< mouse update FUNC
#define INT_MOUSE_PRESSED 0x05   //!<  mouse pressed FUNC
#define INT_MOUSE_RELEASED 0x06  //!< mouse released FUNC

/* ======================================================================
** public variables
** ====================================================================== */
//! pre-defined cursor "cross"
const mouse_pointer_t MOUSE_CROSS = {
    6,                                   // hotspot X
    6,                                   // hotspot Y
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 1
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 2
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 3
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 4
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 5
    1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1,  // row 6
    1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1,  // row 7
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 8
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 9
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 0
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  // row 11
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0   // row 12
};

//! pre-defined cursor "pointer"
const mouse_pointer_t MOUSE_POINTER = {
    1,                                   // hotspot X
    1,                                   // hotspot Y
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // row 1
    0, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  // row 2
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // row 3
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // row 4
    0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  // row 5
    0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0,  // row 6
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,  // row 7
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  // row 8
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  // row 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // row 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // row 11
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // row 12
};

/* ======================================================================
** private variables
** ====================================================================== */
//! static buffer for mouse data updates
static mouse_t mouse_data;

/* ======================================================================
** public functions
** ====================================================================== */
/**
 * @brief init mouse.
 *
 * @param image image for the cursor.
 * @return mouse_t* if mouse driver was installed, else NULL.
 */
mouse_t* mouse_init(const mouse_pointer_t* image) {
    union REGS regs;

    regs.x.ax = INT_MOUSE_RESET;
    int86(INT_MOUSE, &regs, &regs);

    // check if driver is installed
    if (!regs.x.ax) {
        ERR_DRIVR();
        return NULL;
    }

    mouse_update(false);
    memcpy(&mouse_data.cursor, image, sizeof(mouse_pointer_t));
    ERR_OK();
    return &mouse_data;
}

/**
 * @brief update the data in mouse_t
 *
 * @param show true to show the cursor, false to hide it
 */
void mouse_update(bool show) {
    union REGS regs;

    if (show) {
        vga_hide_mouse(&mouse_data);
    }

    regs.x.ax = INT_MOUSE_UPDATE;
    int86(INT_MOUSE, &regs, &regs);

    mouse_data.x = regs.x.cx >> 1;  // map 0..640 MOUSE to 0..320 VGA
    mouse_data.y = regs.x.dx;
    mouse_data.left = (regs.x.bx & (1 << MOUSE_BUTTON_LEFT)) != 0;
    mouse_data.right = (regs.x.bx & (1 << MOUSE_BUTTON_RIGHT)) != 0;
    mouse_data.middle = (regs.x.bx & (1 << MOUSE_BUTTON_MIDDLE)) != 0;

    if (show) {
        vga_show_mouse(&mouse_data);
    }
}

/**
 * @brief get number of button pressed since last call.
 *
 * @param button one of MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT or MOUSE_BUTTON_MIDDLE.
 * @return number of times the button was pressed.
 */
int mouse_pressed(int button) {
    union REGS regs;

    regs.x.bx = button;

    regs.x.ax = INT_MOUSE_PRESSED;
    int86(INT_MOUSE, &regs, &regs);

    return regs.x.bx;
}

/**
 * @brief get number of button releases since last call.
 *
 * @param button one of MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT or MOUSE_BUTTON_MIDDLE.
 * @return number of times the button was released.
 */
int mouse_released(int button) {
    union REGS regs;

    regs.x.bx = button;

    regs.x.ax = INT_MOUSE_RELEASED;
    int86(INT_MOUSE, &regs, &regs);

    return regs.x.bx;
}
