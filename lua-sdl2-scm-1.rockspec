package = "Lua-SDL2"
version = "scm-1"
source = {
   url = "git://github.com/Tangent128/luasdl2"
}
description = {
   summary = "Lua-SDL2 is a pure C binding of SDL 2.0 for Lua 5.1, Lua 5.2 and LuaJIT.",
   detailed = "Lua-SDL2 is a pure C binding of SDL 2.0 for Lua 5.1, Lua 5.2 and LuaJIT.",
   homepage = "https://github.com/Tangent128/luasdl2/",
   license = "ISC",
   maintainer = "Joseph Wallace <Tangent128@gmail.com>"
}
dependencies = {}
build = {
   type = "cmake",
   variables = {
      CMAKE_INSTALL_PREFIX = "$(PREFIX)",
      LUA_LIBDIR = "$(LIBDIR)",
      
      WITH_DOCS = "Off",
      
      -- Uncomment to disable any not desired/installed:
      --WITH_MIXER = "Off",
      --WITH_TTF = "Off",
      --WITH_NET = "Off",
      --WITH_IMAGE = "Off",
   },
}


