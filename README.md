Lua-SDL2 (SDL2 binding for Lua)
==================================

Lua-SDL2 is a pure C binding of SDL2 to Lua 5.1, Lua 5.2, Lua 5.3, and LuaJIT.

The current version is 2.1, compatible with SDL 2.0.1 - 2.0.5.

Lua-SDL2 follows the [SemVer](semver.org) standard with regards to project
versioning.

Features
========

Lua-SDL2 is a portable binding of SDL2, written in pure C for efficiency.
It tries to stay as close to SDL as possible, acting as a simple binding
rather than attempting to re-design the interaction between the programmer
and SDL.

Lua-SDL2 takes full advantage of Lua's object-oriented capabilities wherever
possible, allowing the programmer to fully leverage SDL's inherent
object-oriented design.

Lua-SDL2 is very well documented, with copious source-code comments, and a full
API reference available at [the wiki](https://github.com/Tangent128/LuaSDL2/wiki/).

Compatability
=============

Lua-SDL2 is designed to be as compatible as possible.  The library has support
for all the latest features of SDL2, while still compiling with SDL 2.0.1.

If the library is compiled with a newer version of SDL than what is listed here,
the library should still function, simply without access to the newer features.

Lua-SDL2 is compatible with:

	* Lua 5.1, 5.2, 5.3, or LuaJIT
	* SDL 2.0.1 and greater

Installing
==========

If you have LuaRocks installed,

    $ luarocks install lua-sdl2

Otherwise, read `INSTALL.md` for instructions to build it yourself.

Website
=======

The official website is hosted at [https://github.com/Tangent128/luasdl2][].

The current documentation is available at the associated
[wiki](https://github.com/Tangent128/luasdl2/wiki/).

Author and Maintainer
=====================

The Lua-SDL2 library was written by David Demelier <markand@malikania.fr>

It is currently being maintained by Joseph Wallace <tangent128@gmail.com>
