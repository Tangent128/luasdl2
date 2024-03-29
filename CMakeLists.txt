#
# CMakeLists.txt -- build system for LuaSDL2
#
# Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
# Copyright (c) 2014, 2015 Joseph Wallace <tangent128@gmail.com>
# Copyright (c) 2017 Webster Sheets <webster@web-eworks.com>
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

cmake_minimum_required(VERSION 3.10)
project(Lua-SDL2 C)

set(CMAKE_MODULE_PATH "${Lua-SDL2_SOURCE_DIR}/cmake")

include(CheckStructHasMember)
include(GNUInstallDirs)

# Project information.  Follows the [SEMVER](semver.org) standard
set(BINDING_MAJOR "2")
set(BINDING_MINOR "1")

# Latest implemented SDL2 version.  Only increment when a new SDL2 version is
# completely implemented.
set(MAJOR "2")
set(MINOR "0")
set(PATCH "5")
set(VERSION "${BINDING_MAJOR}.${BINDING_MINOR} for SDL ${MAJOR}.${MINOR}.${PATCH}")

set(WITH_DOCSDIR "share/docs/Lua-SDL2"
    CACHE STRING "Path to install examples")
set(WITH_LUAVER "5.3"
	CACHE STRING "Version of Lua to use: 5.4 (default), 5.3, 5.2, 5.1 or JIT)")

find_package(SDL2 REQUIRED)
find_package(SDL2 COMPONENTS image mixer net ttf)

if (WITH_LUAVER MATCHES "JIT")
	find_package(LuaJIT REQUIRED)
	# LuaJIT is 5.1 for the moment.
	set(Lua_VERSION "5.1")
else ()
	find_package(Lua ${WITH_LUAVER} EXACT REQUIRED)
endif ()

# Check for SDL_DropEvent.windowID (See GH-#61)
set(CMAKE_REQUIRED_INCLUDES ${SDL2_INCLUDE_DIR})
set(CMAKE_REQUIRED_DEFINITIONS -DSDL_MAIN_HANDLED)
check_struct_has_member(SDL_DropEvent windowID SDL.h HAVE_DROPEVENT_WINDOW_ID)

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
	src/filesystem.c
	src/filesystem.h
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
	src/vulkan.c
	src/vulkan.h
	src/window.c
	src/window.h
)

configure_file(
	${Lua-SDL2_SOURCE_DIR}/cmake/config.h.in
	${Lua-SDL2_BINARY_DIR}/config.h
)

include_directories(${Lua-SDL2_BINARY_DIR})

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
	include_directories(BEFORE ${Lua-SDL2_SOURCE_DIR}/extern/queue)
endif ()

# Common code for modules
add_subdirectory(common)

add_library(
	SDL
	MODULE
	${SOURCES}
)

if (APPLE)
	#
	# TODO: change once there is a better CMake variant for this issue.
	#
	# Lua modules must not link to Lua, unfortunately add_library(MODULE) uses
	# -bundle on macOS while it should use -dynamiclib instead.
	#
	set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-dynamiclib -undefined dynamic_lookup")
endif ()

set_target_properties(SDL PROPERTIES PREFIX "")
target_link_libraries(SDL common)

install(
	TARGETS SDL
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/lua/${Lua_VERSION}
)

# Other libSDL sub modules
option(WITH_IMAGE "Enable SDL_image" On)
option(WITH_MIXER "Enable SDL_mixer" On)
option(WITH_TTF "Enable SDL_ttf" On)
option(WITH_NET "Enable SDL_net" On)

if (WITH_IMAGE AND TARGET SDL2::image)
	add_subdirectory(sdl-image)
endif ()

if (WITH_MIXER AND TARGET SDL2::mixer)
	add_subdirectory(sdl-mixer)
endif ()

if (WITH_TTF AND TARGET SDL2::ttf)
	add_subdirectory(sdl-ttf)
endif ()

if (WITH_NET AND TARGET SDL2::net)
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

	# This determine the *target* architecture
	if (CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(WINARCH "amd64")
	else ()
		set(WINARCH "x86")
	endif ()

	if (MINGW)
		set(GENERATOR "MinGW")
	else ()
		set(GENERATOR "VC")
	endif ()

	set(CPACK_GENERATOR "ZIP")
	set(CPACK_MONOLITHIC_INSTALL FALSE)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
	set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")

	set(CPACK_PACKAGE_VENDOR "Malikania")
	set(CPACK_PACKAGE_VERSION "${VERSION}")
	set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR})
	set(CPACK_PACKAGE_VERSION_MINOR ${MINOR})
	set(CPACK_PACKAGE_FILE_NAME "Lua-SDL2-${VERSION}-${GENERATOR}-${WINARCH}")
	set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lua-SDL2")
else ()
	set(CPACK_GENERATOR "TGZ")
endif ()

set(CPACK_SOURCE_PACKAGE_FILE_NAME "Lua-SDL2-${VERSION}-source")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES ".hg;_build_;.git")

include(CPack)
