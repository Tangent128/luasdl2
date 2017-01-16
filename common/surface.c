/*
 * surface.c -- legacy surfaces support
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

#include "rwops.h"
#include "surface.h"
#include "video.h"

/* --------------------------------------------------------
 * Surface functions
 * -------------------------------------------------------- */

/*
 * SDL.createRGBSurface(width,
 * 			height,
 * 			depth = 32,
 *			rmask = endian-dependant,
 *			gmask = endian-dependant,
 * 			bmask = endian-dependant,
 * 			amask = endian-dependant)
 *
 * Create a surface object and return it.
 *
 * Returns:
 *	The surface or nil
 *	The error message
 */
static int l_surface_createRGB(lua_State *L)
{
	int width	= luaL_checkinteger(L, 1);
	int height	= luaL_checkinteger(L, 2);
	int depth	= 32;
	Uint32 rmask, gmask, bmask, amask;
	SDL_Surface *s;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	if (lua_gettop(L) >= 3)
		depth = luaL_checkinteger(L, 3);
	if (lua_gettop(L) >= 4)
		rmask = luaL_checkinteger(L, 4);
	if (lua_gettop(L) >= 5)
		gmask = luaL_checkinteger(L, 5);
	if (lua_gettop(L) >= 6)
		bmask = luaL_checkinteger(L, 6);
	if (lua_gettop(L) >= 7)
		amask = luaL_checkinteger(L, 7);

	s = SDL_CreateRGBSurface(0, width, height, depth, rmask, gmask, bmask, amask);
	if (s == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, s);
}

#if 0

/*
 * SDL.createRGBSurfaceFrom(pixels,
 * 			    width,
 * 			    height,
 * 			    depth,
 * 			    pitch,
 * 			    rmask,
 * 			    gmask,
 * 			    bmask,
 * 			    amask)
 *
 * Create a surface object and return it.
 *
 * Returns:
 *	The surface or nil
 *	The error message
 */
static int l_surface_createRGBFrom(lua_State *L)
{
	char *pixels	= strdup(luaL_checkstring(L, 1));
	int width	= luaL_checkinteger(L, 2);
	int height	= luaL_checkinteger(L, 3);
	int depth	= luaL_checkinteger(L, 4);
	int pitch	= luaL_checkinteger(L, 5);
	int rmask	= luaL_checkinteger(L, 6);
	int gmask	= luaL_checkinteger(L, 7);
	int bmask	= luaL_checkinteger(L, 8);
	int amask	= luaL_checkinteger(L, 9);

	/*
	 * TODO: we need to store the pixels into the metatable and free
	 * them after the surface.
	 */

	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixels, width, height, depth,
	    pitch, rmask, gmask, bmask, amask);

	if (s == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, s);
}

#endif

#if SDL_VERSION_ATLEAST(2, 0, 5)
/*
 * SDL.createRGBSurfaceWithFormat(
 *	width,
 * 	height,
 * 	depth = 32,
 *	format = SDL.pixelFormat.RGBA32)
 *
 * Create a surface object with the specified format and return it.
 *
 * Returns:
 *	The surface or nil
 *	The error message
 */
static int l_surface_createRGBWithFormat(lua_State *L)
{
	int width	= luaL_checkinteger(L, 1);
	int height	= luaL_checkinteger(L, 2);
	int depth	= luaL_optinteger(L, 3, 32);
	int format	= luaL_optinteger(L, 4, SDL_PIXELFORMAT_RGBA32);
	SDL_Surface *s;

	s = SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
	if (s == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, s);
}
#endif

/*
 * SDL.loadBMP(path)
 *
 * Load a BMP surface and return it
 *
 * Params:
 * 	path the path to the file
 *
 * Returns:
 *	The surface or nil
 *	The error message
 */
static int l_surface_loadBMP(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	SDL_Surface *s;

	s = SDL_LoadBMP(path);
	if (s == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, s);
}

/*
 * SDL.loadBMP_RW(rwops)
 *
 * Load a BMP image by RW operations, then return the surface.
 *
 * Params:
 *	rwops the RWOps object created with SDL.RWCreate
 *
 * Returns:
 *	The surface or nil
 *	The error message
 */
static int l_surface_loadBMP_RW(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	SDL_Surface *s;

	s = SDL_LoadBMP_RW(ops, 0);
	if (s == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, s);
}

const luaL_Reg SurfaceFunctions[] = {
	{ "createRGBSurface",			l_surface_createRGB		},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "createRGBSurfaceWithFormat",		l_surface_createRGBWithFormat	},
