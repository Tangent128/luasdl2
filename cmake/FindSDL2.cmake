#
# FindSDL2.cmake -- find SDL2 library and addons
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
# Find SDL2 library and components, this modules defines:
#
#  SDL2_LIBRARY, the name of the library to link against.
#  SDL2_MAIN_LIBRARY, for SDL2main (if present).
#  SDL2_LIBRARIES, alias to SDL2_LIBRARY.
#  SDL2_FOUND, true if found.
#  SDL2_INCLUDE_DIR, where to find SDL.h.
#
# The following imported targets will be defined:
#
# SDL2::SDL2
# SDL2::SDL2main (if present)
#
# This module also handle the following official SDL addons:
#
# - image
# - mixer
# - net
# - ttf
#
# And thus, variables SDL2_<C>_LIBRARY, SDL2_<C>_INCLUDE_DIRS and SDL2::<C>
# imported targets will be defined if they are found.
#

include(FindPackageHandleStandardArgs)

# The official include convention is <SDL.h> not <SDL/SDL.h>.
find_path(
	SDL2_INCLUDE_DIR
	NAMES SDL.h
	PATH_SUFFIXES include/SDL2 include
)

find_library(SDL2_LIBRARY NAMES SDL2 libSDL2)
find_library(SDL2_MAIN_LIBRARY NAMES SDL2main libSDL2main)

# Standard components.
foreach (c ${SDL2_FIND_COMPONENTS})
	find_path(
		SDL2_${c}_INCLUDE_DIR
		NAMES SDL.h
		PATH_SUFFIXES include/SDL2 include
	)

	find_library(
		SDL2_${c}_LIBRARY
		NAMES SDL2_${c} libSDL2_${c}
	)

	if (NOT TARGET SDL2::${c} AND SDL2_${c}_LIBRARY)
		set(SDL2_${c}_FOUND TRUE)
		add_library(SDL2::${c} UNKNOWN IMPORTED)
		set_target_properties(
			SDL2::${c}
			PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${SDL2_${c}_LIBRARY}"
				INTERFACE_INCLUDE_DIRECTORIES "${SDL2_${c}_INCLUDE_DIRS}"
		)
	endif ()

	mark_as_advanced(SDL2_${c}_INCLUDE_DIR SDL2_${c}_LIBRARY)
endforeach ()

find_package_handle_standard_args(
	SDL2
	REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
	HANDLE_COMPONENTS
)

if (SDL2_FOUND)
	set(SDL2_LIBRARIES ${SDL2_LIBRARY})
	set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

	if (NOT TARGET SDL2::SDL2)
		add_library(SDL2::SDL2 UNKNOWN IMPORTED)
		set_target_properties(
			SDL2::SDL2
			PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${SDL2_LIBRARY}"
				INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
		)
	endif ()

	if (NOT TARGET SDL2::main AND SDL2_MAIN_LIBRARY)
		add_library(SDL2::main UNKNOWN IMPORTED)
		set_target_properties(
			SDL2::main
			PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}"
				INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
		)
	endif ()
endif ()

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY)
