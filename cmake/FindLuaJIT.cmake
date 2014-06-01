# Locate Lua library
# This module defines
#  LUAJIT_FOUND, if false, do not try to link to Lua 
#  LUAJIT_LIBRARIES
#  LUAJIT_INCLUDE_DIR, where to find lua.h
#  LUAJIT_VERSION_STRING, the version of Lua found (since CMake 2.8.8)
#
# Note that the expected include convention is
#  #include "lua.h"
# and not
#  #include <lua/lua.h>
# This is because, the lua location is not standardized and may exist
# in locations other than lua/

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(LUAJIT_INCLUDE_DIR luajit.h
	HINTS
	$ENV{LUA_DIR}
	PATH_SUFFIXES include/luajit-2.0 include
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

find_library(LUAJIT_LIBRARY
	NAMES luajit luajit-5.1
	HINTS
	$ENV{LUA_DIR}
	PATH_SUFFIXES lib64 lib
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/sw
	/opt/local
	/opt/csw
	/opt
)

if (LUAJIT_LIBRARY)
	# include the math library for Unix
	if (UNIX AND NOT APPLE)
		find_library(LUAJIT_MATH_LIBRARY m)
		set(LUAJIT_LIBRARIES "${LUAJIT_LIBRARY};${LUAJIT_MATH_LIBRARY}" CACHE STRING "Lua Libraries")
	# For Windows and Mac, don't need to explicitly include the math library
	else ()
	  set(LUAJIT_LIBRARIES "${LUAJIT_LIBRARY}" CACHE STRING "Lua Libraries")
	endif ()
endif ()

if(LUAJIT_INCLUDE_DIR AND EXISTS "${LUAJIT_INCLUDE_DIR}/lua.h")
	file(STRINGS "${LUAJIT_INCLUDE_DIR}/lua.h" luajit_version_str REGEX "^#define[ \t]+LUA_RELEASE[ \t]+\"Lua .+\"")

	string(REGEX REPLACE "^#define[ \t]+LUA_RELEASE[ \t]+\"Lua ([^\"]+)\".*" "\\1" LUAJIT_VERSION_STRING "${luajit_version_str}")
	unset(luajit_version_str)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LUAJIT_FOUND to TRUE if 
# all listed variables are TRUE
find_package_handle_standard_args(LuaJIT
                                  REQUIRED_VARS LUAJIT_LIBRARIES LUAJIT_INCLUDE_DIR
                                  VERSION_VAR LUAJIT_VERSION_STRING)

mark_as_advanced(LUAJIT_INCLUDE_DIR LUAJIT_LIBRARIES LUAJIT_LIBRARY LUAJIT_MATH_LIBRARY)
