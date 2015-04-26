-- simple keyboard control example. using the UP, DOWN, LEFT, RIGHT arrow keys to move things around.
-- example uses lua-SDL2 https://github.com/Tangent128/luasdl2
-- lewis lepton 2015 - smokingbunny.net

local SDL	= require 'SDL'
local image = require 'SDL.image'

local graphics = {}
local pos = {}
--moves character 5 pixels
local dir = 5
--width and height of character
local width = 32
local height = 32

local function initialize()
	--init SDL
	local ret, err = SDL.init {SDL.flags.Video}
	if not ret then
		error(err)
	end

	--make sure our image is init
	local ret, err = image.init{image.flags.PNG}
	if not ret then
		error(err)
	end

	--create window
	local win, err = SDL.createWindow {
		title	= "Keyboard",
		width	= 320,
		height	= 240
	}
	if not win then
		error(err)
	end

	--create renderer
	local rdr, err = SDL.createRenderer(win, -1)
	if not rdr then
		error(err)
	end

	--draws background white
	rdr:setDrawColor(0xFFFFFF)

	--allow loading of image
	local img, ret = image.load("../sprite/player.png")
	if not img then
		error(err)
	end

	--embedding above loaded image into a texture
	local logo, err = rdr:createTextureFromSurface(img)
	if not logo then
		error(err)
	end

	graphics.win = win
	graphics.rdr = rdr
	graphics.logo = logo

	-- get size of image
	local f, a, w, h = logo:query()
	pos.x = width / 2 - w / 2
	pos.y = height / 2 - h / 2
	pos.w = 32
	pos.h = 32
end

initialize()

local keys = SDL.getKeyboardState()

while true do
	--this reads SDL keyboari input -- needed
	SDL.pumpEvents()
	--rendering in the graphics/player
	graphics.rdr:clear()
	graphics.rdr:copy(graphics.logo, nil, pos)
	graphics.rdr:present()

	--keyboard contorls. also spit out what you are pressing
	if keys[SDL.scancode.Left] then
		pos.x = pos.x - dir
		print("left pressed")
	end
	if keys[SDL.scancode.Right] then
		pos.x = pos.x + dir
		print("right pressed")
	end
	if keys[SDL.scancode.Up] then
		pos.y = pos.y - dir
		print("up pressed")
	end
	if keys[SDL.scancode.Down] then
		pos.y = pos.y + dir
		print("down pressed")
	end
	if keys[SDL.scancode.Escape] then
		print("Exiting!")
		break
	end
	--delays message output and activity every 60 millisecs
	SDL.delay(60)
end