#endif
	{ "loadBMP",				l_surface_loadBMP		},
	{ "loadBMP_RW",				l_surface_loadBMP_RW		},
#if 0
	{ "createRGBSurfaceFrom",		l_surface_createRGBFrom		},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "createRGBSurfaceFromWithFormat",
		l_surface_createRGBFromWithFormat				},
#endif
#endif
	{ NULL,				NULL			}
};

/* --------------------------------------------------------
 * Surface helpers
 * -------------------------------------------------------- */

static int
surfaceBlit(lua_State *L, int scaled, int lower)
{
	typedef int (*BlitFunc)(SDL_Surface *, const SDL_Rect *, SDL_Surface *, SDL_Rect *);
	typedef int (*LowerBlitFunc)(SDL_Surface *, SDL_Rect *, SDL_Surface *, SDL_Rect *);

	SDL_Surface *src = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Surface *dst = commonGetAs(L, 2, SurfaceName, SDL_Surface *);
	SDL_Rect srcrect, dstrect;
	SDL_Rect *srcptr = &srcrect, *dstptr = &dstrect;

	if (lua_type(L, 3) == LUA_TTABLE)
		videoGetRect(L, 3, &srcrect);
	else
		SDL_GetClipRect(src, &srcrect);

	if (lua_type(L, 4) == LUA_TTABLE)
		videoGetRect(L, 4, &dstrect);
	else
		SDL_GetClipRect(dst, &dstrect);

	if (!lower) {
		BlitFunc func = (scaled) ? SDL_BlitScaled : SDL_BlitSurface ;

		if (func(src, srcptr, dst, dstptr) < 0)
			return commonPushSDLError(L, 2);
	} else {
		LowerBlitFunc func = (scaled) ? SDL_LowerBlitScaled : SDL_LowerBlit;

		if (func(src, srcptr, dst, dstptr) < 0)
			return commonPushSDLError(L, 2);
	}

	/* Push true + the modified rectangle */
	lua_pushboolean(L, 1);
	videoPushRect(L, &dstrect);

	return 2;
}

/* --------------------------------------------------------
 * Surface methods
 * -------------------------------------------------------- */

/*
 * Surface:blit(surface, srcrect, dstrect)
 *
 * Params:
 *	surface the destination surface
 *	srcrect the source clip rectangle (nil for all)
 *	dstrect the destination rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_blit(lua_State *L)
{
	return surfaceBlit(L, 0, 0);
}

/*
 * Surface:blitScaled(surface, srcrect, dstrect)
 *
 * Params:
 *	surface the destination surface
 *	srcrect the source clip rectangle (nil for all)
 *	dstrect the destination rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_blitScaled(lua_State *L)
{
	return surfaceBlit(L, 1, 0);
}

static int
l_surface_convert(lua_State *L)
{
	/* XXX: IMPLEMENT */
	(void)L;

	return 0;
}

/*
 * Surface:convertFormat(format)
 *
 * Params:
 *	format the format
 *
 * Returns:
 *	The new surface or nil
 *	The error message
 */
static int
l_surface_convertFormat(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	int format		= luaL_checkinteger(L, 2);
	SDL_Surface *ret;

	ret = SDL_ConvertSurfaceFormat(surf, format, 0);
	if (ret == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, ret);
}

