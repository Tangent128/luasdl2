Lua-SDL2 INSTALL
================

Lua-SDL2 installation guide.

Requirements
============

* [Lua](http://lua.org), mandatory (Lua 5.1, LuaJIT, Lua 5.2, or Lua 5.3)
* [SDL 2.0.1+](http://libsdl.org), mandatory
* Either [CMake](http://cmake.org) or [LuaRocks](https://luarocks.org), for building only

And optional libraries for the official SDL modules:

* [SDL_mixer](http://www.libsdl.org/projects/SDL_mixer)
* [SDL_ttf](http://www.libsdl.org/projects/SDL_ttf)
* [SDL_net](http://www.libsdl.org/projects/SDL_net)
* [SDL_image](http://www.libsdl.org/projects/SDL_image)

Installation (LuaRocks)
=======================

    $ luarocks install lua-sdl2

Will install Lua-SDL2 from the Luarocks repositories.

If you want to install a development version, clone the repository and run:

    $ luarocks install lua-sdl2-scm-3.rockspec

Currently, there is no way to disable the mixer/ttf/net/image modules.
Eventually they will be split into separate rockspecs.

Manually locating SDL2 headers
------------------------------

By default, Luarocks expects to find the SDL2 headers under `/usr/include/SDL2/`.
You can customize the prefix by setting the SDL2_INCDIR variable in your Luarocks
configuration or on the command line for installing.

On most systems, you need to specify the path *containing* the `SDL2/` folder,
for example `/usr/local/include`.

On Windows, you need to specify the folder containing the headers directly,
such as `c:\include\SDL2\`.

Installation (CMake)
====================

Take care to substitute version with the current Lua-SDL2 version.

    $ tar xvzf Lua-SDL2-version.tar.gz
    $ cd Lua-SDL2-version
    $ mkdir _build_
    $ cd _build_
    $ cmake ..
    $ make
    # make install

Customizing the build (CMake)
=============================

Several options are available. The following commands are expected to be ran in
the _build_ directory created above.

Disable SDL_mixer
-----------------

$ cmake .. -DWITH_MIXER=Off

Disable SDL_ttf
---------------

$ cmake .. -DWITH_TTF=Off

Disable SDL_net
---------------

$ cmake .. -DWITH_NET=Off

Disable SDL_image
-----------------

$ cmake .. -DWITH_IMAGE=Off

Disable installation of examples
--------------------------------

$ cmake .. -DWITH_DOCS=Off

Change the installation path of libraries
-----------------------------------------

$ cmake .. -DLUA_LIBDIR=/path/to/install/libraries

Note that this is relative to CMAKE_INSTALL_PREFIX.

Change the installation directory of examples
---------------------------------------------

$ cmake .. -DWITH_DOCSDIR=/path/to/install/examples

Note that this is relative to CMAKE_INSTALL_PREFIX.

Changing the Lua version
------------------------

You can change the Lua version by setting the WITH_LUAVER CMake variable;
supported values are:

* 53, Lua 5.3
* 52, Lua 5.2 (default)
* 51, Lua 5.1
* JIT, LuaJIT

$ cmake .. -DWITH_LUAVER=JIT

Changing the Lua location
-------------------------

It is sometimes useful to explicitly specify the path to the selected Lua version's include and library directories. The variables to set depend on the configured version:</p>

    $ cmake .. -DWITH_LUAVER=53 -DLUA53_INCLUDE_DIR=/path/to/headers/

    $ cmake .. -DWITH_LUAVER=52 -DLUA52_INCLUDE_DIR=/path/to/headers/

    $ cmake .. -DWITH_LUAVER=51 -DLUA_INCLUDE_DIR=/path/to/headers/

    $ cmake .. -DWITH_LUAVER=JIT -DLUAJIT_INCLUDE_DIR=/path/to/headers/

Also note that the include directory should be the one directly containing
the headers. We `#include <lua.h>`, not `#include <lua/lua.h>`.
