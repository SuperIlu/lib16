#include <stddef.h>
#include <stdlib.h>
#include <dos.h>
#include <stdio.h>
#include <strings.h>
#include <conio.h>

#include "lib16.h"

typedef struct __draw_packet {
    uint16_t x;  //!< mouse X 0..639
    uint16_t y;  //!< mouse y 0.199

    uint8_t padding[IPX_MAX_PACKET_LEN - 4];
} draw_packet_t;

#define NUM_COLORS 8

palette_color_t colors[NUM_COLORS] = {
    {0, 0, 0},        // black
    {255, 255, 255},  // white
    {255, 0, 0},      // red
    {0, 255, 0},      // green
    {0, 0, 255},      // blue
    {255, 255, 0},    //
    {255, 0, 255},    //
    {0, 255, 255},    //
};

int main(int argc, char *argv[]) {
    int i;
    mouse_t *m;

    draw_packet_t draw;

    uint16_t socket_num = 0x1234;
    ipx_net_t net;
    ipx_node_t node;
    ipx_received_t packet;

    UNUSED(argc);
    UNUSED(argv);

    if (!ipx_init()) {
        puts("IPX NOT available");
        exit(1);
    }
    ipx_get_local_address(net, node);
    ipx_print_address(net, node);

    if (!ipx_open_socket(&socket_num)) {
        puts("Could not open IPX socket");
        exit(1);
    }
    m = mouse_init(&MOUSE_CROSS);
    if (!m) {
        puts("Mouse NOT available");
        exit(1);
    }

    vga_init();

    for (i = 0; i < 255; i++) {
        vga_set_color(i, &colors[i % NUM_COLORS]);
    }

    while (true) {
        // exit on keypress
        if (kbhit()) {
            getch();
            break;
        }
        mouse_update(true);
        if (m->left) {
            draw.x = m->x;
            draw.y = m->y;

            ipx_send_packet((ipx_data_t *)&draw, IPX_BROADCAST_ADDR);
            vga_hide_mouse(m);
            vga_set_pixel(draw.x, draw.y, 1);
            vga_show_mouse(m);
        }

        // received packets from others
        if (ipx_check_packet()) {
            vga_hide_mouse(m);
            while (ipx_get_packet(&packet)) {
                uint8_t col = packet.source[5] + 2;
                draw_packet_t *rec = (draw_packet_t *)&packet.data;
                vga_set_pixel(rec->x, rec->y, col);
            }
            vga_show_mouse(m);
        }
    }

    // cleanup
    vga_exit();
    ipx_close_socket();

    exit(0);
}
