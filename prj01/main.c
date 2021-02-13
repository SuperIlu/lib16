#include <stddef.h>
#include <stdlib.h>
#include <dos.h>
#include <stdio.h>
#include <strings.h>

#include "lib16.h"

void draw(char *fname) {
    bitmap_t *bm;
    bm = bitmap_load(fname, true);
    if (bm) {
        bitmap_draw(bm, 0, 0, true);
        bitmap_free(bm);
        bm = NULL;
    } else {
        printf("Could not load %s: %s\n", fname, err_str);
    }
    sleep(2);
}

void render_text(char *fname, uint16_t y) {
    bitmap_t *bm;
    bm = bitmap_load(fname, true);
    if (bm) {
        bitmap_render_char(bm, 100, y, '$', 63);
        bitmap_render_char(bm, 120, y, 'X', 63);

        bitmap_render_string(bm, 10, y + 20, "This is a test\nof the emergency\nbroadcast system!", 127);

        bitmap_save(bm, "WRTST.BMP");

        bitmap_free(bm);
        bm = NULL;
    } else {
        printf("Could not load %s: %s\n", fname, err_str);
    }
}

int main(int argc, char *argv[]) {
    bool send = false;
    ipx_data_t data;
    ipx_net_t net;
    ipx_node_t node;
    ipx_received_t packet;
    uint16_t socket_num = 0x1234;

    mouse_t *m;

    bitmap_t *bm;
    char *fname;

    int i, x, y;
    vertex_t v[3] = {{100, 10}, {120, 30}, {90, 30}};

    if (argc >= 2 && (strcasecmp("send", argv[1]) == 0)) {
        send = true;
    }

    if (ipx_init()) {
        puts("IPX is available");

        ipx_get_local_address(net, node);
        ipx_print_address(net, node);

        if (ipx_open_socket(&socket_num)) {
            printf("Opened IPX socket %d\n", socket_num);
            if (send) {
                bzero(data, sizeof(ipx_data_t));
                for (i = 0; i < 5; i++) {
                    snprintf(&data, IPX_MAX_PACKET_LEN, "WATCOM %d", i);
                    ipx_send_packet(&data, IPX_BROADCAST_ADDR);
                    printf("IPX packet %d sent\n", i);
                }
            } else {
                for (i = 0; i < 5; i++) {
                    if (ipx_check_packet()) {
                        while (ipx_get_packet(&packet)) {
                            printf("IPX packet received %s from\n", &packet.data);
                            ipx_print_address(NULL, packet.source);
                        }
                    } else {
                        puts("No packet in buffer");
                    }
                    sleep(1);
                }
            }

            ipx_close_socket();
        } else {
            printf("Could not open IPX socket: %s", err_str);
        }
    } else {
        printf("IPX NOT available: %s\n", err_str);
    }

    m = mouse_init(&MOUSE_CROSS);
    if (m) {
        puts("Mouse is available");

        for (i = 0; i < 5; i++) {
            mouse_update(false);
            printf("x=%d, y=%d, l=%d, m=%d, r=%d\n", m->x, m->y, m->left, m->middle, m->right);
            sleep(1);
        }
    } else {
        printf("Mouse NOT available: %s\n", err_str);
    }

    if (vga_init()) {
        for (x = 10; x < 40; x += 2) {
            for (y = 10; y < 40; y += 2) {
                vga_set_pixel(x, y, x + y);
            }
        }

        vga_line(50, 50, 60, 70, 1);
        vga_rect(10, 100, 60, 150, 2);
        vga_filled_rect(100, 100, 150, 150, 3);

        vga_circle(200, 100, 20, 4);
        vga_filled_circle(250, 150, 30, 5);

        vga_polygon(&v, 3, 4);

        sleep(10);

        vga_grayscale_palette();
        render_text("COMPUT8.BMP", 10);
        render_text("MAGIC5_8.BMP", 100);

        sleep(10);

        draw("TST01.BMP");
        draw("CAT.BMP");
        draw("3DFX.BMP");

        fname = "OUT.BMP";
        bm = bitmap_copy(0, 0, VGA_SCREEN_WIDTH, VGA_SCREEN_HEIGHT, true);
        if (!bitmap_save(bm, fname)) {
            printf("Could not save %s: %s\n", fname, err_str);
        }
        bitmap_free(bm);
        bm = NULL;

        vga_exit();
    } else {
        printf("VGA is not supported:%s", err_str);
    }

    exit(0);
}
