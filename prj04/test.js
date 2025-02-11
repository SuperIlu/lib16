let api = require("api.js");

vga_init();
mouse_init();

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

for (let x = 0; x < 1000; x++) {
	let k = getkey();
	let hurz = 'b' === k;
	if (hurz) {
		break;
	}
	mouse_update(true);
	vga_rect(mouse_x(), mouse_y(), mouse_x() + 5, mouse_y() + 5, 4);
}

vga_exit();

print(get_color);
print(get_pixel);

print("hello");
print(api);
print(getkey());

api.mul(0x10, 10);
