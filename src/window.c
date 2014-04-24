/*
 * window.c -- window management
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

#include <common/surface.h>
#include <common/table.h>
#include <common/video.h>

#include "window.h"

/* --------------------------------------------------------
 * Window functions
 * -------------------------------------------------------- */

/*
 * SDL.createWindow(params)
 *
 * The table params may or must have the following fields:
 *	title (optional) the window title
 *	width (optional) the window width
 *	height (optional) the window height
 *	x (optional) the x position
 *	y (optional) the y position
 *	flags (optional) the enum flags (SDL.windowFlags)
 *
 * Returns:
 *	The window object or nil on failure
 *	The error message
 */
static int
l_createWindow(lua_State *L)
{
	const char *name = "lua-SDL2 Window";
	int x = SDL_WINDOWPOS_UNDEFINED;
	int y = SDL_WINDOWPOS_UNDEFINED;
	int width = 50;
	int height = 50;
	int flags = 0;
	SDL_Window *win;

	/*
	 * For convenience, use the parameters as a table. We use default
	 * variables so every field are optional. We use a size of 50 to be
	 * a minimum seekable on the screen.
	 */
	luaL_checktype(L, 1, LUA_TTABLE);

	if (tableIsType(L, 1, "title", LUA_TSTRING))
		name = tableGetString(L, 1, "title");
	if (tableIsType(L, 1, "width", LUA_TNUMBER))
		width = tableGetInt(L, 1, "width");
	if (tableIsType(L, 1, "height", LUA_TNUMBER))
		height = tableGetInt(L, 1, "height");
	if (tableIsType(L, 1, "x", LUA_TNUMBER))
		x = tableGetInt(L, 1, "x");
	if (tableIsType(L, 1, "y", LUA_TNUMBER))
		y = tableGetInt(L, 1, "y");
	if (tableIsType(L, 1, "flags", LUA_TTABLE))
		flags = tableGetEnum(L, 1, "flags");

	win = SDL_CreateWindow(name, x, y, width, height, flags);
	if (win == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", WindowName, win);
}

const struct luaL_Reg WindowFunctions[] = {
	{ "createWindow",	l_createWindow		},
	{ NULL,			NULL			}
};

/* --------------------------------------------------------
 * Window object methods
 * -------------------------------------------------------- */

/*
 * Window:getBrightness()
 *
 * Returns:
 *	The brightness
 */
static int
l_window_getBrightness(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	return commonPush(L, "d", SDL_GetWindowBrightness(w));
}

/*
 * Window:getDisplayIndex()
 *
 * Returns:
 *	The display index
 */
static int
l_window_getDisplayIndex(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	return commonPush(L, "i", SDL_GetWindowDisplayIndex(w));
}

/*
 * Window:getDisplayMode()
 *
 * Returns:
 *	The display mode or nil on failure
 *	The error message
 */
static int
l_window_getDisplayMode(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	SDL_DisplayMode mode;

	if (SDL_GetWindowDisplayMode(w, &mode) < 0)
		return commonPushSDLError(L, 1);

	videoPushDisplayMode(L, &mode);

	return 1;
}

/*
 * Window:getFlags()
 *
 * Returns:
 *	The flags as enum table
 */
static int
l_window_getFlags(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	commonPushEnum(L, SDL_GetWindowFlags(w), WindowFlags);

	return 1;
}

/*
 * Window:getGammaRamp()
 *
 * Returns:
 *	A sequence table with 3 tables of 256 integers in the following order: red, green blue or nil
 *	The error message
 */
static int
l_window_getGammaRamp(lua_State *L)
{
#define REGISTER(which)							\
	do {								\
		int i;							\
									\
		lua_createtable(L, 256, 256);				\
		for (i = 0; i < 256; ++i) {				\
			lua_pushinteger(L, (which)[i]);			\
			lua_rawseti(L, -2, i + 1);			\
		}							\
		lua_rawseti(L, -2, c++);				\
	} while (/* CONSTCOND */ 0)

	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	Uint16 red[256];
	Uint16 green[256];
	Uint16 blue[256];
	int c;

	if (SDL_GetWindowGammaRamp(w, red, green, blue) < 0)
		return commonPushSDLError(L, 1);

	lua_createtable(L, 3, 3);
	c = 1;

	REGISTER(red);
	REGISTER(green);
	REGISTER(blue);

	return 1;
}

/*
 * Window:getGrab()
 *
 * Returns:
 *	The grab mode
 */
static int
l_window_getGrab(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	return commonPush(L, "b", SDL_GetWindowGrab(w));
}

/*
 * Window:getID()
 *
 * Returns:
 *	The window id
 */
static int
l_window_getID(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	return commonPush(L, "i", SDL_GetWindowID(w));
}

/*
 * Window:getMaximumSize()
 *
 * Returns:
 *	The max width
 *	The max height
 */
static int
l_window_getMaximumSize(lua_State *L)
{
	SDL_Window *win = commonGetAs(L, 1, WindowName, SDL_Window *);
	int w, h;

	SDL_GetWindowMaximumSize(win, &w, &h);

	return commonPush(L, "ii", w, h);
}

/*
 * Window:getMinimumSize()
 *
 * Returns:
 *	The minimum width
 *	The minimum height
 */
static int
l_window_getMinimumSize(lua_State *L)
{
	SDL_Window *win = commonGetAs(L, 1, WindowName, SDL_Window *);
	int w, h;

	SDL_GetWindowMinimumSize(win, &w, &h);

	return commonPush(L, "ii", w, h);
}

/*
 * Window:getPixelFormat()
 *
 * Returns:
 *	The pixel format (SDL.pixelFormat)
 */
static int
l_window_getPixelFormat(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	return commonPush(L, "i", SDL_GetWindowPixelFormat(w));
}

/*
 * Window:getPosition()
 *
 * Returns:
 *	The x
 *	The y
 */
static int
l_window_getPosition(lua_State *L)
{
	SDL_Window *win = commonGetAs(L, 1, WindowName, SDL_Window *);
	int x, y;

	SDL_GetWindowPosition(win, &x, &y);

	return commonPush(L, "ii", x, y);
}

/*
 * Window:getSurface()
 *
 * Returns:
 *	The surface object or nil
 *	The error message
 */
static int
l_window_getSurface(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	SDL_Surface *s;

	CommonUserdata *udata;

	s = SDL_GetWindowSurface(w);

	if (s == NULL)
		return commonPushSDLError(L, 1);

	/* The surface is managed by the window, user should not delete it */
	udata = commonPushUserdata(L, SurfaceName, s);
	udata->mustdelete = 0;

	return 1;
}

/*
 * Window:getSize()
 *
 * Returns:
 *	The width
 *	The height
 */
static int
l_window_getSize(lua_State *L)
{
	SDL_Window *win = commonGetAs(L, 1, WindowName, SDL_Window *);
	int w, h;

	SDL_GetWindowSize(win, &w, &h);

	return commonPush(L, "ii", w, h);
}

/*
 * Window:getTitle()
 *
 * Returns:
 *	The window title
 */
static int
l_window_getTitle(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	return commonPush(L, "s", SDL_GetWindowTitle(w));
}

/*
 * Window:hide()
 */
static int
l_window_hide(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	SDL_HideWindow(w);

	return 0;
}

/*
 * Window:maximize()
 */
static int
l_window_maximize(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	SDL_MaximizeWindow(w);

	return 0;
}

/*
 * Window:minimize()
 */
static int
l_window_minimize(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	SDL_MinimizeWindow(w);

	return 0;
}

/*
 * Window:raise()
 */
static int
l_window_raise(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	SDL_RaiseWindow(w);

	return 0;
}

/*
 * Window:restore()
 */
static int
l_window_restore(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	SDL_RestoreWindow(w);

	return 0;
}

/*
 * Window:setBrightness(level)
 *
 * Arguments:
 *	level the brightness level
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_window_setBrightness(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	float brightness = (float)luaL_checknumber(L, 2);

	if (SDL_SetWindowBrightness(w, brightness) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Window:setDisplayMode(mode)
 *
 * Arguments:
 *	mode the mode
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_window_setDisplayMode(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	SDL_DisplayMode mode;

	videoGetDisplayMode(L, 2, &mode);

	if (SDL_SetWindowDisplayMode(w, &mode) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Window:setFullscreen(mode)
 *
 * Arguments:
 *	mode the mode
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_window_setFullscreen(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	Uint32 flags = luaL_checkinteger(L, 2);

	if (SDL_SetWindowFullscreen(w, flags) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Window:setGammaRamp(gamma)
 *
 * Arguments:
 *	gamma a table with 3 arrays of 256 elements representing the channel in the order (red, green, blue)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_window_setGammaRamp(lua_State *L)
{
#define GET(which)							\
	do {								\
		int i;							\
									\
		lua_rawgeti(L, 2, c++);					\
		luaL_checktype(L, -1, LUA_TTABLE);			\
									\
		for (i = 0; i < 256; ++i) {				\
			lua_rawgeti(L, -1, i + 1);			\
									\
			if (lua_type(L, -1) == LUA_TNUMBER)		\
				which[i] = lua_tointeger(L, -1);	\
			else						\
				which[i] = 0;				\
									\
			lua_pop(L, 1);					\
		}							\
									\
		lua_pop(L, 1);						\
	} while (/* CONSTCOND */ 0)

	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	Uint16 red[256];
	Uint16 green[256];
	Uint16 blue[256];
	int c = 1;

	luaL_checktype(L, 2, LUA_TTABLE);

	GET(red);
	GET(green);
	GET(blue);

	if (SDL_SetWindowGammaRamp(w, red, green, blue) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Window:setGrab(mode)
 *
 * Arguments:
 *	mode the mode
 */
static int
l_window_setGrab(lua_State *L)
{
	SDL_Window *w	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int grabbed	= luaL_checkinteger(L, 2);

	SDL_SetWindowGrab(w, grabbed);

	return 0;
}

/*
 * Window:setIcon(surface)
 *
 * Arguments:
 *	surface the surface
 */
static int
l_window_setIcon(lua_State *L)
{
	SDL_Window *w	= commonGetAs(L, 1, WindowName, SDL_Window *);
	SDL_Surface *s	= commonGetAs(L, 2, SurfaceName, SDL_Surface *);

	SDL_SetWindowIcon(w, s);

	return 0;
}

/*
 * Window:setMaximumSize(w, h)
 *
 * Arguments:
 *	w the width
 *	h the height
 */
static int
l_window_setMaximumSize(lua_State *L)
{
	SDL_Window *w	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int maxw	= luaL_checkinteger(L, 2);
	int maxh	= luaL_checkinteger(L, 3);

	SDL_SetWindowMaximumSize(w, maxw, maxh);

	return 0;
}

/*
 * Window:setMinimumSize(w, h)
 *
 * Arguments:
 *	w the width
 *	h the height
 */
static int
l_window_setMinimumSize(lua_State *L)
{
	SDL_Window *w	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int maxw	= luaL_checkinteger(L, 2);
	int maxh	= luaL_checkinteger(L, 3);

	SDL_SetWindowMinimumSize(w, maxw, maxh);

	return 0;
}

/*
 * Window:setPosition(x, y)
 *
 * Arguments:
 *	x the x
 *	y the y
 */
static int
l_window_setPosition(lua_State *L)
{
	SDL_Window *w	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int x		= luaL_checkinteger(L, 2);
	int y		= luaL_checkinteger(L, 3);

	SDL_SetWindowPosition(w, x, y);

	return 0;
}

/*
 * Window:setSize(w, h)
 *
 * Arguments:
 *	w the width
 *	h the height
 */
static int
l_window_setSize(lua_State *L)
{
	SDL_Window *win	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int w		= luaL_checkinteger(L, 2);
	int h		= luaL_checkinteger(L, 3);

	SDL_SetWindowSize(win, w, h);

	return 0;
}

/*
 * Window:setTitle(title)
 *
 * Arguments:
 *	title the title
 */
static int
l_window_setTitle(lua_State *L)
{
	SDL_Window *w		= commonGetAs(L, 1, WindowName, SDL_Window *);
	const char *title	= luaL_checkstring(L, 2);

	SDL_SetWindowTitle(w, title);

	return 0;
}

/*
 * Window:show()
 */
static int
l_window_show(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	SDL_ShowWindow(w);

	return 0;
}

/*
 * Window:updateSurface()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_window_updateSurface(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);

	if (SDL_UpdateWindowSurface(w) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Window:updateSurfaceRects(rects)
 *
 * Arguments:
 *	rects the rectangles
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_window_updateSurfaceRects(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	Array rects;
	int ret;

	luaL_checktype(L, 2, LUA_TTABLE);

	if (videoGetRects(L, 2, &rects) < 0)
		return commonPushErrno(L, 1);

	ret = SDL_UpdateWindowSurfaceRects(w, rects.data, rects.length);
	arrayFree(&rects);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Window:warpMouse(x, y)
 *
 * Arguments:
 *	x the x
 *	y the y
 */
static int
l_window_warpMouse(lua_State *L)
{
	SDL_Window *w	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int x		= luaL_checkinteger(L, 2);
	int y		= luaL_checkinteger(L, 3);

	SDL_WarpMouseInWindow(w, x, y);

	return 0;
}

/* --------------------------------------------------------
 * Window object metamethods
 * -------------------------------------------------------- */

/*
 * Window:__gc()
 */
static int
l_window_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, WindowName);

	if (udata->mustdelete)
		SDL_DestroyWindow(udata->data);

	return 0;
}

