/*
 * common.h -- common code for lua-SDL2
 *
 * Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
 * Copyright (c) 2014 Joseph Wallace <tangent128@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdarg.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <SDL.h>

/*
 * Portability bits
 */
#if defined(_WIN32)
#  define EXPORT	__declspec(dllexport)
#  define strdup	_strdup
#else
#  define EXPORT
#endif

/*
 * Portability bits for different Lua versions
 */
#if LUA_VERSION_NUM == 501

#  define LUA_OK			0
#  define lua_load(l, r, d, c, m)		lua_load(l, r, d, c)

void *
luaL_testudata(lua_State *L, int index, const char *tname);

void
lua_rawsetp(lua_State *L, int index, void *key);

#endif

#if LUA_VERSION_NUM < 503

#  define lua_dump(l, w, d, s)		lua_dump(l, w, d)

#endif

/**
 * @struct common_evalue
 * @brief bind C enum as tables to Lua
 *
 * This is used with commonBindEnum().
 *
 * @see commonBindEnum
 */
typedef struct {
	const char	*name;			/*! the field name */
	int		value;			/*! the integer value */
} CommonEnum;

/**
 * @CommonObject
 * @brief bind a C object to Lua
 *
 * This is used with commonBindObject().
 *
 * @see commonBindObject
 */
typedef struct {
	const char	*name;			/*! metatable name */
	const luaL_Reg	*methods;		/*! methods (optional) */
	const luaL_Reg	*metamethods;		/*! metamethods (optional) */
} CommonObject;

/**
 * Wrapper for SDL object to Lua userdata. Some objects must be passed
 * so that they will not be freed, for instance SDL_GetWindowSurface() returns
 * a pointer that should not be freed while SDL_Surface objects has a __gc
 * that destroy the object.
 *
 * So any object should be passed as common_userdata so we know if we should
 * really delete the object or just let it live.
 */
typedef struct {
	int		mustdelete;		/*! tells if we should delete it */
	void		*data;			/*! the data itself */
} CommonUserdata;

/**
 * Bind a C enum as a table to Lua.
 *
 * @param L the Lua state
 * @param tindex the table index where to set the table
 * @param tname the new table name as tindex new field
 * @param evalues the values
 */
void
commonBindEnum(lua_State *L,
		 int tindex,
		 const char *tname,
		 const CommonEnum *values);

/**
 * Get a enumeration from a Lua table.
 *
 * @param L the Lua state
 * @param tindex the table index
 * @return the value or 0
 */
int
commonGetEnum(lua_State *L, int tindex);

/**
 * Push a enumeration as a map of enum to bool. That is if flag A is set and B
 * too on value, the pushed table will looks like this:
 *
 * table[A] = true
 * table[B] = true
 *
 * @param L the Lua state
 * @param value the value
 * @param evalue the enumeration to check
 */
void
commonPushEnum(lua_State *L, int value, const CommonEnum *evalue);

/**
 * Bind an object to Lua.
 *
 * @param L the Lua state
 * @param def the object definition
 */
void
commonBindObject(lua_State *L, const CommonObject *def);

/**
 * This function binds all functions to the already created table SDL.
 *
 * @param L the Lua state
 * @param functions the functions
 */
void
commonBindLibrary(lua_State *L, const luaL_Reg *functions);

/**
 * Create a table and fills the functions in it.
 *
 * @param L the Lua state
 * @param functions the functions
 */
void
commonNewLibrary(lua_State *L, const luaL_Reg *functions);

/**
 * Push a SDL object to Lua as an userdata. By default, the object
 * is set to be deleted.
 *
 * @param L the Lua state
 * @param tname the metatable name to use
 * @param data the pointer to the data
 * @return the new created object
 */
CommonUserdata *
commonPushUserdata(lua_State *L, const char *tname, void *data);

/**
 * Get a SDL object from Lua.
 *
 * @param L the Lua state
 * @param index the index
 * @param tname the object metatable name
 * @return the object or raise an error
 */
CommonUserdata *
commonGetUserdata(lua_State *L, int index, const char *tname);

/**
 * Pushes count * nil + the SDL_GetError() message.
 *
 * @param L the Lua state
 * @param count number of nil to pushes before
 * @return count
 */
int
commonPushSDLError(lua_State *L, int count);

/**
 * Pushes the errno error.
 *
 * @param L the Lua state
 * @param count number of nil to pushes before
 * @return count
 */
int
commonPushErrno(lua_State *L, int count);

/**
 * Convenient wrapper for pushing values.
 * Format supported:
 *	i -> integer
 *	d -> double
 *	s -> string
 *	b -> boolean
 *	l -> long
 *	n -> nil (no arg)
 *	p -> userdata, userdata-name (two args)
 *
 * @param L the Lua state
 * @param fmt the format
 * @return the number of values pushed
 */
int
commonPush(lua_State *L, const char *fmt, ...);

#define commonGetAs(L, index, name, type)				\
	((type)commonGetUserdata(L, index, name)->data)

#endif /* !_COMMON_H_ */
