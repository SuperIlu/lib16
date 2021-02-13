/**
 * @file mouse.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief simple mouse access
 *
 * @copyright SuperIlu
 */
#ifndef __MOUSE_H_
#define __MOUSE_H_

#include <stdbool.h>
#include <stdint.h>

/* ======================================================================
** defines
** ====================================================================== */
#define MOUSE_BUTTON_LEFT 0    //!< left button
#define MOUSE_BUTTON_RIGHT 1   //!< right button
#define MOUSE_BUTTON_MIDDLE 2  //!< middle button

#define MOUSE_X_RESOLUTION 640  //!< mouse resolution horizontally
#define MOUSE_Y_RESOLUTION 200  //!< mouse resolution vertically

#define MOUSE_CURSOR_WIDTH 12   //!< width of a mouse pointer
#define MOUSE_CURSOR_HEIGHT 12  //!< height of a mouse pointer

/* ======================================================================
** typedefs
** ====================================================================== */
//! mouse pointer graphics definition
typedef struct __mouse_pointer {
    uint16_t x;                                             //!< hotspot x
    uint16_t y;                                             //!< hotspot y
    uint8_t img[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT];  //!< VGA buffer image
} mouse_pointer_t;

//! mouse info struct
typedef struct __mouse {
    uint16_t buttons;  //!< number of buttons
    uint16_t x;        //!< mouse X 0..VGA_SCREEN_WIDTH
    uint16_t y;        //!< mouse y 0..VGA_SCREEN_HEIGHT
    bool left;         //!< left button is pressed right now true/false
    bool right;        //!< right button is pressed right now true/false
    bool middle;       //!< middle button is pressed right now true/false

    mouse_pointer_t cursor[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT];  //!< cursor image
    mouse_pointer_t under[MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT];   //!< original image under cursor
} mouse_t;

/* ======================================================================
** global variables
** ====================================================================== */
//! build in mouse pointer image CROSS
extern const mouse_pointer_t MOUSE_CROSS;

//! build in mouse pointer image POINTER
extern const mouse_pointer_t MOUSE_POINTER;

/* ======================================================================
** prototypes
** ====================================================================== */
extern mouse_t* mouse_init(const mouse_pointer_t* image);
extern void mouse_update(bool show);
extern int mouse_pressed(int button);
extern int mouse_released(int button);

#endif  // __MOUSE_H_
