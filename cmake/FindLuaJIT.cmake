#
# FindLuaJIT.cmake -- find LuaJIT
#
# Copyright (c) 2020 David Demelier <markand@malikania.fr>
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
# Find LuaJIT library, this modules defines:
#
#  LuaJIT_LIBRARY, the name of the library to link against.
#  LuaJIT_LIBRARIES, alias to Lua_LIBRARY.
#  LuaJIT_FOUND, true if found.
#  LuaJIT_INCLUDE_DIR, where to find lua.h.
#  LuaJIT_VERSION, the string version in the form x.y
#  LuaJIT_VERSION_MAJOR, major version (e.g. 5 in Lua 5.3)
#  LuaJIT_VERSION_MINOR, major version (e.g. 3 in Lua 5.3)
#
# The following imported targets will be defined:
#
# LuaJIT::LuaJIT
#

include(FindPackageHandleStandardArgs)

find_path(
	LuaJIT_INCLUDE_DIR
	NAMES lua.h
	PATH_SUFFIXES include/luajit-2.0
)

find_library(
	LuaJIT_LIBRARY
	NAMES luajit luajit-5.1
)

if (LuaJIT_INCLUDE_DIR AND EXISTS "${LuaJIT_INCLUDE_DIR}/lua.h")
	file(STRINGS "${LuaJIT_INCLUDE_DIR}/lua.h" LUA_H REGEX "^#define LUA_VERSION_NUM.*$")

	string(REGEX MATCH "([0-9])[0-9]([0-9])" LuaJIT_VERSION "${LUA_H}")
	string(REGEX REPLACE "([0-9])[0-9]([0-9])" "\\1.\\2" LuaJIT_VERSION ${LuaJIT_VERSION})

	string(REGEX REPLACE "([0-9])\\.[0-9]" "\\1" LuaJIT_VERSION_MAJOR ${LuaJIT_VERSION})
	string(REGEX REPLACE "[0-9]\\.([0-9])" "\\1" LuaJIT_VERSION_MINOR ${LuaJIT_VERSION})
endif ()

find_package_handle_standard_args(
	LuaJIT
	REQUIRED_VARS LuaJIT_LIBRARY LuaJIT_INCLUDE_DIR
	VERSION_VAR LuaJIT_VERSION
	HANDLE_COMPONENTS
)

if (LuaJIT_FOUND)
	set(LuaJIT_INCLUDE_DIRS ${LuaJIT_INCLUDE_DIR})
	set(LuaJIT_LIBRARIES ${LuaJIT_LIBRARY})

	if (NOT TARGET LuaJIT::LuaJIT)
		add_library(LuaJIT::LuaJIT UNKNOWN IMPORTED)
		set_target_properties(
			LuaJIT::LuaJIT
			PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${LuaJIT_LIBRARY}"
				INTERFACE_INCLUDE_DIRECTORIES "${LuaJIT_INCLUDE_DIRS}"
		)
	endif ()
endif ()

mark_as_advanced(LuaJIT_INCLUDE_DIR LuaJIT_LIBRARY)
