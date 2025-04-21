#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos.h>
#include <conio.h>

#include "lib16.h"

/* ======================================================================
** defines
** ====================================================================== */
#define NFUNC(f, n)              \
    {                            \
        lua_pushcfunction(L, f); \
        lua_setglobal(L, n);     \
    }

#define NIVAR(v, n)            \
    {                          \
        lua_pushinteger(L, v); \
        lua_setglobal(L, n);   \
    }

/* ======================================================================
** local variables
** ====================================================================== */
static mouse_t *l_mouse = NULL;

/* ======================================================================
** private functions
** ====================================================================== */
static void error(lua_State *L, const char *fmt, ...) {
    vga_exit();

    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    lua_close(L);

    exit(EXIT_FAILURE);
}

static void setfield(lua_State *L, const char *index, int value) {
    lua_pushstring(L, index);
    lua_pushinteger(L, value);
    lua_settable(L, -3);
}

static int pos_int(lua_State *L, int arg, unsigned int max, const char *name) {
    int i = luaL_checkinteger(L, arg);
    if (i < 0) {
        luaL_error(L, "%s can't be negative", name);
        return 0;
    }
    if (i > max) {
        luaL_error(L, "%s can't be >%d", name, max);
        return 0;
    }
    return i;
}

static int pos_int_noop(lua_State *L, int arg, unsigned int max) {
    int i = luaL_checknumber(L, arg);
    if (i < 0 || i > max) {
        return -1;
    }
    return i;
}

static int l_sleep(lua_State *L) {
    int i = luaL_checkinteger(L, 1);
    if (i < 0) {
        luaL_error(L, "can't sleep for negative values");
        return 0;
    }

    sleep(i);
    return 0; /* number of results */
}

static int l_getkey(lua_State *L) {
    if (kbhit()) {
        unsigned int code = getch();
        if (code == 0) {
            code = 0xFF00 | getch();
        }
        lua_pushinteger(L, code);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int l_vga_init(lua_State *L) {
    UNUSED(L);
    vga_init();
    return 0;
}

static int l_vga_exit(lua_State *L) {
    UNUSED(L);
    vga_exit();
    return 0;
}

static int l_vga_grayscale_palette(lua_State *L) {
    UNUSED(L);
    vga_grayscale_palette();
    return 0;
}

static int l_vga_get_color(lua_State *L) {
    int idx = pos_int(L, 1, VGA_MAX_COLORS, "color index");

    palette_color_t pc;
    vga_get_color(idx, &pc);

    lua_newtable(L);
    setfield(L, "r", pc.red);
    setfield(L, "g", pc.green);
    setfield(L, "b", pc.blue);
    return 1;
}

static int l_vga_set_color(lua_State *L) {
    UNUSED(L);
    int idx = pos_int(L, 1, VGA_MAX_COLORS, "color index");

    palette_color_t pc;
    pc.red = luaL_checkinteger(L, 2);
    pc.green = luaL_checkinteger(L, 3);
    pc.blue = luaL_checkinteger(L, 4);
    vga_set_color(idx, &pc);

    return 0;
}

static int l_vga_set_pixel(lua_State *L) {
    UNUSED(L);
    int x = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int y = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);
    if (x == -1 || y == -1) {
        return 0;
    }
    int idx = pos_int(L, 3, VGA_MAX_COLORS, "color index");

    vga_set_pixel(x, y, idx);

    return 0;
}

static int l_vga_get_pixel(lua_State *L) {
    UNUSED(L);
    int x = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int y = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);

    if (x == -1 || y == -1) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, vga_get_pixel(x, y));
    }
    return 1;
}

static int l_vga_line(lua_State *L) {
    UNUSED(L);
    int x1 = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int y1 = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);
    int x2 = pos_int_noop(L, 3, VGA_SCREEN_WIDTH);
    int y2 = pos_int_noop(L, 4, VGA_SCREEN_HEIGHT);
    if (x1 == -1 || y1 == -1 || x2 == -1 || y2 == -1) {
        return 0;
    }
    int idx = pos_int(L, 5, VGA_MAX_COLORS, "color index");

    vga_line(x1, y1, x2, y2, idx);

    return 0;
}

static int l_vga_rect(lua_State *L) {
    UNUSED(L);
    int l = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int t = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);
    int r = pos_int_noop(L, 3, VGA_SCREEN_WIDTH);
    int b = pos_int_noop(L, 4, VGA_SCREEN_HEIGHT);
    if (l == -1 || t == -1 || r == -1 || b == -1) {
        return 0;
    }
    int idx = pos_int(L, 5, VGA_MAX_COLORS, "color index");

    vga_rect(l, t, r, b, idx);

    return 0;
}

