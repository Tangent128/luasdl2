---window module to be required in main.lua
---leòdhas lepton 2015

---set your width and height of your screen
local screenWidth = 800
local screenHeight = 600

---set up a graphics table for our render
local graphics = {}

function windowInit()
	assert(SDL.init{SDL.flags.Video})
	local win = assert(SDL.createWindow{title = 'module', width = screenWidth, height = screenHeight})
	local rndr = assert(SDL.createRenderer(win,0,0))

	graphics.rndr = rndr
	graphics.rndr:clear()
	graphics.rndr:present()
end