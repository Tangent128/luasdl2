--
-- client.lua -- send "Hello" to a server using SDL_net
--

local net	= require "SDL.net"

-- Init net
net.init()

-- Create and connect
local addr = net.resolveHost("localhost", 5959)
local s = net.openTcp(addr)

s:send("Hello")
net.quit()