static int l_vga_filled_rect(lua_State *L) {
    UNUSED(L);
    int l = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int t = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);
    int r = pos_int_noop(L, 3, VGA_SCREEN_WIDTH);
    int b = pos_int_noop(L, 4, VGA_SCREEN_HEIGHT);
    if (l == -1 || t == -1 || r == -1 || b == -1) {
        return 0;
    }
    int idx = pos_int(L, 5, VGA_MAX_COLORS, "color index");

    vga_filled_rect(l, t, r, b, idx);

    return 0;
}

static int l_vga_circle(lua_State *L) {
    UNUSED(L);
    int x = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int y = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);
    if (x == -1 || y == -1) {
        return 0;
    }
    int r = pos_int(L, 3, VGA_SCREEN_WIDTH, "radius");
    int idx = pos_int(L, 4, VGA_MAX_COLORS, "color index");

    vga_circle(x, y, r, idx);

    return 0;
}

static int l_vga_filled_circle(lua_State *L) {
    UNUSED(L);
    int x = pos_int_noop(L, 1, VGA_SCREEN_WIDTH);
    int y = pos_int_noop(L, 2, VGA_SCREEN_HEIGHT);
    if (x == -1 || y == -1) {
        return 0;
    }
    int r = pos_int(L, 3, VGA_SCREEN_WIDTH, "radius");
    int idx = pos_int(L, 4, VGA_MAX_COLORS, "color index");

    vga_filled_circle(x, y, r, idx);

    return 0;
}

static int l_vga_hide_mouse(lua_State *L) {
    UNUSED(L);
    if (l_mouse) {
        vga_hide_mouse(l_mouse);
    }
    return 0;
}

static int l_vga_show_mouse(lua_State *L) {
    UNUSED(L);
    if (l_mouse) {
        vga_show_mouse(l_mouse);
    }
    return 0;
}

static int l_mouse_init(lua_State *L) {
    UNUSED(L);
    l_mouse = mouse_init(&MOUSE_CROSS);
    return 0;
}

static int l_vga_wait_for_retrace(lua_State *L) {
    UNUSED(L);
    vga_wait_for_retrace();
    return 0;
}

static int l_mouse_update(lua_State *L) {
    if (l_mouse) {
        mouse_update(lua_toboolean(L, 1));
        lua_newtable(L);
        setfield(L, "x", l_mouse->x);
        setfield(L, "y", l_mouse->y);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* ======================================================================
** public functions
** ====================================================================== */
int main(int argc, char *argv[]) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s <lua script>\n", argv[0]);
    }
    const char *filename = argv[1];

    NIVAR(VGA_MAX_COLORS, "num_colors");
    NIVAR(VGA_SCREEN_WIDTH, "width");
    NIVAR(VGA_SCREEN_HEIGHT, "height");

    NFUNC(l_sleep, "sleep");
    NFUNC(l_getkey, "getkey");
    NFUNC(l_vga_init, "vga_init");
    NFUNC(l_vga_exit, "vga_exit");
    NFUNC(l_vga_grayscale_palette, "vga_grayscale_palette");
    NFUNC(l_vga_set_color, "vga_set_color");
    NFUNC(l_vga_get_color, "vga_get_color");
    NFUNC(l_vga_set_pixel, "vga_set_pixel");
    NFUNC(l_vga_get_pixel, "vga_get_pixel");
    NFUNC(l_vga_line, "vga_line");
    NFUNC(l_vga_rect, "vga_rect");
    NFUNC(l_vga_filled_rect, "vga_filled_rect");
    NFUNC(l_vga_circle, "vga_circle");
    NFUNC(l_vga_filled_circle, "vga_filled_circle");
    NFUNC(l_vga_hide_mouse, "vga_hide_mouse");
    NFUNC(l_vga_show_mouse, "vga_show_mouse");
    NFUNC(l_vga_wait_for_retrace, "vga_wait_for_retrace");
    NFUNC(l_mouse_init, "mouse_init");
    NFUNC(l_mouse_update, "mouse_update");

    // NFUNC(l_, "");

    if (luaL_dofile(L, filename) != LUA_OK) {
        error(L, "cannot run file: %s", lua_tostring(L, -1));
    }

    vga_exit();
    lua_close(L);
    exit(EXIT_SUCCESS);
}
