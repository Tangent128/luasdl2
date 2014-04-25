--
-- tutorial.lua -- 01 initialization
--

-- Load SDL module
local SDL	= require "SDL"

-- Initialize video and audio
local ret, err = SDL.init {
	SDL.flags.Video,
	SDL.flags.Audio
}

if not ret then
	error(err)
end

-- Show the version
print(string.format("SDL %d.%d.%d",
    SDL.VERSION_MAJOR,
    SDL.VERSION_MINOR,
    SDL.VERSION_PATCH
))

-- Close everything
SDL.quit()
