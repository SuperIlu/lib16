-- This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
-- http://creativecommons.org/licenses/by-nc-sa/4.0/
--
-- The original source can be found here:
-- https://p5js.org/examples/math-additive-wave.html
--
-- It was modified to run with Lua by superilu@yahoo.com.
--
--
-- Additive Wave
-- by Daniel Shiffman.
--
-- Create a more complex wave by adding two waves together.

require "prj04/func"

FG_COL_IDX = 32
BG_COL_IDX = 0

X_SPACING = 8 -- How far apart should each horizontal location be spaced
MAX_WAVES = 4 -- total # of waves to add together

Theta = 0.0;
Amplitude = {} -- Height of wave
Dx = {}        -- Value for incrementing X, to be calculated as a function of period and X_SPACING
YValues = 0;   -- Using an array to store height values for the wave (not entirely necessary)

function Setup()
	local w = width + 16; -- Width of entire wave

	for i = 0, MAX_WAVES do
		Amplitude[i] = (math.random(10, 30))
		local period = math.random(100, 300) -- How many pixels before the wave repeats
		Dx[i] = ((math.pi * 2 / period) * X_SPACING);
	end

	YValues = {}
	NumYValues = math.floor(w / X_SPACING)
	for i = 1, NumYValues do
		YValues[i] = 0
	end
end

function Draw()
	CalcWave();
	vga_wait_for_retrace()
	vga_filled_rect(0, 0, width, height, BG_COL_IDX)
	vga_wait_for_retrace()
	RenderWave();
end

function CalcWave()
	-- Increment Theta (try different values for 'angular velocity' here
	Theta = Theta + 0.02

	-- Set all height values to zero
	for i = 0, NumYValues do
		YValues[i] = 0;
	end

	-- Accumulate wave height values
	for j = 0, MAX_WAVES do
		local x = Theta;
		for i = 0, NumYValues do
			-- Every other wave is cosine instead of sine
			if j % 2 == 0 then
				YValues[i] = YValues[i] + math.sin(x) * Amplitude[j];
			else
				YValues[i] = YValues[i] + math.cos(x) * Amplitude[j];
			end
			x = x + Dx[j];
		end
	end
end

function RenderWave()
	-- A simple way to draw the wave with an ellipse at each location
	for x = 0, NumYValues do
		vga_circle(x * X_SPACING, height / 2 + YValues[x], X_SPACING, FG_COL_IDX)
		-- vga_set_pixel(x * X_SPACING, height / 2 + YValues[x], FG_COL_IDX)
	end
end

vga_init()
vga_grayscale_palette()
Setup()
while true do
	local k = getkey();
	if k == KEY_ESC then
		break
	end
	Draw()
end
vga_exit()
