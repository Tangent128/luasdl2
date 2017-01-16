/*
 * display.c -- basic display functions
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

#include <common/video.h>

#include "display.h"

/*
 * SDL.disableScreenSaver()
 */
static int
l_video_disableScreenSaver(lua_State *L)
{
	SDL_DisableScreenSaver();

	(void)L;

	return 0;
}

/*
 * SDL.enableScreenSaver()
 */
static int
l_video_enableScreenSaver(lua_State *L)
{
	SDL_EnableScreenSaver();

	(void)L;

	return 0;
}

/*
 * SDL.getClosestDisplayMode(index, mode)
 *
 * Arguments:
 *	index the index
 *	mode the requested mode
 *
 * Returns:
 *	The closest display mode or nil
 *	The error message
 */
static int
l_video_getClosestDisplayMode(lua_State *L)
{
	SDL_DisplayMode mode, result;
	int index;

	index = luaL_checkinteger(L, 1);
	videoGetDisplayMode(L, 2, &mode);

	if (SDL_GetClosestDisplayMode(index, &mode, &result) == NULL)
		return commonPushSDLError(L, 1);

	videoPushDisplayMode(L, &result);

	return 1;
}

/*
 * SDL.getCurrentDisplayMode(index)
 *
 * Arguments:
 *	index the index
 *
 * Returns:
 *	The current display mode or nil
 *	The error message
 */
static int
l_video_getCurrentDisplayMode(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_DisplayMode current;

	if (SDL_GetCurrentDisplayMode(index, &current) < 0)
		return commonPushSDLError(L, 1);

	videoPushDisplayMode(L, &current);

	return 1;
}

/*
 * SDL.getCurrentVideoDriver()
 *
 * Returns:
 *	The current video driver name or nil if uninitialized
 */
static int
l_video_getCurrentVideoDriver(lua_State *L)
{
	const char *name = SDL_GetCurrentVideoDriver();

	if (name == NULL)
		lua_pushnil(L);
	else
		lua_pushstring(L, name);

	return 1;
}

/*
 * SDL.getDesktopDisplayMode(index)
 *
 * Returns:
 *	The display mode or nil on failure
 *	The error message
 */
static int
l_video_getDesktopDisplayMode(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_DisplayMode mode;

	if (SDL_GetDesktopDisplayMode(index, &mode) < 0)
		return commonPushSDLError(L, 1);

	videoPushDisplayMode(L, &mode);

	return 1;
}

/*
 * SDL.getDisplayBounds(index)
 *
 * Returns:
 *	The bounds rectangle or nil on failure
 *	The error message
 */
static int
l_video_getDisplayBounds(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_Rect ret;

	if (SDL_GetDisplayBounds(index, &ret) < 0)
		return commonPushSDLError(L, 1);

	videoPushRect(L, &ret);

	return 1;
}

#if SDL_VERSION_ATLEAST(2, 0, 5)
/*
 * SDL.getDisplayUsableBounds(index)
 *
 * Returns:
 *	The bounds rectangle or nil on failure
 *	The error message
 */
static int
l_video_getDisplayUsableBounds(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_Rect ret;

	if (SDL_GetDisplayUsableBounds(index, &ret) < 0)
		return commonPushSDLError(L, 1);

	videoPushRect(L, &ret);

	return 1;
}
#endif

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.getDisplayDPI(index)
 *
 * Arguments:
 *	index the display index
 *
 * Returns:
 *	The diagonal DPI of the display or nil on failure
 *	The horizontal DPI of the display, or an error message on failure
 *	The vertical DPI of the display, or nil on failure
 */
static int
l_video_getDisplayDPI(lua_State *L)
{
	int display_index	= luaL_checkinteger(L, 1);
	float ddpi, hdpi, vdpi;

	if (SDL_GetDisplayDPI(display_index, &ddpi, &hdpi, &vdpi) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "ddd", ddpi, hdpi, vdpi);
}
#endif

/*
 * SDL.getDisplayMode(index, modeIndex)
 *
 * Arguments:
 *	index the display index
 *	modeIndex the mode index
 *
 * Returns:
 *	The display mode or nil on failure
 *	The error message
 */
static int
l_video_getDisplayMode(lua_State *L)
{
	int display_index	= luaL_checkinteger(L, 1);
	int mode_index		= luaL_checkinteger(L, 2);
	SDL_DisplayMode mode;

	if (SDL_GetDisplayMode(display_index, mode_index, &mode) < 0)
		return commonPushSDLError(L, 1);

	videoPushDisplayMode(L, &mode);

	return 1;
}

/*
 * SDL.getNumDisplayModes(index)
 *
 * Arguments:
 *	index the index
 *
 * Returns:
 * 	The number of display modes or nil on failure
 *	The error message
 */