/*
 * Surface:fillRect(rect = nil, color = black)
 *
 * Params:
 *	rect the rectangle to fill
 *	color the color
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_fillRect(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Rect rect, *rectptr = NULL;
	Uint32 color = 0;

	if (lua_type(L, 2) != LUA_TNIL) {
		videoGetRect(L, 2, &rect);
		rectptr = &rect;
	}

	/* Default color is black */
	if (lua_gettop(L) >= 3)
		color = videoGetColorHex(L, 3);

	if (SDL_FillRect(surf, rectptr, color) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:fillRects(rects, color)
 *
 * Params:
 *	rects the sequence of rects
 *	color the color
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_fillRects(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	Uint32 color		= videoGetColorHex(L, 3);
	Array rects;
	int ret;

	/* Get / check arguments */
	luaL_checktype(L, 2, LUA_TTABLE);

	if (videoGetRects(L, 2, &rects) < 0)
		return commonPushErrno(L, 1);

	ret = SDL_FillRects(surf, rects.data, rects.length, color);
	arrayFree(&rects);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", ret);
}

/*
 * Surface:mapRGB()
 *
 * Returns:
 * 	A Uint32 filled with the correct pixel value
 */
static int
l_surface_mapRGB(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Color c = videoGetColorRGB(L, 2);

	lua_pushinteger(L, SDL_MapRGB(surf->format, c.r, c.g, c.b));

	return 1;
}

/*
 * Surface:mapRGBA()
 *
 * Returns:
 * 	A Uint32 filled with the correct pixel value.
 */
static int
l_surface_mapRGBA(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Color c = videoGetColorRGB(L, 2);

	lua_pushinteger(L, SDL_MapRGBA(surf->format, c.r, c.g, c.b, c.a));

	return 1;
}

/*
 * Surface:getClipRect()
 *
 * Returns:
 *	The clip rect
 */
static int
l_surface_getClipRect(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Rect rect;

	SDL_GetClipRect(surf, &rect);
	videoPushRect(L, &rect);

	return 1;
}

/*
 * Surface:getColorKey()
 *
 * Returns:
 *	The color key (number) or nil
 *	The color key (table) or nil
 *	The error message
 */
static int
l_surface_getColorKey(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Color c;
	Uint32 value;

	if (SDL_GetColorKey(surf, &value) < 0)
		return commonPushSDLError(L, 1);

	c.r = ((value >> 16) & 0xFF);
	c.g = ((value >> 8) & 0xFF);
	c.b = ((value) & 0xFF);

	commonPush(L, "i", value);
	videoPushColorRGB(L, &c);

	return 2;
}

/*
 * Surface:getAlphaMod()
 *
 * Returns:
 *	The alpha mod value or nil
 *	The error message
 */
static int
l_surface_getAlphaMod(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	Uint8 value;

	if (SDL_GetSurfaceAlphaMod(surf, &value) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", value);
}

/*
 * Surface:getBlendMode()
 *
 * Returns:
 *	The blend mode or nil
 *	The error message
 */
static int
l_surface_getBlendMode(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_BlendMode value;

	if (SDL_GetSurfaceBlendMode(surf, &value) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", value);
}

/*
 * Surface:getColorMod()
 *
 * Returns:
 *	The color mod or nil
 *	The error message
 */
static int
l_surface_getColorMod(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Color c;
	Uint32 value;

	if (SDL_GetSurfaceColorMod(surf, &c.r, &c.g, &c.b) < 0)
		return commonPushSDLError(L, 2);

	value = (c.r << 16) | (c.g << 8) | c.b;

	commonPush(L, "i", value);
	videoPushColorRGB(L, &c);

	return 2;
}

/*
 * Surface:lock()
 *
 * Returns:
 *	True on success or nil
 *	The error message
 */
static int
l_surface_lock(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);

	if (SDL_LockSurface(surf) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:lowerBlit(surface, srcrect, dstrect)
 *
 * Params:
 *	surface the destination surface
 *	srcrect the source clip rectangle (nil for all)
 *	dstrect the destination rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_lowerBlit(lua_State *L)
{
	return surfaceBlit(L, 0, 1);
}

/*
 * Surface:lowerBlitScaled(surface, srcrect, dstrect)
 *
 * Params:
 *	surface the destination surface
 *	srcrect the source clip rectangle (nil for all)
 *	dstrect the destination rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_lowerBlitScaled(lua_State *L)
{
	return surfaceBlit(L, 1, 1);
}

/*
 * Surface:mustLock()
 *
 * Returns
 *	True or false
 */
static int
l_surface_mustLock(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);

	return commonPush(L, "b", SDL_MUSTLOCK(surf));
}

/*
 * Surface:saveBMP(path)
 *
 * Params:
 *	path the path
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_saveBMP(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	const char *path	= luaL_checkstring(L, 2);

	if (SDL_SaveBMP(surf, path) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:saveBMP_RW(ops)
 *
 * Params:
 *	ops the rwops
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_saveBMP_RW(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_RWops *ops		= commonGetAs(L, 2, RWOpsName, SDL_RWops *);

	if (SDL_SaveBMP_RW(surf, ops, 0) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:setClipRect(rect)
 *
 * Params:
 *	rect the clip rectangle
 *
 * Returns:
 *	True or false
 */
static int
l_surface_setClipRect(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Rect rect;

	videoGetRect(L, 2, &rect);

	return commonPush(L, "b", SDL_SetClipRect(surf, &rect));
}

/*
 * Surface:setColorKey(flag, key)
 *
 * Params:
 *	flag the flag
 *	key the key color
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_setColorKey(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	int flag		= lua_toboolean(L, 2);
	int key			= luaL_checkinteger(L, 3);

	if (SDL_SetColorKey(surf, flag, key) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:setAlphaMod(alpha)
 *
 * Params:
 *	alpha the alpha mod
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_setAlphaMod(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	int alpha		= luaL_checkinteger(L, 2);

	if (SDL_SetSurfaceAlphaMod(surf, alpha) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:setBlendMod(mode)
 *
 * Params:
 *	mode the blend mode
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_setBlendMode(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_BlendMode mode	= luaL_checkinteger(L, 2);

	if (SDL_SetSurfaceBlendMode(surf, mode) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:setColorMod(color)
 *
 * Params:
 *	color the color mod
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_setColorMod(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Color color;

	color = videoGetColorRGB(L, 2);

	if (SDL_SetSurfaceColorMod(surf, color.r, color.g, color.b) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:setPalette(colors)
 *
 * Arguments:
 *	colors the colors
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_setPalette(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Palette palette;
	Array colors;
	int ret;

	if (videoGetColorsRGB(L, 2, &colors) < 0)
		return commonPushSDLError(L, 1);

	palette.ncolors	= colors.length;
	palette.colors	= colors.data;

	ret = SDL_SetSurfacePalette(surf, &palette);
	arrayFree(&colors);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:setRLE(flag)
 *
 * Params:
 *	flag the flag
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_surface_setRLE(lua_State *L)
{
	SDL_Surface *surf	= commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	int flag		= lua_toboolean(L, 2);

	if (SDL_SetSurfaceRLE(surf, flag) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Surface:unlock()
 */
static int
l_surface_unlock(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);

	SDL_UnlockSurface(surf);

	return 0;
}

/*
 * Surface:__gc()
 */
static int
l_surface_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, SurfaceName);

	if (udata->mustdelete)
		SDL_FreeSurface(udata->data);

	return 0;
}



