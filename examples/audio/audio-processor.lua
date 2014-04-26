--
-- audio-processor.lua -- the callback function
--

local SDL	= require "SDL"

local args	= { ... }

local sound	= { }
local channel	= SDL.getChannel "Audio"

--
-- This will be called.
--
sound.data	= channel:first()
sound.buf	= sound.data.data
sound.pos	= 0
sound.len	= sound.data.length

if not sound.data then
	error("No data to play")
end

return function (length)
	if length == 0 then
		return nil
	end

	if length > sound.len then
		length = sound.len
	end

	local s = sound.buf:sub(sound.pos + 1, sound.pos + 1 + length)

	sound.pos = sound.pos + length
	sound.len = sound.len - length

	if #s == 0 then
		return nil
	end

	return s
end