static int
l_video_getNumDisplayModes(lua_State *L)
{
	int display_index = luaL_checkinteger(L, 1);
	int num;

	num = SDL_GetNumDisplayModes(display_index);
	if (num < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", num);
}

/*
 * SDL.getNumVideoDisplays()
 *
 * Returns:
 *	The number or nil on failure
 *	The error message
 */
static int
l_video_getNumVideoDisplays(lua_State *L)
{
	int ret = SDL_GetNumVideoDisplays();

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

/*
 * SDL.getNumVideoDrivers()
 *
 * Returns:
 *	The number or nil on failure
 *	The error message
 */
static int
l_video_getNumVideoDrivers(lua_State *L)
{
	int ret = SDL_GetNumVideoDrivers();

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

/*
 * SDL.getVideoDriver(index)
 *
 * Returns:
 *	The name
 */
static int
l_video_getVideoDriver(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);

	return commonPush(L, "s", SDL_GetVideoDriver(index));
}

/*
 * SDL.isScreenSaverLoaded()
 *
 * Returns:
 *	True if loaded
 */
static int
l_video_isScreenSaverLoaded(lua_State *L)
{
	return commonPush(L, "b", SDL_IsScreenSaverEnabled());
}

/*
 * SDL.videoInit(name)
 *
 * Arguments:
 *	name (optional) the driver name
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_video_init(lua_State *L)
{
	const char *dvname = NULL;

	if (lua_gettop(L) >= 1)
		dvname = luaL_checkstring(L, 1);

	if (SDL_VideoInit(dvname) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.videoQuit()
 */
static int
l_video_quit(lua_State *L)
{
	SDL_VideoQuit();

	(void)L;

	return 0;
}

const luaL_Reg DisplayFunctions[] = {
	{ "disableScreenSaver",			l_video_disableScreenSaver	},
	{ "enableScreenSaver",			l_video_enableScreenSaver	},
	{ "getClosestDisplayMode",		l_video_getClosestDisplayMode	},
	{ "getCurrentDisplayMode",		l_video_getCurrentDisplayMode	},
	{ "getCurrentVideoDriver",		l_video_getCurrentVideoDriver	},
	{ "getDesktopDisplayMode",		l_video_getDesktopDisplayMode	},
	{ "getDisplayBounds",			l_video_getDisplayBounds	},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "getDisplayUsableBounds",		l_video_getDisplayUsableBounds	},
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "getDisplayDPI",			l_video_getDisplayDPI		},
#endif
	{ "getDisplayMode",			l_video_getDisplayMode		},
	{ "getNumDisplayModes",			l_video_getNumDisplayModes	},
	{ "getNumVideoDisplays",		l_video_getNumVideoDisplays	},
	{ "getNumVideoDrivers",			l_video_getNumVideoDrivers	},
	{ "getVideoDriver",			l_video_getVideoDriver		},
	{ "isScreenSavedLoaded",		l_video_isScreenSaverLoaded	},
	{ "videoInit",				l_video_init			},
	{ "videoQuit",				l_video_quit			},
	{ NULL,					NULL				}
};

/*
 * SDL.pixelFormat
 */
const CommonEnum PixelFormat[] = {
	{ "Unknown",				SDL_PIXELFORMAT_UNKNOWN		},
	{ "Index1LSB",				SDL_PIXELFORMAT_INDEX1LSB	},
	{ "Index1MSB",				SDL_PIXELFORMAT_INDEX1MSB	},
	{ "Index4LSB",				SDL_PIXELFORMAT_INDEX4LSB	},
	{ "Index4MSB",				SDL_PIXELFORMAT_INDEX4MSB	},
	{ "Index8",				SDL_PIXELFORMAT_INDEX8		},
	{ "RGB332",				SDL_PIXELFORMAT_RGB332		},
	{ "RGB444",				SDL_PIXELFORMAT_RGB444		},
	{ "RGB555",				SDL_PIXELFORMAT_RGB555		},
	{ "BGR555",				SDL_PIXELFORMAT_BGR555		},
	{ "ARGB4444",				SDL_PIXELFORMAT_ARGB4444	},
	{ "RGBA4444",				SDL_PIXELFORMAT_RGBA4444	},
	{ "ABGR4444",				SDL_PIXELFORMAT_ABGR4444	},
	{ "BGRA4444",				SDL_PIXELFORMAT_BGRA4444	},
	{ "ARGB1555",				SDL_PIXELFORMAT_ARGB1555	},
	{ "RGBA5551",				SDL_PIXELFORMAT_RGBA5551	},
	{ "ABGR1555",				SDL_PIXELFORMAT_ABGR1555	},
	{ "BGRA5551",				SDL_PIXELFORMAT_BGRA5551	},
	{ "RGB565",				SDL_PIXELFORMAT_RGB565		},
	{ "BGR565",				SDL_PIXELFORMAT_BGR565		},
	{ "RGB24",				SDL_PIXELFORMAT_RGB24		},
	{ "BGR24",				SDL_PIXELFORMAT_BGR24		},
	{ "RGB888",				SDL_PIXELFORMAT_RGB888		},
	{ "RGBX8888",				SDL_PIXELFORMAT_RGBX8888	},
	{ "BGR888",				SDL_PIXELFORMAT_BGR888		},
	{ "BGRX8888",				SDL_PIXELFORMAT_BGRX8888	},
	{ "ARGB8888",				SDL_PIXELFORMAT_ARGB8888	},
	{ "RGBA8888",				SDL_PIXELFORMAT_RGBA8888	},
	{ "ABGR8888",				SDL_PIXELFORMAT_ABGR8888	},
	{ "BGRA8888",				SDL_PIXELFORMAT_BGRA8888	},
	{ "ARGB2101010",			SDL_PIXELFORMAT_ARGB2101010	},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "RGBA32",				SDL_PIXELFORMAT_RGBA32		},
	{ "ARGB32",				SDL_PIXELFORMAT_ARGB32		},
	{ "BGRA32",				SDL_PIXELFORMAT_BGRA32		},
	{ "ABGR32",				SDL_PIXELFORMAT_ABGR32		},
#endif
	{ "YV12",				SDL_PIXELFORMAT_YV12		},
	{ "IYUV",				SDL_PIXELFORMAT_IYUV		},
	{ "YUY2",				SDL_PIXELFORMAT_YUY2		},
	{ "UYVY",				SDL_PIXELFORMAT_UYVY		},
	{ "YVYU",				SDL_PIXELFORMAT_YVYU		},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "NV12",				SDL_PIXELFORMAT_NV12		},
	{ "NV21",				SDL_PIXELFORMAT_NV21		},
#endif
	{ NULL,					-1				}
};
