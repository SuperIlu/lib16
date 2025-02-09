#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <dos.h>

#include "elk.h"
#include "lib16.h"

mouse_t *js_mouse = NULL;

static jsval_t run_script(struct js *js, const char *fname) {
    char *data;
    size_t size;
    if (!util_read_file(fname, (void **)&data, &size)) {
        printf("require read() failed\n");
        return js_mkundef();
    }

    jsval_t ret = js_eval(js, data, size);
    free(data);

    return ret;
}

static jsval_t js_sleep(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 1) {
        return js_mkundef();
    }
    sleep((unsigned int)js_getnum(args[0]));
    return js_mkundef();
}
static jsval_t js_require(struct js *js, jsval_t *args, int nargs) {
    if (nargs != 1) {
        return js_mkundef();
    }
    return run_script(js, js_getstr(js, args[0], NULL));
}
static jsval_t js_print(struct js *js, jsval_t *args, int nargs) {
    for (int i = 0; i < nargs; i++) {
        const char *space = i == 0 ? "" : " ";
        printf("%s%s", space, js_str(js, args[i]));
    }
    putchar('\n');  // Finish by newline
    return js_mkundef();
}

static jsval_t js_vga_init(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    UNUSED(args);
    UNUSED(nargs);
    vga_init();
    return js_mkundef();
}
static jsval_t js_vga_exit(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    UNUSED(args);
    UNUSED(nargs);
    vga_exit();
    return js_mkundef();
}
static jsval_t js_vga_grayscale_palette(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    UNUSED(args);
    UNUSED(nargs);
    vga_grayscale_palette();
    return js_mkundef();
}
static jsval_t js_vga_get_color(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 1) {
        return js_mkundef();
    }
    palette_color_t pc;
    vga_get_color(js_getnum(args[0]), &pc);

    uint32_t col = ((uint32_t)pc.red << 16 | (uint32_t)pc.green << 8 | (uint32_t)pc.blue) & 0x00FFFFFF;
    return js_mknum(col);
}
static jsval_t js_vga_set_color(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 4) {
        return js_mkundef();
    }
    uint16_t idx = js_getnum(args[0]);
    palette_color_t pc;
    pc.red = js_getnum(args[1]);
    pc.green = js_getnum(args[2]);
    pc.blue = js_getnum(args[3]);
    vga_set_color(idx, &pc);
    return js_mkundef();
}
static jsval_t js_vga_set_pixel(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 3) {
        return js_mkundef();
    }

    uint16_t x = js_getnum(args[0]);
    uint16_t y = js_getnum(args[1]);
    color_t c = js_getnum(args[2]);

    vga_set_pixel(x, y, c);

    return js_mkundef();
}
static jsval_t js_vga_get_pixel(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 2) {
        return js_mkundef();
    }
    uint16_t x = js_getnum(args[0]);
    uint16_t y = js_getnum(args[1]);

    color_t c = vga_get_pixel(x, y);

    return js_mknum(c);
}
static jsval_t js_vga_line(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 5) {
        return js_mkundef();
    }

    uint16_t x1 = js_getnum(args[0]);
    uint16_t y1 = js_getnum(args[1]);
    uint16_t x2 = js_getnum(args[2]);
    uint16_t y2 = js_getnum(args[3]);
    color_t c = js_getnum(args[4]);

    vga_line(x1, y1, x2, y2, c);

    return js_mkundef();
}

static jsval_t js_vga_rect(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 5) {
        return js_mkundef();
    }

    uint16_t l = js_getnum(args[0]);
    uint16_t t = js_getnum(args[1]);
    uint16_t r = js_getnum(args[2]);
    uint16_t b = js_getnum(args[3]);
    color_t c = js_getnum(args[4]);

    vga_rect(l, t, r, b, c);

    return js_mkundef();
}

static jsval_t js_vga_filled_rect(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 5) {
        return js_mkundef();
    }

    uint16_t l = js_getnum(args[0]);
    uint16_t t = js_getnum(args[1]);
    uint16_t r = js_getnum(args[2]);
    uint16_t b = js_getnum(args[3]);
    color_t c = js_getnum(args[4]);

    vga_filled_rect(l, t, r, b, c);

    return js_mkundef();
}

static jsval_t js_vga_circle(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 4) {
        return js_mkundef();
    }

    uint16_t x = js_getnum(args[0]);
    uint16_t y = js_getnum(args[1]);
    uint16_t r = js_getnum(args[2]);
    color_t c = js_getnum(args[3]);

    vga_circle(x, y, r, c);

    return js_mkundef();
}