/*
 * Surface:getSize()
 *
 * Returns:
 *	The width
 *	The height
 */
static int
l_surface_getSize(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);

	return commonPush(L, "ii", surf->w, surf->h);
}


/*
 * Surface:getRawPixel(x,y)
 *
 * Returns:
 *	(raw) byte string of pixel data (pixel format dependent)
 */
static int
l_surface_getRawPixel(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int size = surf->format->BytesPerPixel;
	Uint8 *ptr = (Uint8 *)surf->pixels + y * surf->pitch + x * size;
	lua_pushlstring(L, ptr, size);
	return 1;
}

static const luaL_Reg methods[] = {
	{ "blit",		l_surface_blit			},
	{ "blitScaled",		l_surface_blitScaled		},
	{ "convert",		l_surface_convert		},
	{ "convertFormat",	l_surface_convertFormat		},
	{ "fillRect",		l_surface_fillRect		},
	{ "fillRects",		l_surface_fillRects		},
	{ "mapRGB", 		l_surface_mapRGB		},
	{ "mapRGBA",		l_surface_mapRGBA		},
	{ "getClipRect",	l_surface_getClipRect,		},
	{ "getColorKey",	l_surface_getColorKey		},
	{ "getAlphaMod",	l_surface_getAlphaMod		},
	{ "getBlendMode",	l_surface_getBlendMode		},
	{ "getColorMod",	l_surface_getColorMod		},
	{ "getSize",		l_surface_getSize		},
	{ "getRawPixel",        l_surface_getRawPixel		},
	{ "lock",		l_surface_lock			},
	{ "lowerBlit",		l_surface_lowerBlit		},
	{ "lowerBlitScaled",	l_surface_lowerBlitScaled	},
	{ "mustLock",		l_surface_mustLock		},
	{ "saveBMP",		l_surface_saveBMP		},
	{ "saveBMP_RW",		l_surface_saveBMP_RW		},
	{ "setClipRect",	l_surface_setClipRect		},
	{ "setColorKey",	l_surface_setColorKey		},
	{ "setAlphaMod",	l_surface_setAlphaMod		},
	{ "setBlendMode",	l_surface_setBlendMode		},
	{ "setColorMod",	l_surface_setColorMod		},
	{ "setPalette",		l_surface_setPalette		},
	{ "setRLE",		l_surface_setRLE		},
	{ "unlock",		l_surface_unlock		},
	{ NULL,			NULL				}
};

static const luaL_Reg metamethods[] = {
	{ "__gc",		l_surface_gc			},
	{ NULL,			NULL				}
};

const CommonEnum BlendMode[] = {
	{ "None",		SDL_BLENDMODE_NONE		},
	{ "Blend",		SDL_BLENDMODE_BLEND		},
	{ "Add",		SDL_BLENDMODE_ADD		},
	{ "Mod",		SDL_BLENDMODE_MOD		},
	{ NULL,			-1				}
};

const CommonObject Surface = {
	"Surface",
	methods,
	metamethods
};
