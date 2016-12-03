/*
 * mouse.c -- mouse event management
 *
 * Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
 * Copyright (c) 2016 Webster Sheets <webster@web-eworks.com>
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

#include <common/surface.h>

#include "mouse.h"
#include "window.h"

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.captureMouse(enabled)
 *
 * Arguments:
 *	enabled enables or disables global mouse event capturing.
 *
 * Returns:
 *	true on success, nil if not supported
 *	an error message on failure
 */
static int
l_captureMouse(lua_State *L)
{
	SDL_bool enabled = lua_toboolean(L, 1);

	if (SDL_CaptureMouse(enabled) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}
#endif

/*
 * SDL.createColorCursor(s, x, y)
 *
 * Arguments:
 *	s the surface
 *	x the hot x
 *	y the hot y
 *
 * Returns:
 *	A cursor object or nil on failure
 *	The error message
 */
static int
l_createColorCursor(lua_State *L)
{
	SDL_Surface *s	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	int hot_x	= luaL_checkinteger(L, 1);
	int hot_y	= luaL_checkinteger(L, 2);
	SDL_Cursor *c;

	c = SDL_CreateColorCursor(s, hot_x, hot_y);
	if (c == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", MouseCursorName, c);
}

/*
 * SDL.createCursor(data, mask, w, h, x, y)
 *
 * Arguments:
 *	data the data
 *	mask the mask
 *	w the width
 *	h the height
 *	x the x
 *	y the y
 *
 * Returns:
 *	The cursor object or nil on failure
 *	The error message
 */
static int
l_createCursor(lua_State *L)
{
	const Uint8 *data	= (const Uint8 *)luaL_checkstring(L, 1);
	const Uint8 *mask	= (const Uint8 *)luaL_checkstring(L, 2);
	int w			= luaL_checkinteger(L, 3);
	int h			= luaL_checkinteger(L, 4);
	int hot_x		= luaL_checkinteger(L, 5);
	int hot_y		= luaL_checkinteger(L, 6);

	SDL_Cursor *c = SDL_CreateCursor(data, mask, w, h, hot_x, hot_y);
	if (c == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", MouseCursorName, c);
}

/*
 * SDL.getCursor()
 *
 * Returns:
 *	The cursor object or nil
 */
static int
l_getCursor(lua_State *L)
{
	SDL_Cursor *c = SDL_GetCursor();

	if (c == NULL)
		return commonPush(L, "n");

	return commonPush(L, "p", MouseCursorName, c);
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.getGlobalMouseState()
 *
 * Returns:
 *	A table enum with buttons
 */
static int
l_getGlobalMouseState(lua_State *L)
{
	int x, y;
	Uint32 state;

	state = SDL_GetGlobalMouseState(&x, &y);

	commonPushEnum(L, state, MouseMask);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	return 3;
}
#endif

/*
 * SDL.getMouseFocus()
 *
 * Returns:
 *	The window object
 */
static int
l_getMouseFocus(lua_State *L)
{
	SDL_Window *w = SDL_GetMouseFocus();
	CommonUserdata *data;

	data = commonPushUserdata(L, WindowName, w);
	data->mustdelete = 0;

	return 1;
}

/*
 * SDL.getMouseState()
 *
 * Returns:
 *	A table enum with buttons
 */
static int
l_getMouseState(lua_State *L)
{
	int x, y;
	Uint32 state;

	state = SDL_GetMouseState(&x, &y);

	commonPushEnum(L, state, MouseMask);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	return 3;
}

/*
 * SDL.getRelativeMouseMode()
 *
 * Returns:
 *	True if relative
 */
static int
l_getRelativeMouseMode(lua_State *L)
{
	return commonPush(L, "b", SDL_GetRelativeMouseMode() == 0);
}

/*
 * SDL.getRelativeMouseState()
 *
 * Returns:
 *	The button state as a enum table
 *	The x
 *	The y
 */
static int
l_getRelativeMouseState(lua_State *L)
{
	int x, y;
	Uint32 state;

	state = SDL_GetRelativeMouseState(&x, &y);

	commonPushEnum(L, state, MouseMask);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	return 0;
}

/*
 * SDL.setCursor(c)
 *
 * Arguments:
 *	c the cursor object
 */
static int
l_setCursor(lua_State *L)
{
	SDL_Cursor *c = commonGetAs(L, 1, MouseCursorName, SDL_Cursor *);

	SDL_SetCursor(c);

	return 0;
}

/*
 * SDL.setRelativeMouseMode(mode)
 *
 * Arguments:
 *	mode true or false
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_setRelativeMouseMode(lua_State *L)
{
	int enabled = lua_toboolean(L, 1);

	if (SDL_SetRelativeMouseMode(enabled) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.showCursor(mode)
 *
 * Arguments:
 *	mode the mode
 *
 * Returns:
 *	The old mode
 */
static int
l_showCursor(lua_State *L)
{
	int toggle = lua_toboolean(L, 1);

	return commonPush(L, "i", SDL_ShowCursor(toggle));
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.warpMouseGlobal(x, y)
 *
 * Arguments:
 *	x the x
 *	y the y
 */
static int
l_warpMouseGlobal(lua_State *L)
{
	int x		= luaL_checkinteger(L, 1);
	int y		= luaL_checkinteger(L, 2);

	SDL_WarpMouseGlobal(x, y);

	return 0;
}
#endif

const luaL_Reg MouseFunctions[] = {
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "captureMouse",		l_captureMouse		},
#endif
	{ "createColorCursor",		l_createColorCursor	},
	{ "createCursor",		l_createCursor		},
	{ "getCursor",			l_getCursor		},
	{ "getMouseFocus",		l_getMouseFocus		},
	{ "getMouseState",		l_getMouseState		},
	{ "getRelativeMouseMode",	l_getRelativeMouseMode	},
	{ "getRelativeMouseState",	l_getRelativeMouseState	},
	{ "setCursor",			l_setCursor		},
	{ "setRelativeMouseMode",	l_setRelativeMouseMode	},
	{ "showCursor",			l_showCursor		},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "getGlobalMouseState",	l_getGlobalMouseState	},
	{ "warpMouseGlobal",		l_warpMouseGlobal	},
#endif
	{ NULL,				NULL			}
};

/* --------------------------------------------------------
 * Cursor object metamethods
 * -------------------------------------------------------- */

static int
l_cursor_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, MouseCursorName);

	if (udata->mustdelete)
		SDL_FreeCursor(udata->data);

	return 0;
}

static const luaL_Reg metamethods[] = {
	{ "__gc",			l_cursor_gc			},
	{ NULL,				NULL				}
};

const CommonObject MouseCursor = {
	"Cursor",
	NULL,
	metamethods
};

/*
 * SDL.mouseButton
 */
const CommonEnum MouseButtons[] = {
	{ "Left",			SDL_BUTTON_LEFT		},
	{ "Right",			SDL_BUTTON_RIGHT	},
	{ "Middle",			SDL_BUTTON_MIDDLE	},
	{ "X1",				SDL_BUTTON_X1		},
	{ "X2",				SDL_BUTTON_X2		},
	{ NULL,				-1			}
};

/*
 * SDL.mouseMask
 */
const CommonEnum MouseMask[] = {
	{ "Left",			SDL_BUTTON_LMASK	},
	{ "Right",			SDL_BUTTON_RMASK	},
	{ "Middle",			SDL_BUTTON_RMASK	},
	{ "X1",				SDL_BUTTON_X1MASK	},
	{ "X2",				SDL_BUTTON_X2MASK	},
	{ NULL,				-1			},
};

#if SDL_VERSION_ATLEAST(2, 0, 2)

/*
 * SDL.mouseClick
 */
const CommonEnum MouseClick[] = {
	{ "Single", 			1	},
	{ "Double",			2	},
	{ NULL,				-1	}
};

#endif
