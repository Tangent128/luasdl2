--
-- tutorial.lua -- 06 writing text
--

local SDL	= require "SDL"
local ttf	= require "SDL.ttf"

local function trySDL(func, ...)
	local t = { func(...) }

	if not t[1] then
		error(t[#t])
	end

	return table.unpack(t)
end

trySDL(SDL.init, { SDL.flags.Video })
trySDL(ttf.init)

-- Create the window
local win = trySDL(SDL.createWindow, {
	title	= "06 - Drawing text",		-- optional
	width	= 90,				-- optional
	height	= 50,				-- optional
})

-- Create the renderer
local rdr = trySDL(SDL.createRenderer, win, -1)

-- Open the font
local font = trySDL(ttf.open, "DejaVuSans.ttf", 10)

-- Create some text
local s = trySDL(font.renderUtf8, font, "Lua-SDL2", "solid", 0xFFFFFF)

-- Convert to texture and show
local text = trySDL(rdr.createTextureFromSurface, rdr, s)

for i = 1, 50 do
	rdr:clear()
	rdr:copy(text)
	rdr:present()

	SDL.delay(100)
end

SDL.quit()
ttf.quit()
