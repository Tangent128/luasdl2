--
-- tutorial.lua -- 05 playing sound
--

local SDL	= require "SDL"
local mixer	= require "SDL.mixer"

local function trySDL(func, ...)
	local t = { func(...) }

	if not t[1] then
		error(t[#t])
	end

	return table.unpack(t)
end

trySDL(SDL.init, { SDL.flags.Video })

local win = trySDL(SDL.createWindow, {
	title	= "05 - Playing sound",		-- optional
	width	= 320,				-- optional
	height	= 320,				-- optional
})

-- Open the audio
trySDL(mixer.openAudio, 44100, SDL.audioFormat.S16, 2, 1024)

-- Open the sound
local sound = trySDL(mixer.loadWAV, "gun.wav")

-- Play the sound indefinitely
sound:playChannel(1)

-- Let the window opened a bit
SDL.delay(5000)

SDL.quit()
mixer.quit()
