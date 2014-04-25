--
-- image.lua -- shows SDL_image module for Lua
--

local SDL	= require "SDL"
local image	= require "SDL.image"

local ret, err = SDL.init { SDL.flags.Video }
if not ret then
	error(err)
end

local formats, ret, err = image.init { image.flags.PNG }
if not ret then
	error(err)
end

local win, err = SDL.createWindow {
	title	= "Image",
	height	= 256,
	width	= 256
}

if not win then
	error(err)
end

local rdr, err = SDL.createRenderer(win, 0, 0)
if not rdr then
	error(err)
end

local img, ret = image.load("Lua-SDL2.png")
if not img then
	error(err)
end

img = rdr:createTextureFromSurface(img)

for i = 1, 50 do
	rdr:setDrawColor(0xFFFFFF)
	rdr:clear()
	rdr:copy(img)
	rdr:present()

	SDL.delay(100)
end