static jsval_t js_vga_filled_circle(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    if (nargs != 4) {
        return js_mkundef();
    }

    uint16_t x = js_getnum(args[0]);
    uint16_t y = js_getnum(args[1]);
    uint16_t r = js_getnum(args[2]);
    color_t c = js_getnum(args[3]);

    vga_filled_circle(x, y, r, c);

    return js_mkundef();
}

static jsval_t js_vga_hide_mouse(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    UNUSED(args);
    UNUSED(nargs);
    if (js_mouse) {
        vga_hide_mouse(js_mouse);
    }
    return js_mkundef();
}
static jsval_t js_vga_show_mouse(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    UNUSED(args);
    UNUSED(nargs);
    if (js_mouse) {
        vga_show_mouse(js_mouse);
    }
    return js_mkundef();
}
static jsval_t js_mouse_init(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    UNUSED(args);
    UNUSED(nargs);

    js_mouse = mouse_init(&MOUSE_CROSS);
    return js_mkundef();
}

static jsval_t js_mouse_update(struct js *js, jsval_t *args, int nargs) {
    UNUSED(js);
    bool update = true;
    if (nargs >= 1) {
        update = ((int)js_getnum(args[0])) != 0;
    }
    mouse_update(update);
    return js_mkundef();
}

// extern void vga_polygon(vertex_t *vertices, uint16_t num_vertices, color_t c);
// extern void vga_hide_mouse(mouse_t *mouse);
// extern void vga_show_mouse(mouse_t *mouse);

int main(int argc, char *argv[]) {
    int c;
    char *end;
    bool debug = false;
    unsigned int mem = UTIL_KILOBYTE(16);

    while ((c = getopt(argc, argv, ":dm:")) != -1) {
        switch (c) {
            case 'd':
                debug = true;
                break;
            case 'm':
                mem = strtol(optarg, &end, 0);
                if (end == optarg) {
                    printf("Illegal number: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                if (mem < 1024) {
                    printf("Memory to small: %s < 1024\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case ':':
                printf("-%c without argument\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case '?':
                printf("usage: %s [flags] <scripname>\n", argv[0]);
                printf("  -d         - print debug info.\n");
                printf("  -m <bytes> - amount of memory to use for execution (default 16KiB).\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    _nheapgrow();
    if (debug) {
        printf("DEBUG >>> interpreter mem size: %lu\n", mem);
        printf("DEBUG >>> stack size: %u, free mem: %u, max size: %u\n", stackavail(), _memavl(), _memmax());
    }

    void *mem_ptr = malloc(mem);
    if (!mem_ptr) {
        printf("ERROR >>> Could not allocate %u bytes.\n", mem);
    }

    if (optind >= argc) {
        printf("Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    struct js *js = js_create(mem_ptr, mem);
    jsval_t res = js_mkundef();

    // add C-functions
    js_set(js, js_glob(js), "require", js_mkfun(js_require));
    js_set(js, js_glob(js), "print", js_mkfun(js_print));
    js_set(js, js_glob(js), "sleep", js_mkfun(js_sleep));

    js_set(js, js_glob(js), "vga_init", js_mkfun(js_vga_init));
    js_set(js, js_glob(js), "vga_exit", js_mkfun(js_vga_exit));
    js_set(js, js_glob(js), "vga_grayscale_palette", js_mkfun(js_vga_grayscale_palette));
    js_set(js, js_glob(js), "vga_get_color", js_mkfun(js_vga_get_color));
    js_set(js, js_glob(js), "vga_set_color", js_mkfun(js_vga_set_color));
    js_set(js, js_glob(js), "vga_set_pixel", js_mkfun(js_vga_set_pixel));
    js_set(js, js_glob(js), "vga_get_pixel", js_mkfun(js_vga_get_pixel));
    js_set(js, js_glob(js), "vga_line", js_mkfun(js_vga_line));
    js_set(js, js_glob(js), "vga_rect", js_mkfun(js_vga_rect));
    js_set(js, js_glob(js), "vga_filled_rect", js_mkfun(js_vga_filled_rect));
    js_set(js, js_glob(js), "vga_circle", js_mkfun(js_vga_circle));
    js_set(js, js_glob(js), "vga_filled_circle", js_mkfun(js_vga_filled_circle));
    js_set(js, js_glob(js), "vga_hide_mouse", js_mkfun(js_vga_hide_mouse));
    js_set(js, js_glob(js), "vga_show_mouse", js_mkfun(js_vga_show_mouse));
    js_set(js, js_glob(js), "mouse_init", js_mkfun(js_mouse_init));
    js_set(js, js_glob(js), "mouse_update", js_mkfun(js_mouse_update));

    res = run_script(js, argv[optind]);

    // Print the result of the last one
    printf("EXIT  >>> '%s'\n", js_str(js, res));

    if (debug) {
        js_dump(js);
    }

    exit(EXIT_SUCCESS);
}
