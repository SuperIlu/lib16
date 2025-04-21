KEY_ESC = 27
KEY_SPACE = 32
KEY_ENTER = 0x0d
KEY_BACKSPACE = 0x08
KEY_TAB = 0x09

KEY_DEL = 0xFF53
KEY_INS = 0xFF52
KEY_HOME = 0xFF47
KEY_END = 0xFF4f
KEY_PAGEUP = 0xFF49
KEY_PAGEDOWN = 0xFF51

KEY_F1 = 0xFF3b
KEY_F2 = 0xFF3c
KEY_F3 = 0xFF3d
KEY_F4 = 0xFF3e

KEY_F5 = 0xFF3f
KEY_F6 = 0xFF40
KEY_F7 = 0xFF41
KEY_F8 = 0xFF42

KEY_F9 = 0xFF43
KEY_F10 = 0xFF44
KEY_F11 = 0xFF85
KEY_F12 = 0xFF86

KEY_LEFT = 0xFF4b
KEY_RIGHT = 0xFF4d
KEY_UP = 0xFF48
KEY_DOWN = 0xFF50

function to_char(num)
	return string.char(num)
end

function print_table(t, n)
	if n then
		print(n)
	end
	for k, v in pairs(t) do
		print("  " .. k .. "=" .. v)
	end
end
