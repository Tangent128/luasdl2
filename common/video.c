/*
 * video.c -- video initialization and display management
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

#include <string.h>

#include <SDL.h>

#include "table.h"
#include "video.h"

/* --------------------------------------------------------
 * Tables callbacks for reading
 * -------------------------------------------------------- */

typedef int (*TableReadFunc)(lua_State *, Array *);

static int
readRects(lua_State *L, Array *rects)
{
	SDL_Rect r;

	/* Verify the point as a table */
	if (lua_type(L, -1) != LUA_TTABLE)
		return 0;

	r.w = tableGetInt(L, -1, "w");
	r.h = tableGetInt(L, -1, "h");
	r.x = tableGetInt(L, -1, "x");
	r.y = tableGetInt(L, -1, "y");

	return arrayAppend(rects, &r) != -1;
}

static int
readPoints(lua_State *L, Array *points)
{
	SDL_Point p;

	/* Verify the point as a table */
	if (lua_type(L, -1) != LUA_TTABLE)
		return 0;

	p.x = tableGetInt(L, -1, "x");
	p.y = tableGetInt(L, -1, "y");

	return arrayAppend(points, &p) != -1;
}

static int
readColors(lua_State *L, Array *colors)
{
	SDL_Color color;

	color = videoGetColorRGB(L, -1);

	return arrayAppend(colors, &color) != -1;
}

/**
 * This is a generic function for reading tables to fill an array. It calls
 * the function `func' for entry in the array.
 *
 * The function func should return 0 if we can continue, if the function
 * return -1, the array is free'd and we also return -1 to the caller.
 *
 * @param L the Lua state
 * @param index the table index
 * @param array the array to fill
 * @param unit the size of object
 * @param func the function to call
 * @return 0 on success and -1 on failure
 */
static int
readTable(lua_State *L, int index, Array *array, size_t unit, TableReadFunc func)
{
	int ok = 1;

	luaL_checktype(L, index, LUA_TTABLE);

	/* No memory */
	if (arrayInit(array, unit, 32) < 0)
		return -1;

	if (index < 0)
		index --;

	lua_pushnil(L);
	while (lua_next(L, index) != 0 && ok) {
		ok = func(L, array);
		lua_pop(L, 1);
	}

	/* Should not be needed, but in case of array.c update */
	if (!ok)
		arrayFree(array);

	return (ok) ? 0 : -1;
}

/* --------------------------------------------------------
 * Shared functions
 * -------------------------------------------------------- */

void
videoPushColorRGB(lua_State *L, const SDL_Color *color)
{
	lua_createtable(L, 0, 4);

	tableSetInt(L, -1, "r", color->r);
	tableSetInt(L, -1, "g", color->g);
	tableSetInt(L, -1, "b", color->b);
	tableSetInt(L, -1, "a", color->a);
}

void
videoPushRect(lua_State *L, const SDL_Rect *rect)
{
	lua_createtable(L, 4, 4);

	tableSetInt(L, -1, "w", rect->w);
	tableSetInt(L, -1, "h", rect->h);
	tableSetInt(L, -1, "x", rect->x);
	tableSetInt(L, -1, "y", rect->y);
}

void
videoGetRect(lua_State *L, int index, SDL_Rect *rect)
{
	luaL_checktype(L, index, LUA_TTABLE);

	rect->w = tableGetInt(L, index, "w");
	rect->h = tableGetInt(L, index, "h");
	rect->x = tableGetInt(L, index, "x");
	rect->y = tableGetInt(L, index, "y");
}

int
videoGetRects(lua_State *L, int index, Array *rects)
{
	return readTable(L, index, rects, sizeof (SDL_Rect), readRects);
}

void
videoPushPoint(lua_State *L, const SDL_Point *point)
{
	lua_createtable(L, 2, 2);

	tableSetInt(L, -1, "x", point->x);
	tableSetInt(L, -1, "y", point->y);
}

void
videoGetPoint(lua_State *L, int index, SDL_Point *point)
{
	luaL_checktype(L, index, LUA_TTABLE);

	point->x = tableGetInt(L, index, "x");
	point->y = tableGetInt(L, index, "y");
}

int
videoGetPoints(lua_State *L, int index, Array *points)
{
	return readTable(L, index, points, sizeof (SDL_Point), readPoints);
}

void
videoGetLine(lua_State *L, int index, Line *line)
{
	luaL_checktype(L, index, LUA_TTABLE);

	line->x1 = tableGetInt(L, index, "x1");
	line->y1 = tableGetInt(L, index, "y1");
	line->x2 = tableGetInt(L, index, "x2");
	line->y2 = tableGetInt(L, index, "y2");
}

Uint32
videoGetColorHex(lua_State *L, int index)
{
	Uint32 value = 0;

	if (lua_type(L, index) == LUA_TNUMBER) {
		value = lua_tointeger(L, index);
	} else if (lua_type(L, index) == LUA_TTABLE) {
		SDL_Color tmp;

		tmp.r = tableGetInt(L, index, "r") & 0xFF;
		tmp.g = tableGetInt(L, index, "g") & 0xFF;
		tmp.b = tableGetInt(L, index, "b") & 0xFF;
		tmp.a = tableGetInt(L, index, "a") & 0xFF;

		value = (tmp.r << 16) | (tmp.g << 8) | tmp.b | (tmp.a << 24);
	}

	return value;
}

SDL_Color
videoGetColorRGB(lua_State *L, int index)
{
	SDL_Color c = { 0, 0, 0, 0 };

	if (lua_type(L, index) == LUA_TNUMBER) {
		int value = lua_tointeger(L, index);

		c.a = ((value >> 24) & 0xFF);
		c.r = ((value >> 16) & 0xFF);
		c.g = ((value >> 8) & 0xFF);
		c.b = ((value) & 0xFF);
	} else if (lua_type(L, index) == LUA_TTABLE) {
		c.r = tableGetInt(L, index, "r");
		c.g = tableGetInt(L, index, "g");
		c.b = tableGetInt(L, index, "b");
		c.a = tableGetInt(L, index, "a");
	}

	return c;
}

int
videoGetColorsRGB(lua_State *L, int index, Array *colors)
{
	return readTable(L, index, colors, sizeof (SDL_Color), readColors);
}

void
videoGetDisplayMode(lua_State *L, int idx, SDL_DisplayMode *mode)
{
	luaL_checktype(L, idx, LUA_TTABLE);

	mode->format	= tableGetInt(L, idx, "format");
	mode->w		= tableGetInt(L, idx, "w");
	mode->h		= tableGetInt(L, idx, "h");
	mode->refresh_rate = tableGetInt(L, idx, "refreshRate");
	mode->driverdata = NULL;
}

void
videoPushDisplayMode(lua_State *L, const SDL_DisplayMode *mode)
{
	lua_createtable(L, 4, 4);

	tableSetInt(L, -1, "w", mode->w);
	tableSetInt(L, -1, "h", mode->h);
	tableSetInt(L, -1, "format", mode->format);
	tableSetInt(L, -1, "refreshRate", mode->refresh_rate);
}
