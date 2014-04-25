#
# CMakeLists.txt -- build system for LuaSDL2
#
# Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

cmake_minimum_required(VERSION 2.8)
project(LuaSDL C)

set(CMAKE_MODULE_PATH "${LuaSDL_SOURCE_DIR}/cmake")

# Path where to install docs
set(WITH_DOCSDIR "share/docs/Lua-SDL2"
    CACHE STRING "Path to install examples")

set(
	SOURCES
	src/audio.c
	src/audio.h
	src/channel.c
	src/channel.h
	src/clipboard.c
	src/clipboard.h
	src/cpu.c
	src/cpu.h
	src/display.c
	src/display.h
	src/events.c
	src/events.h
	src/gamecontroller.c
	src/gamecontroller.h
	src/gl.c
	src/gl.h
	src/haptic.c
	src/haptic.h
	src/joystick.c
	src/joystick.h
	src/keyboard.c
	src/keyboard.h
	src/logging.c
	src/logging.h
	src/mouse.c
	src/mouse.h
	src/platform.c
	src/platform.h
	src/power.c
	src/power.h
	src/rectangle.c
	src/rectangle.h
	src/renderer.c
	src/renderer.h
	src/SDL.c
	src/texture.c
	src/texture.h
	src/thread.c
	src/thread.h
	src/timer.c
	src/timer.h
	src/window.c
	src/window.h
)

find_package(SDL2 REQUIRED)
find_package(Lua52 REQUIRED)

include_directories(
	${SDL2_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
	${LuaSDL_SOURCE_DIR}
)

if (WIN32)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
else ()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif ()

# We always use _FOREACH_SAFE.
find_package(SysQueue)
if (NOT SYSQUEUE_FOUND
    OR NOT SYSQUEUE_SLIST_FOREACH_SAFE
    OR NOT SYSQUEUE_STAILQ_FOREACH_SAFE
    OR NOT SYSQUEUE_LIST_FOREACH_SAFE
    OR NOT SYSQUEUE_TAILQ_FOREACH_SAFE)
	include_directories(BEFORE ${LuaSDL_SOURCE_DIR}/extern/queue)
endif ()

# Common code for modules
add_subdirectory(common)

add_library(
	SDL
	MODULE
	${SOURCES}
)

set_target_properties(
	SDL
	PROPERTIES
	PREFIX ""
)

target_link_libraries(
	SDL
	common
	${SDL2_LIBRARY}
)

set(WITH_LIBDIR "lib/lua/5.2" CACHE STRING "Lua library directory")

install(
	TARGETS SDL
	DESTINATION ${WITH_LIBDIR}
)

# Other libSDL sub modules
option(WITH_IMAGE "Enable SDL_image" On)
option(WITH_MIXER "Enable SDL_mixer" On)
option(WITH_TTF "Enable SDL_ttf" On)
option(WITH_NET "Enable SDL_net" On)

if (WITH_IMAGE)
	add_subdirectory(sdl-image)
endif ()

if (WITH_MIXER)
	add_subdirectory(sdl-mixer)
endif ()

if (WITH_TTF)
	add_subdirectory(sdl-ttf)
endif ()

if (WITH_NET)
	add_subdirectory(sdl-net)
endif ()

# For the examples and tutorials
add_subdirectory(examples)
add_subdirectory(tutorials)