/*
 * Window:__tostring()
 */
static int
l_window_tostring(lua_State *L)
{
	SDL_Window *win = commonGetAs(L, 1, WindowName, SDL_Window *);
	const char *title;
	int w, h;

	title = SDL_GetWindowTitle(win);
	SDL_GetWindowSize(win, &w, &h);

	lua_pushfstring(L, "window \"%s\": size %dx%d", title, w, h);

	return 1;
}

/* --------------------------------------------------------
 * Window object definition
 * -------------------------------------------------------- */

static const luaL_Reg WindowMethods[] = {
	{ "getBrightness",	l_window_getBrightness		},
	{ "getDisplayIndex",	l_window_getDisplayIndex	},
	{ "getDisplayMode",	l_window_getDisplayMode		},
	{ "getFlags",		l_window_getFlags		},
	{ "getGammaRamp",	l_window_getGammaRamp		},
	{ "getGrab",		l_window_getGrab		},
	{ "getID",		l_window_getID			},
	{ "getMaximumSize",	l_window_getMaximumSize		},
	{ "getMinimumSize",	l_window_getMinimumSize		},
	{ "getPixelFormat",	l_window_getPixelFormat		},
	{ "getPosition",	l_window_getPosition		},
	{ "getSurface",		l_window_getSurface		},
	{ "getSize",		l_window_getSize		},
	{ "getTitle",		l_window_getTitle		},
	{ "hide",		l_window_hide			},
	{ "maximize",		l_window_maximize		},
	{ "minimize",		l_window_minimize		},
	{ "raise",		l_window_raise			},
	{ "restore",		l_window_restore		},
	{ "setBrightness",	l_window_setBrightness		},
	{ "setDisplayMode",	l_window_setDisplayMode		},
	{ "setFullscreen",	l_window_setFullscreen		},
	{ "setGammaRamp",	l_window_setGammaRamp		},
	{ "setGrab",		l_window_setGrab		},
	{ "setIcon",		l_window_setIcon		},
	{ "setMaximumSize",	l_window_setMaximumSize		},
	{ "setMinimumSize",	l_window_setMinimumSize		},
	{ "setPosition",	l_window_setPosition		},
	{ "setSize",		l_window_setSize		},
	{ "setTitle",		l_window_setTitle		},
	{ "show",		l_window_show			},
	{ "updateSurface",	l_window_updateSurface		},
	{ "updateSurfaceRects",	l_window_updateSurfaceRects	},
	{ "warpMouse",		l_window_warpMouse		},
	{ NULL,			NULL				}
};

