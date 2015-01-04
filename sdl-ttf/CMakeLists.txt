#
# CMakeLists.txt -- build system for LuaSDL2 (SDL_ttf module)
#
# Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
# Copyright (c) 2014 Joseph Wallace <tangent128@gmail.com>
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

find_package(SDL2_ttf REQUIRED)

set(
	TTF_SOURCES
	src/ttf.c
)

include_directories(
	"${LuaSDL_SOURCE_DIR}/src"
	${SDL2_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIR}
)

add_library(
	ttf
	MODULE
	${TTF_SOURCES}
)

set_target_properties(
	ttf
	PROPERTIES
	PREFIX ""
)

install(
	TARGETS ttf
	DESTINATION ${LUA_LIBDIR}/SDL
)

target_link_libraries(
	ttf
	common
	${SDL2_LIBRARY}
	${SDL2_TTF_LIBRARY}
)
