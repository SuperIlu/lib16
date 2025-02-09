let api = require("api.js");

vga_init();

vga_set_color(1, 0xFF, 0xBB, 0xCC);
vga_set_color(2, 0xAA, 0xFF, 0xCC);
vga_set_color(3, 0xAA, 0xBB, 0xFF);
let get_color = vga_get_color(1);

vga_set_pixel(api.VGA_SCREEN_WIDTH / 2, api.VGA_SCREEN_HEIGHT / 2, 1);
let get_pixel = vga_get_pixel(api.VGA_SCREEN_WIDTH / 2, api.VGA_SCREEN_HEIGHT / 2);

vga_line(50, 50, 60, 70, 1);
vga_rect(10, 100, 60, 150, 2);
vga_filled_rect(100, 100, 150, 150, 3);

vga_circle(200, 100, 20, 4);
vga_filled_circle(250, 150, 30, 5);

sleep(3);

vga_exit();

print(get_color);
print(get_pixel);

print("hello");
print(api);

api.mul(0x10, 10);
