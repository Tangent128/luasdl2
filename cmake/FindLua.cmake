#
# FindLua.cmake -- find Lua versions
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
# Find bare Lua library, this modules defines:
#
#  Lua_LIBRARY, the name of the library to link against.
#  Lua_LIBRARIES, alias to Lua_LIBRARY.
#  Lua_FOUND, true if found.
#  Lua_INCLUDE_DIR, where to find lua.h.
#  Lua_VERSION, the string version in the form x.y
#  Lua_VERSION_MAJOR, major version (e.g. 5 in Lua 5.3)
#  Lua_VERSION_MINOR, major version (e.g. 3 in Lua 5.3)
#
# The following imported targets will be defined:
#
# Lua::Lua
#

include(FindPackageHandleStandardArgs)

find_path(
	Lua_INCLUDE_DIR
	NAMES lua.h
	PATH_SUFFIXES include/lua
)

find_library(
	Lua_LIBRARY
	NAMES lua
)

if (Lua_INCLUDE_DIR AND EXISTS "${Lua_INCLUDE_DIR}/lua.h")
	file(STRINGS "${Lua_INCLUDE_DIR}/lua.h" LUA_H REGEX "^#define LUA_VERSION_NUM.*$")

	string(REGEX MATCH "([0-9])[0-9]([0-9])" Lua_VERSION "${LUA_H}")
	string(REGEX REPLACE "([0-9])[0-9]([0-9])" "\\1.\\2" Lua_VERSION ${Lua_VERSION})

	string(REGEX REPLACE "([0-9])\\.[0-9]" "\\1" Lua_VERSION_MAJOR ${Lua_VERSION})
	string(REGEX REPLACE "[0-9]\\.([0-9])" "\\1" Lua_VERSION_MINOR ${Lua_VERSION})
endif ()

find_package_handle_standard_args(
	Lua
	REQUIRED_VARS Lua_LIBRARY Lua_INCLUDE_DIR
	VERSION_VAR Lua_VERSION
	HANDLE_COMPONENTS
)

if (Lua_FOUND)
	set(Lua_INCLUDE_DIRS ${Lua_INCLUDE_DIR})
	set(Lua_LIBRARIES ${Lua_LIBRARY})

	if (NOT TARGET Lua::Lua)
		add_library(Lua::Lua UNKNOWN IMPORTED)
		set_target_properties(
			Lua::Lua
			PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${Lua_LIBRARY}"
				INTERFACE_INCLUDE_DIRECTORIES "${Lua_INCLUDE_DIRS}"
		)
	endif ()
endif ()

mark_as_advanced(Lua_INCLUDE_DIR Lua_LIBRARY)