static const luaL_Reg WindowMetamethods[] = {
	{ "__gc",		l_window_gc			},
	{ "__tostring",		l_window_tostring		},
	{ NULL,			NULL				}
};

const CommonObject Window = {
	"Window",
	WindowMethods,
	WindowMetamethods
};

/*
 * SDL.window
 */
const CommonEnum WindowFlags[] = {
	{ "Fullscreen",			SDL_WINDOW_FULLSCREEN		},
	{ "Desktop",			SDL_WINDOW_FULLSCREEN_DESKTOP	},
	{ "OpenGL",			SDL_WINDOW_OPENGL		},
	{ "Shown",			SDL_WINDOW_SHOWN		},
	{ "Hidden",			SDL_WINDOW_HIDDEN		},
	{ "Borderless",			SDL_WINDOW_BORDERLESS		},
	{ "Resizable",			SDL_WINDOW_RESIZABLE		},
	{ "Minimized",			SDL_WINDOW_MINIMIZED		},
	{ "Maximized",			SDL_WINDOW_MAXIMIZED		},
	{ "InputGrabbed",		SDL_WINDOW_INPUT_GRABBED	},
	{ "InputFocused",		SDL_WINDOW_INPUT_FOCUS		},
	{ "MouseFocused",		SDL_WINDOW_MOUSE_FOCUS		},
	{ "Foreign",			SDL_WINDOW_FOREIGN		},
	{ NULL,				-1				}
};
