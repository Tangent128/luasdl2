--
-- tutorial.lua -- 02 opening a window
--

local SDL	= require "SDL"

local ret, err = SDL.init { SDL.flags.Video }
if not ret then
	error(err)
end

local win, err = SDL.createWindow {
	title	= "02 - Opening a window",	-- optional
	width	= 320,				-- optional
	height	= 320,				-- optional
	flags	= { SDL.window.Resizable }	-- optional
}

if not win then
	error(err)
end

-- Let the window opened a bit
SDL.delay(5000)
