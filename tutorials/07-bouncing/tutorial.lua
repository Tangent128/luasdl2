--
-- tutorial.lua -- 07 logo bouncing
--

local SDL	= require "SDL"
local image	= require "SDL.image"

local running	= true
local graphics	= { }
local pos	= { }
local dir	= { 1, 1 }
local width	= 640
local height	= 480

local function initialize()
	local ret, err = SDL.init { SDL.flags.Video }
	if not ret then
		error(err)
	end

	local ret, err = image.init { image.flags.PNG }
	if not ret then
		error(err)
	end

	local win, err = SDL.createWindow {
		title	= "07 - Bouncing logo",
		width	= width,
		height	= height
	}

	if not win then
		error(err)
	end

	local rdr, err = SDL.createRenderer(win, -1)
	if not rdr then
		error(err)
	end

	-- Set to white for the logo
	rdr:setDrawColor(0xFFFFFF)

	local img, ret = image.load("Lua-SDL2.png")
	if not img then
		error(err)
	end

	local logo, err = rdr:createTextureFromSurface(img)
	if not logo then
		error(err)
	end

	-- Store in global graphics
	graphics.win	= win
	graphics.rdr	= rdr
	graphics.logo	= logo

	-- Get the size of the logo
	local f, a, w, h = logo:query()

	pos.x = width / 2 - w / 2
	pos.y = height / 2 - h / 2
	pos.w = 256
	pos.h = 256
end

initialize()

while running do
	for e in SDL.pollEvent() do
		if e.type == SDL.event.Quit then
			running = false
		end
	end

	graphics.rdr:clear()
	graphics.rdr:copy(graphics.logo, nil, pos)
	graphics.rdr:present()

	pos.x = pos.x + dir[1]
	pos.y = pos.y + dir[2]

	if dir[1] > 0 and pos.x > width - 256 then
		dir[1] = -1
	elseif dir[1] < 0 and pos.x <= 0 then
		dir[1] = 1
	end

	if dir[2] > 0 and pos.y > height - 256 then
		dir[2] = -1
	elseif dir[2] < 0 and pos.y <= 0 then
		dir[2] = 1
	end

	SDL.delay(20)
end
