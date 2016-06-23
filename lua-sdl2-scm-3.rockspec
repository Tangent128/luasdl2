package = "Lua-SDL2"
version = "scm-3"
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
external_dependencies = {
   SDL2 = {
      header = "SDL2/SDL_scancode.h"
   },
   SDL2_image = {
      header = "SDL2/SDL_image.h"
   },
   SDL2_mixer = {
      header = "SDL2/SDL_mixer.h"
   },
   SDL2_net = {
      header = "SDL2/SDL_net.h"
   },
   SDL2_ttf = {
      header = "SDL2/SDL_ttf.h"
   },
   
   platforms = {
      windows = {
         SDL2 = {
            header = "SDL_scancode.h"
         },
         SDL2_image = {
            header = "SDL_image.h"
         },
         SDL2_mixer = {
            header = "SDL_mixer.h"
         },
         SDL2_net = {
            header = "SDL_net.h"
         },
         SDL2_ttf = {
            header = "SDL_ttf.h"
         },
      }
   }
}

local function PlusCommon(...)
   return {
      "common/array.c",
      "common/common.c",
      "common/rwops.c",
      "common/surface.c",
      "common/table.c",
      "common/variant.c",
      "common/video.c",
      ...
   }
end

build = {
   type = "builtin",
   modules = {
      SDL = {
         libraries = {"SDL2"},
         defines = {},
         incdirs = {"$(SDL2_INCDIR)/SDL2", "src/", "extern/queue/", "./", "rocks/"},
         libdirs = {"$(SDL2_LIBDIR)"},
         sources = PlusCommon(
            "src/audio.c",
            "src/channel.c",
            "src/clipboard.c",
            "src/cpu.c",
            "src/display.c",
            "src/events.c",
            "src/filesystem.c",
            "src/gamecontroller.c",
            "src/gl.c",
            "src/haptic.c",
            "src/joystick.c",
            "src/keyboard.c",
            "src/logging.c",
            "src/mouse.c",
            "src/platform.c",
            "src/power.c",
            "src/rectangle.c",
            "src/renderer.c",
            "src/SDL.c",
            "src/texture.c",
            "src/thread.c",
            "src/timer.c",
            "src/window.c"
         ),
      },
      ["SDL.image"] = {
         libraries = {"SDL2", "SDL2_image"},
         defines = {},
         incdirs = {"$(SDL2_INCDIR)/SDL2", "src/", "extern/queue/", "./", "rocks/"},
         libdirs = {"$(SDL2_LIBDIR)"},
         sources = PlusCommon(
            "sdl-image/src/image.c"
         ),
      },
      ["SDL.mixer"] = {
         libraries = {"SDL2", "SDL2_mixer"},
         defines = {},
         incdirs = {"$(SDL2_INCDIR)/SDL2", "src/", "extern/queue/", "./", "rocks/"},
         libdirs = {"$(SDL2_LIBDIR)"},
         sources = PlusCommon(
            "sdl-mixer/src/mixer.c"
         ),
      },
      ["SDL.net"] = {
         libraries = {"SDL2", "SDL2_net"},
         defines = {},
         incdirs = {"$(SDL2_INCDIR)/SDL2", "src/", "extern/queue/", "./", "rocks/"},
         libdirs = {"$(SDL2_LIBDIR)"},
         sources = PlusCommon(
            "sdl-net/src/net.c"
         ),
      },
      ["SDL.ttf"] = {
         libraries = {"SDL2", "SDL2_ttf"},
         defines = {},
         incdirs = {"$(SDL2_INCDIR)/SDL2", "src/", "extern/queue/", "./", "rocks/"},
         libdirs = {"$(SDL2_LIBDIR)"},
         sources = PlusCommon(
            "sdl-ttf/src/ttf.c"
         ),
      },
   },
   platforms = {
      windows = {
         modules = {
            SDL = {
               incdirs = {"$(SDL2_INCDIR)", "src/", "extern/queue/", "./", "rocks/"}
            },
            ["SDL.image"] = {
               incdirs = {"$(SDL2_INCDIR)", "src/", "extern/queue/", "./", "rocks/"}
            },
            ["SDL.mixer"] = {
               incdirs = {"$(SDL2_INCDIR)", "src/", "extern/queue/", "./", "rocks/"}
            },
            ["SDL.net"] = {
               incdirs = {"$(SDL2_INCDIR)", "src/", "extern/queue/", "./", "rocks/"}
            },
            ["SDL.ttf"] = {
               incdirs = {"$(SDL2_INCDIR)", "src/", "extern/queue/", "./", "rocks/"}
            },
         }
      }
   }
}


