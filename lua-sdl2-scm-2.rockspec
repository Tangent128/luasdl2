package = "Lua-SDL2"
version = "scm-2"
source = {
   url = "https://github.com/Tangent128/luasdl2/archive/v2.0.3-3.tar.gz",
   md5 = "",
   dir = "luasdl2-2.0.3-3"
}
source = {
   -- when making a release rockspec, update fields in the above source
   -- block and delete this source block.
   url = "git://github.com/Tangent128/luasdl2"
}
description = {
   summary = "Lua-SDL2 is a pure C binding of SDL 2.0 for Lua 5.1, JIT, 5.2, and 5.3",
   detailed = "Lua-SDL2 is a pure C binding of SDL 2.0 for Lua 5.1, JIT, 5.2, and 5.3",
   homepage = "https://github.com/Tangent128/luasdl2/",
   license = "ISC",
   maintainer = "Joseph Wallace <Tangent128@gmail.com>"
}
dependencies = {}
build = {
   type = "cmake",
   variables = {
      
      -- directly setting the header path to the LuaRocks-provided one
      -- will build the library for the correct Lua version
      WITH_LUAVER = "user",
      LUA_INCLUDE_DIR = "$(LUA_INCDIR)",
      
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


