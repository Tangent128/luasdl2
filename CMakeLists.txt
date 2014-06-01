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

# Project information
set(MAJOR "2")
set(MINOR "0")
set(PATCH "3")
set(BINDING "1")
set(VERSION "${MAJOR}.${MINOR}.${PATCH}-${BINDING}")

set(CMAKE_MODULE_PATH "${LuaSDL_SOURCE_DIR}/cmake")

# Path where to install docs
set(WITH_DOCSDIR "share/docs/Lua-SDL2"
    CACHE STRING "Path to install examples")
# Lua version: JIT, 51 and 52, default: 5.2
set(WITH_LUAVER "52"
    CACHE STRING "Version of Lua to use")

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

configure_file(
	${LuaSDL_SOURCE_DIR}/cmake/config.h.in
	${LuaSDL_BINARY_DIR}/config.h
)

include_directories(${LuaSDL_BINARY_DIR})

find_package(SDL2 REQUIRED)

if (WITH_LUAVER MATCHES "52")
	find_package(Lua52 REQUIRED)
	set(LUA_LIBRARIES ${LUA52_LIBRARIES})
	set(LUA_INCLUDES ${LUA52_INCLUDE_DIR})
	set(LUA_LIBDIR "lib/lua/5.2")
elseif (WITH_LUAVER MATCHES "51")
	find_package(Lua51 REQUIRED)
	set(LUA_LIBRARIES ${LUA_LIBRARIES})
	set(LUA_INCLUDES ${LUA_INCLUDE_DIR})
	set(LUA_LIBDIR "lib/lua/5.1")
elseif (WITH_LUAVER MATCHES "JIT")
	find_package(LuaJIT REQUIRED)
	set(LUA_LIBRARIES ${LUAJIT_LIBRARIES})
	set(LUA_INCLUDES ${LUAJIT_INCLUDE_DIR})
	set(LUA_LIBDIR "lib/lua/5.1")
else ()
	message(FATAL_ERROR "Unknown Lua version ${WITH_LUAVER}, must be 51, 52 or JIT")
endif ()

include_directories(
	${SDL2_INCLUDE_DIR}
	${LuaSDL_SOURCE_DIR}
	${LUA_INCLUDES}
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

install(
	TARGETS SDL
	DESTINATION ${LUA_LIBDIR}
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

# For Windows DLL
if (WIN32)
	add_subdirectory(windows)
endif ()

# CPack configuration
if (WIN32)
	set(CPACK_PACKAGE_NAME "Lua-SDL2")
	set(CPACK_NSIS_PACKAGE_NAME "Lua-SDL2")

	if (CMAKE_CL_64)
		set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
		set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION} (x64)")
	else ()
		set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
		set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
	endif ()

	# This determine the *target* architecture
	if (CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(WINARCH "amd64")
	else ()
		set(WINARCH "x86")
	endif ()

	set(CPACK_GENERATOR "NSIS;ZIP")
	set(CPACK_MONOLITHIC_INSTALL FALSE)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
	set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")

	set(CPACK_NSIS_DISPLAY_NAME "Lua-SDL2")
	set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
	set(CPACK_NSIS_CONTACT "markand@malikania.fr")

	set(CPACK_PACKAGE_VENDOR "Malikania")
	set(CPACK_PACKAGE_VERSION "${VERSION}")
	set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR})
	set(CPACK_PACKAGE_VERSION_MINOR ${MINOR})
	set(CPACK_PACKAGE_FILE_NAME "Lua-SDL2-${VERSION}-${CMAKE_SYSTEM_NAME}-${WINARCH}")
	set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lua-SDL2")
else ()
	set(CPACK_GENERATOR "TGZ")
endif ()

set(CPACK_SOURCE_PACKAGE_FILE_NAME "Lua-SDL2-${VERSION}-source")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES ".hg;_build_")

include(CPack)