/*
 * table.c -- table helpers
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

#include "table.h"

int
tableIsType(lua_State *L, int idx, const char *name, int type)
{
	int ret;

	lua_getfield(L, idx, name);
	ret = lua_type(L, -1);
	lua_pop(L, 1);

	return ret == type;
}

CommonUserdata *
tableGetUserdata(lua_State *L, int idx, const char *field, const char *tname)
{
	CommonUserdata *ptr = NULL;

	lua_getfield(L, idx, field);
	ptr = luaL_checkudata(L, -1, tname);
	lua_pop(L, 1);

	return ptr;
}

int
tableGetInt(lua_State *L, int idx, const char *name)
{
	int ret;

	lua_getfield(L, idx, name);

	if (lua_type(L, -1) == LUA_TNUMBER)
		ret = lua_tointeger(L, -1);
	else
		ret = 0;

	lua_pop(L, 1);

	return ret;
}

double
tableGetDouble(lua_State *L, int idx, const char *name)
{
	double ret;

	lua_getfield(L, idx, name);

	if (lua_type(L, -1) == LUA_TNUMBER)
		ret = lua_tonumber(L, -1);
	else
		ret = 0;

	lua_pop(L, 1);

	return ret;
}

int
tableGetEnum(lua_State *L, int idx, const char *name)
{
	int ret;
	
	lua_getfield(L, idx, name);

	if (lua_type(L, -1) == LUA_TTABLE)
		ret = commonGetEnum(L, -1);
	else
		ret = 0;

	lua_pop(L, 1);

	return ret;
}

const char *
tableGetString(lua_State *L, int idx, const char *name)
{
	const char *ret;

	lua_getfield(L, idx, name);

	if (lua_type(L, -1) == LUA_TSTRING)
		ret = lua_tostring(L, -1);
	else
		ret = NULL;

	lua_pop(L, 1);

	return ret;
}

const char *
tableGetStringl(lua_State *L, int idx, const char *name, size_t *length)
{
	const char *ret;

	lua_getfield(L, idx, name);

	if (lua_type(L, -1) == LUA_TSTRING)
		ret = lua_tolstring(L, -1, length);
	else
		ret = NULL;

	lua_pop(L, 1);

	return ret;
}

int
tableGetBool(lua_State *L, int idx, const char *name)
{
	int ret = 0;

	lua_getfield(L, idx, name);
	ret = lua_toboolean(L, -1);
	lua_pop(L, 1);

	return ret;
}

void
tableSetInt(lua_State *L, int idx, const char *name, int value)
{
	if (idx < 0)
		idx --;

	lua_pushinteger(L, value);
	lua_setfield(L, idx, name);
}

void
tableSetDouble(lua_State *L, int idx, const char *name, double value)
{
	if (idx < 0)
		idx --;

	lua_pushnumber(L, value);
	lua_setfield(L, idx, name);
}

void
tableSetString(lua_State *L, int idx, const char *name, const char *value)
{
	if (idx < 0)
		idx --;

	lua_pushstring(L, value);
	lua_setfield(L, idx, name);
}

void
tableSetStringl(lua_State *L, int idx, const char *name, const char *value, int length)
{
	if (idx < 0)
		idx --;

	lua_pushlstring(L, value, length);
	lua_setfield(L, idx, name);
}

void
tableSetBool(lua_State *L, int idx, const char *name, int value)
{
	if (idx < 0)
		idx --;

	lua_pushboolean(L, value);
	lua_setfield(L, idx, name);
}

void
tableSetEnum(lua_State *L,
	     int idx,
	     int value,
	     const CommonEnum *evalue,
	     const char *name)
{
	commonPushEnum(L, value, evalue);

	if (idx < 0)
		idx --;

	lua_setfield(L, idx, name);
}
