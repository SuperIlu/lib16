require "prj04/func"

vga_init()
mouse_init()

vga_set_color(1, 0xFF, 0xBB, 0xCC)
vga_set_color(2, 0xAA, 0xFF, 0xCC)
vga_set_color(3, 0xAA, 0xBB, 0xFF)
get_color = vga_get_color(1)

vga_set_pixel(width / 2, height / 2, 1);
get_pixel = vga_get_pixel(width / 2, height / 2);

vga_line(50, 50, 60, 70, 1);
vga_rect(10, 100, 60, 150, 2);
vga_filled_rect(100, 100, 150, 150, 3);

vga_circle(200, 100, 20, 4);
vga_filled_circle(250, 150, 30, 5);

for x = 0, 1000 do
	k = getkey()
	if k == 27 then
		break
	end
	pos = mouse_update(true);
	vga_rect(pos.x, pos.y, pos.x + 5, pos.y + 5, 4);
end

vga_exit()

print_table(get_color, "get_color")
print("get_pixel=" .. get_pixel)

print("press ESC to exit")
while true do
	k = getkey();
	if k == KEY_ESC then
		break
	elseif k ~= nil then
		if k < 0xff00 then
			print(to_char(k))
		else
			print(string.format("0x%04x", k))
		end
	end
end
