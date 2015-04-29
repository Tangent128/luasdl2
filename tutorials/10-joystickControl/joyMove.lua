---really simple tutorial in showing using the joystick [i used an xbox one] to move something round on screen.
---this could do wih more work on the axis length, but a good starting point to get things moving.

---leòdhas lepton - 2015 - smokingbunny.net

local SDL = require 'SDL'
local img = require 'SDL.image'

local graphics = {}
local pos = {}
local width = 32
local height = 32
local dir = 6

function moveInit()
	assert(SDL.init{SDL.flags.Video, SDL.flags.Joystick})
	assert(SDL.joystickOpen(0))

	local win = assert(SDL.createWindow{title = 'moveJoy', width = 320, height = 240})
	local rndr = assert(SDL.createRenderer(win, 0, 0))
	local image = assert(img.load('player.png'))

	local imgTex = assert(rndr:createTextureFromSurface(image))

	rndr:setDrawColor(0xFFFFFF)

	graphics.win = win
	graphics.rndr = rndr
	graphics.imgTex = imgTex

	---setting up our image width and height
	local f,a,w,h = imgTex:query()
	pos.x = width /2 - w / 2
	pos.y = height /2 - w / 2
	pos.w = w
	pos.h = h
end

moveInit()
local keys = SDL.getKeyboardState()

while true do
	graphics.rndr:clear()
	graphics.rndr:copy(graphics.imgTex, nil, pos)
	graphics.rndr:present()
	SDL.pumpEvents()
	if keys[SDL.scancode.Escape] then
		break
	end

	for e in SDL.pollEvent() do
		if e.type == SDL.event.Quit then
			break
		end
		---axis 0 is the left analog stick but for LEFT/RIGHT
		if e.axis == 0 then
			---this will make the image go LEFT on the screen
			if e.value < -10 then
				pos.x = pos.x - dir
				---this will make the player go RIGHT the screen
			elseif e.value > 10 then
				pos.x = pos.x + dir
			end
			---this just prints our axis 0 [left/right] to know what is happening. good for output test
			print(string.format("axis %d: %d", e.axis, e.value))
		end
		---axis 1 is the left analog stick but for UP/DOWN
		if e.axis == 1 then
			---this will make the image go UP the screen
			if e.value < -10 then
				pos.y = pos.y - dir
			---this will make the player go DOWN the screen
			elseif e.value > 10 then
				pos.y = pos.y + dir
			end
			---this just prints our axis 1 [up/down] to know what is happening. good for output test
			print(string.format("axis %d: %d", e.axis, e.value))
		end
	end
end