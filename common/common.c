/*
 * common.c -- common code for LuaSDL2
 *
 * Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#if LUA_VERSION_NUM == 501

void *
luaL_testudata(lua_State *L, int index, const char *tname)
{
	void *p = lua_touserdata(L, index);

	if (p != NULL) {
		if (lua_getmetatable(L, index)) {
			luaL_getmetatable(L, tname);

			if (!lua_rawequal(L, -1, -2))
				p = NULL;

			lua_pop(L, 2);

			return p;
		}
	}

	return NULL;
}

void
lua_rawsetp(lua_State *L, int index, void *p)
{
	int realindex = ((index) < 0) ? (index) - 1 : (index);

	lua_pushlightuserdata(L, p);
	lua_insert(L, -2);
	lua_rawset(L, realindex);
}

#endif

void
commonBindEnum(lua_State *L,
		 int tindex,
		 const char *tname,
		 const CommonEnum *values)
{
	int i;

	lua_createtable(L, 0, 0);

	for (i = 0; values[i].name != NULL; ++i) {
		lua_pushinteger(L, values[i].value);
		lua_setfield(L, -2, values[i].name);
	}

	if (tindex < 0)
		tindex --;

	lua_setfield(L, tindex, tname);
}

int
commonGetEnum(lua_State *L, int tindex)
{
	int value = 0;

	if (lua_type(L, tindex) == LUA_TNUMBER) {
		value = (int)lua_tonumber(L, tindex);
	} else if (lua_type(L, tindex) == LUA_TTABLE) {
		if (tindex < 0)
			tindex --;

		lua_pushnil(L);
		while (lua_next(L, tindex) != 0) {
			if (lua_type(L, -1) == LUA_TNUMBER)
				value |= lua_tointeger(L, -1);

			lua_pop(L, 1);
		}
	}

	return value;
}

void
commonPushEnum(lua_State *L, int value, const CommonEnum *evalue)
{
	int i;

	lua_createtable(L, 0, 0);

	for (i = 0; evalue[i].name != NULL; ++i) {
		/*
		 * Put as a map like table[i] = i so it's cleaner and
		 * more convenient for the user. The user is also able to reuse
		 * the table.
		 */
		if (value & evalue[i].value) {
			lua_pushinteger(L, evalue[i].value);
			lua_rawseti(L, -2, evalue[i].value);
		}
	}
}

void
commonBindObject(lua_State *L, const CommonObject *def)
{
	luaL_newmetatable(L, def->name);

	if (def->metamethods != NULL) {
#if LUA_VERSION_NUM >= 502
		luaL_setfuncs(L, def->metamethods, 0);
#else
		luaL_register(L, NULL, def->metamethods);
#endif
	}

	if (def->methods != NULL) {
		lua_createtable(L, 0, 0);
#if LUA_VERSION_NUM >= 502
		luaL_setfuncs(L, def->methods, 0);
#else
		luaL_register(L, NULL, def->methods);
#endif
		lua_setfield(L, -2, "__index");
	}

	lua_pop(L, 1);
}

void
commonBindLibrary(lua_State *L, const luaL_Reg *functions)
{
#if LUA_VERSION_NUM >= 502
	luaL_setfuncs(L, functions, 0);
#else
	luaL_register(L, NULL, functions);
#endif
}

void
commonNewLibrary(lua_State *L, const luaL_Reg *functions)
{
#if LUA_VERSION_NUM >= 502
	lua_createtable(L, 0, 0);
	luaL_setfuncs(L, functions, 0);
#else
	lua_createtable(L, 0, 0);
	luaL_register(L, NULL, functions);
#endif
}

CommonUserdata *
commonPushUserdata(lua_State *L, const char *tname, void *data)
{
	CommonUserdata *ptr;

	ptr = lua_newuserdata(L, sizeof (CommonUserdata));
	ptr->mustdelete = 1;
	ptr->data = data;

#if LUA_VERSION_NUM >= 502
	luaL_setmetatable(L, tname);
#else
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
#endif

	return ptr;
}

CommonUserdata *
commonGetUserdata(lua_State *L, int index, const char *tname)
{
	return luaL_checkudata(L, index, tname);
}

int
commonPushSDLError(lua_State *L, int count)
{
	int i;

	for (i = 0; i < count; ++i)
		lua_pushnil(L);

	lua_pushstring(L, SDL_GetError());

	return count + 1;
}

int
commonPushErrno(lua_State *L, int count)
{
	int i;

	for (i = 0; i < count; ++i)
		lua_pushnil(L);

	lua_pushstring(L, strerror(errno));

	return count + 1;
}

int
commonPush(lua_State *L, const char *fmt, ...)
{
	va_list ap;
	const char *p;
	int count = 0;

	va_start(ap, fmt);

	for (p = fmt; *p != '\0'; ++p) {
		switch (*p) {
		case 'i':
			lua_pushinteger(L, va_arg(ap, int));
			++ count;
			break;
		case 'd':
			lua_pushnumber(L, va_arg(ap, double));
			++ count;
			break;
		case 's':
			lua_pushstring(L, va_arg(ap, const char *));
			++ count;
			break;
		case 'b':
			lua_pushboolean(L, va_arg(ap, int));
			++ count;
			break;
		case 'l':
			lua_pushinteger(L, va_arg(ap, long));
			++ count;
			break;
		case 'n':
			lua_pushnil(L);
			++ count;
			break;
		case 'p':
		{
			const char *tname = va_arg(ap, const char *);
			void *udata = va_arg(ap, void *);

			commonPushUserdata(L, tname, udata);
			++ count;
		}
			break;
		default:
			break;
		}
	}
	
	return count;
}
