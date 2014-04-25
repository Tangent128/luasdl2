--
-- tutorial.lua -- 03 polling events
--

local SDL	= require "SDL"
local image	= require "SDL.image"

--
-- Small wrapper to check if the SDL function succeed, if not
-- we exit the program.
--
-- SDL functions that fails usually return the values as nil and the last
-- argument is the error message.
--
local function trySDL(func, ...)
	local t = { func(...) }

	if not t[1] then
		error(t[#t])
	end

	return table.unpack(t)
end

-- Initialize SDL and SDL_image
trySDL(SDL.init, { SDL.flags.Video })

-- SDL_image returns the table with the loaded formats
local formats, ret, err = image.init { image.flags.PNG }
if not formats[image.flags.PNG] then
	error(err)
end

-- Initialize the window
local win = trySDL(SDL.createWindow, {
	title	= "04 - Drawing image",
	width	= 256,
	height	= 256,
	flags	= { SDL.flags.OpenGL }
})

-- Create the renderer
local rdr = trySDL(SDL.createRenderer, win, -1)

rdr:setDrawColor(0xFFFFFF)

-- Load the image as a surface
local img = trySDL(image.load, "Lua-SDL2.png")

-- Convert the image surface to texture
local logo = trySDL(rdr.createTextureFromSurface, rdr, img)

for i = 1, 50 do
	-- Draw it
	rdr:clear()
	rdr:copy(logo)
	rdr:present()

	SDL.delay(100)
end

SDL.quit()

image.quit()
