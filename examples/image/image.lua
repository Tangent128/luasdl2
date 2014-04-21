local SDL	= require "SDL"
local image	= require "SDL.image"

local ret, err = SDL.init { SDL.flags.Video }
if not ret then
	error(err)
end

local ret, err = image.init { image.flags.JPEG }
if not ret then
	error(err)
end

local win, err = SDL.createWindow {
	title	= "Image",
	height	= 100,
	width	= 100
}

if not win then
	error(err)
end

local rdr, err = SDL.createRenderer(win, 0, 0)
if not rdr then
	error(err)
end

local img, ret = image.load("urban-terror.png")
if not img then
	error(err)
end

img = rdr:createTextureFromSurface(img)

rdr:clear()
rdr:copy(img)
rdr:present()

SDL.delay(2000)
