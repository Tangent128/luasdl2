--
-- font.lua -- show SDL_ttf module with UTF-8 string
--

local SDL	= require "SDL"
local font	= require "SDL.ttf"

local ret, err = SDL.init { SDL.flags.Video }
if not ret then
	error(err)
end

local ret, err = font.init()
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

local f, err = font.open("DejaVuSans.ttf", 10)
if not f then
	error(err)
end

local s, err = f:renderUtf8("Gérard!", "blended", { r = 255, g = 255, b = 255 })
if not s then
	error(err)
end

local t = rdr:createTextureFromSurface(s)
if not t then
	error(err)
end

rdr:clear()
rdr:copy(t)
rdr:present()

SDL.delay(2000)