--
-- tutorial.lua -- 03 polling events
--

local SDL	= require "SDL"

local ret, err = SDL.init { SDL.flags.Video }
if not ret then
	error(err)
end

local win, err = SDL.createWindow {
	title	= "03 - Polling events",	-- optional
	width	= 320,				-- optional
	height	= 320,				-- optional
	flags	= { SDL.window.Resizable }	-- optional
}

if not win then
	error(err)
end

local running = true

while running do
	--
	-- Iterate over all events, this function does not block.
	--
	for e in SDL.pollEvent() do
		if e.type == SDL.event.Quit then
			running = false
		elseif e.type == SDL.event.KeyDown then
			print(string.format("key down: %d -> %s", e.keysym.sym, SDL.getKeyName(e.keysym.sym)))
		elseif e.type == SDL.event.MouseWheel then
			print(string.format("mouse wheel: %d, x=%d, y=%d", e.which, e.x, e.y))
		elseif e.type == SDL.event.MouseButtonDown then
			print(string.format("mouse button down: %d, x=%d, y=%d", e.button, e.x, e.y))
		elseif e.type == SDL.event.MouseMotion then
			print(string.format("mouse motion: x=%d, y=%d", e.x, e.y))
		end
	end
end
