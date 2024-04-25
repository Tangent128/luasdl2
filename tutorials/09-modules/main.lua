SDL = require 'SDL'
---ere we are importing our window module/class into our main.lua
local window = require 'windowModule'

---this is now using our windowInit() function from window.lua
windowInit()

---we set up our keyboard input call
local keys = SDL.getKeyboardState()

---we set up a while loop to hold an escape key command to close the window
while true do
	SDL.pumpEvents()
	if keys[SDL.scancode.Escape] then
		break
	end
end