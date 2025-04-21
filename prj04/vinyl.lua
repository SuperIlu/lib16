-- This work is licensed under Attribution-ShareAlike 3.0 Unported.
-- https://creativecommons.org/licenses/by-sa/3.0/
--
-- The original source by Arjo Nagelhout can be found here:
-- https://www.openprocessing.org/sketch/681621
--
-- It was modified to run with Lua by superilu@yahoo.com.

require "prj04/func"

T = 0
FG_COL_IDX = 63
BG_COL_IDX = 0

function Draw()
	vga_filled_rect(0, 0, width, height, BG_COL_IDX)
	vga_wait_for_retrace()
	for i = 0, 10 do
		V(width / 2, height / 2, i - T, 5)
	end
end

function V(x, y, a, l)
	x = x + l * math.sin(a) * 15
	y = y - l * math.cos(a) * 15

	-- vga_filled_rect(ix, iy, ix + 4, iy + 4, FG_COL_IDX)
	vga_set_pixel(x, y, FG_COL_IDX)

	if l > 1 then
		l = l * .7
		V(x, y, a + 1 + math.cos(T), l)
		V(x, y, a - 1 - math.sin(T), l)
	end
end

vga_init()
vga_grayscale_palette()
while true do
	local k = getkey();
	if k == KEY_ESC then
		break
	end
	Draw()
	T = T + 0.01
end
vga_exit()
