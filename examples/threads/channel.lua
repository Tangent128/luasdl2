--
-- channel.lua -- shows how to pass / wait data between channels
--

local SDL	= require "SDL"

SDL.init { SDL.flags.Video }

local t, err = SDL.createThread("test",
	function ()
		local SDL	= require "SDL"

		local channel	= SDL.getChannel "Test"

		print("Waiting...")
		local v = channel:wait()
		print("Received: " .. tostring(v))

		return 0
	end
)

if not t then
	error(err)
end

SDL.delay(1000)


print("Main...")
SDL.delay(3000)

local channel = SDL.getChannel "Test"

print("Pushing...")
channel:supply(true)
print("He received")
