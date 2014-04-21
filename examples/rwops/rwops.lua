--
-- rwops.lua -- write a file with RWOps
--

local SDL	= require "SDL"

SDL.init { SDL.flags.Video }

local writer = {
	file = io.open("test.bin", "wb")
}

if not writer.file then
	error("Unable to open file")
end

function writer.size()
	return -1
end

function writer.close()
	writer.file:close()
end

function writer.seek(offset, whence)
	return 0
end

function writer.read(n, size)
end

function writer.write(data, n, size)
	writer.file:write(data)
end

local rw, err = SDL.RWCreate(writer)
if not rw then
	error(err)
end

rw:writeByte(1, 16, "LE")
