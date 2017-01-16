/*
 * renderer.c -- 2D accelerated drawing
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
#include <common/table.h>
#include <common/video.h>

#include "renderer.h"
#include "texture.h"
#include "window.h"

void
pushRendererInfo(lua_State *L, const SDL_RendererInfo *info)
{
	int i;

	lua_createtable(L, 6, 6);

	lua_pushstring(L, info->name);
	lua_setfield(L, -2, "name");

	commonPushEnum(L, info->flags, RendererFlags);
	lua_setfield(L, -2, "flags");

	lua_pushinteger(L, info->num_texture_formats);
	lua_setfield(L, -2, "numTextureFormats");

	lua_pushinteger(L, info->max_texture_width);
	lua_setfield(L, -2, "maxTextureWidth");

	lua_pushinteger(L, info->max_texture_height);
	lua_setfield(L, -2, "maxTextureHeight");

	/* texture_formats as a table 1 .. 17 */
	lua_createtable(L, 0, 0);

	for (i = 0; i < 16; ++i) {
		lua_pushinteger(L, info->texture_formats[i]);
		lua_rawseti(L, -2, i + 1);
	}

	lua_setfield(L, -2, "textureFormats");
}

/* --------------------------------------------------------
 * Renderer functions
 * -------------------------------------------------------- */

/*
 * SDL.createRenderer(win, index, flags)
 *
 * Arguments:
 *	win the window
 *	index the index
 *	flags the flags
 *
 * Returns:
 *	The renderer object or nil on failure
 *	The error message
 */
static int
l_createRenderer(lua_State *L)
{
	SDL_Window *win	= commonGetAs(L, 1, WindowName, SDL_Window *);
	int index	= luaL_checkinteger(L, 2);
	int flags	= commonGetEnum(L, 3);

	SDL_Renderer *rd;

	rd = SDL_CreateRenderer(win, index, flags);
	if (rd == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", RendererName, rd);
}

/*
 * SDL.createSoftwareRenderer(surface)
 *
 * Arguments:
 *	surface the surface
 *
 * Returns:
 *	The renderer or nil on failure
 *	The error message
 */
static int
l_createSoftwareRenderer(lua_State *L)
{
	SDL_Surface *surf = commonGetAs(L, 1, SurfaceName, SDL_Surface *);
	SDL_Renderer *rd;

	rd = SDL_CreateSoftwareRenderer(surf);
	if (rd == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", RendererName, rd);
}

/*
 * SDL.getNumRenderDrivers()
 *
 * Returns:
 *	The number of render drivers
 */
static int
l_getNumRenderDrivers(lua_State *L)
{
	return commonPush(L, "i", SDL_GetNumRenderDrivers());
}

/*
 * SDL.getRenderDriverInfo(index)
 *
 * Arguments:
 *	index the index
 *
 * Returns:
 *	The renderer info or nil on failure
 *	The error message
 */
static int
l_getRenderDriverInfo(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_RendererInfo info;

	if (SDL_GetRenderDriverInfo(index, &info) < 0)
		return commonPushSDLError(L, 1);

	pushRendererInfo(L, &info);

	return 1;
}

const luaL_Reg RendererFunctions[] = {
	{ "createRenderer",		l_createRenderer		},
	{ "createSoftwareRenderer",	l_createSoftwareRenderer	},
	{ "getNumRenderDrivers",	l_getNumRenderDrivers		},
	{ "getRenderDriverInfo",	l_getRenderDriverInfo		},
	{ NULL,				NULL				}
};

/* --------------------------------------------------------
 * Renderer enumerations
 * -------------------------------------------------------- */

/*
 * SDL.rendererFlags
 */
const CommonEnum RendererFlags[] = {
	{ "Software",			SDL_RENDERER_SOFTWARE		},
	{ "Accelerated",		SDL_RENDERER_ACCELERATED	},
	{ "PresentVSYNC",		SDL_RENDERER_PRESENTVSYNC	},
	{ "TargetTexture",		SDL_RENDERER_TARGETTEXTURE	},
	{ NULL,				-1				}
};

/*
 * SDL.rendererFlip
 */
const CommonEnum RendererFlip[] = {
	{ "None",			SDL_FLIP_NONE			},
	{ "Horizontal",			SDL_FLIP_HORIZONTAL		},
	{ "Vertical",			SDL_FLIP_VERTICAL		},
	{ NULL,				-1				}
};

/* --------------------------------------------------------
 * Private renderer object helpers
 * -------------------------------------------------------- */

static int
rendererDrawOrFillRect(lua_State *L, int draw)
{
	typedef int (*UseFunc)(SDL_Renderer *, const SDL_Rect *);

	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Rect r;

	/* Draw or fill? */
	UseFunc func = (draw) ? SDL_RenderDrawRect : SDL_RenderFillRect;

	videoGetRect(L, 2, &r);

	if (func(rd, &r) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static int
rendererDrawOrFillRects(lua_State *L, int draw)
{
	typedef int (*UseFunc)(SDL_Renderer *, const SDL_Rect *, int);

	Array rects;
	int ret;

	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	UseFunc func = (draw) ? SDL_RenderDrawRects : SDL_RenderFillRects;

	if (videoGetRects(L, 2, &rects) < 0)
		return commonPushErrno(L, 1);

	ret = func(rd, rects.data, rects.length);
	arrayFree(&rects);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/* --------------------------------------------------------
 * Renderer object methods
 * -------------------------------------------------------- */

/*
 * Renderer:createTexture(format, access, w, h)
 *
 * Arguments:
 *	rd the renderer
 *	format the format (SDL.pixelFormat)
 *	access the access (SDL.textureAccess)
 *	w the width
 *	h the height
 *
 * Returns:
 *	The texture or nil on failure
 *	The error message
 */
static int
l_renderer_createTexture(lua_State *L)
{
	SDL_Renderer *rd	= commonGetAs(L, 1, RendererName, SDL_Renderer *);
	int format		= luaL_checkinteger(L, 2);
	int access		= luaL_checkinteger(L, 3);
	int w			= luaL_checkinteger(L, 4);
	int h			= luaL_checkinteger(L, 5);

	SDL_Texture *tex = SDL_CreateTexture(rd, format, access, w, h);
	if (tex == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", TextureName, tex);
}

/*
 * Renderer:createTextureFromSurface(surface)
 *
 * Arguments:
 *	rd the renderer
 *	surface the surface
 *
 * Returns:
 *	The texture object or nil on failure
 *	The error message
 */
static int
l_renderer_createTextureFromSuface(lua_State *L)
{
	SDL_Renderer *rd	= commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Surface *surf	= commonGetAs(L, 2, SurfaceName, SDL_Surface *);

	SDL_Texture *tex = SDL_CreateTextureFromSurface(rd, surf);
	if (tex == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", TextureName, tex);
}

/*
 * Renderer:clear()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_clear(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	if (SDL_RenderClear(rd) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:copy(texture, srcrect, dstrect)
 *
 * Arguments:
 *	texture the texture
 *	srcrect (optional) the source rectangle
 *	dstrect (optional) the destination rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_copy(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Texture *tex = commonGetAs(L, 2, TextureName, SDL_Texture *);
	SDL_Rect srcr, dstr;
	SDL_Rect *srcptr = NULL;
	SDL_Rect *dstptr = NULL;

	if (lua_gettop(L) >= 3 && lua_type(L, 3) == LUA_TTABLE) {
		videoGetRect(L, 3, &srcr);
		srcptr = &srcr;
	}
	if (lua_gettop(L) >= 4 && lua_type(L, 4) == LUA_TTABLE) {
		videoGetRect(L, 4, &dstr);
		dstptr = &dstr;
	}

	if (SDL_RenderCopy(rd, tex, srcptr, dstptr) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:copyEx(params)
 *
 * The table params may or must have the following fields:
 *	texture the texture
 *	source (optional) the source rectangle
 *	destination (optional) the destination rectangle
 *	angle (optional) the angle
 *	center (optional) the center point
 *	flip (optional) the flip (SDL.rendererFlip)
 *
 * Arguments:
 *	params the parameters
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_copyEx(lua_State *L)
{
#define GET_RECT(idx, field, rect, rectptr) do {			\
	lua_getfield(L, idx, field);					\
	if (lua_type(L, -1) == LUA_TTABLE) {				\
		videoGetRect(L, -1, &rect);				\
		rectptr = &rect;					\
	}								\
	lua_pop(L, 1);							\
} while (/* CONSTCOND */ 0)

	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Texture *tex;
	SDL_RendererFlip flip;
	SDL_Point point, *pointptr = NULL;
	SDL_Rect srcr, *srcptr = NULL;
	SDL_Rect dstr, *dstptr = NULL;
	double angle;

	luaL_checktype(L, 2, LUA_TTABLE);

	/* Texture is mandatory */
	tex = tableGetUserdata(L, 2, "texture", TextureName)->data;

	/* Optional SDL_Rect fields */
	GET_RECT(2, "source", srcr, srcptr);
	GET_RECT(2, "destination", dstr, dstptr);

	/* Optional angle */
	angle = tableGetDouble(L, 2, "angle");

	/* Optional point */
	lua_getfield(L, 2, "center");
	if (lua_type(L, -1) == LUA_TTABLE) {
		videoGetPoint(L, -1, &point);
		pointptr = &point;
	}
	lua_pop(L, 1);

	/* Optional flip */
	flip = tableGetInt(L, 2, "flip");

	if (SDL_RenderCopyEx(rd, tex, srcptr, dstptr, angle, pointptr, flip) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:drawLine(line)
 *
 * Arguments:
 *	line the line
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_drawLine(lua_State *L)
{
	SDL_Renderer *r	= commonGetAs(L, 1, RendererName, SDL_Renderer *);
	Line line;

	videoGetLine(L, 2, &line);

	if (SDL_RenderDrawLine(r, line.x1, line.y1, line.x2, line.y2) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:drawLines(points)
 *
 * Arguments:
 *	points a sequence of points connected by lines
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_drawLines(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	Array points;
	int ret;

	if (videoGetPoints(L, 2, &points) < 0)
		return commonPushErrno(L, 1);

	ret = SDL_RenderDrawLines(rd, points.data, points.length);
	arrayFree(&points);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:drawPoint(point)
 *
 * Arguments:
 *	point the point
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_drawPoint(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Point p;

	videoGetPoint(L, 2, &p);

	if (SDL_RenderDrawPoint(rd, p.x, p.y) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:drawPoints(points)
 *
 * Arguments:
 *	points a sequence of points
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_drawPoints(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	Array points;
	int ret;

	if (videoGetPoints(L, 2, &points) < 0)
		return commonPushErrno(L, 1);

	ret = SDL_RenderDrawPoints(rd, points.data, points.length);
	arrayFree(&points);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:drawRect(rect)
 *
 * Arguments:
 *	rect the rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_drawRect(lua_State *L)
{
	return rendererDrawOrFillRect(L, 1);
}

/*
 * Renderer:drawRects(rect)
 *
 * Arguments:
 *	rects a sequence of rectangles
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_drawRects(lua_State *L)
{
	return rendererDrawOrFillRects(L, 1);
}

/*
 * Renderer:fillRect(rect)
 *
 * Arguments:
 *	rect the rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_fillRect(lua_State *L)
{
	return rendererDrawOrFillRect(L, 0);
}

/*
 * Renderer:fillRects(rect)
 *
 * Arguments:
 *	rects a sequence of rectangles
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_fillRects(lua_State *L)
{
	return rendererDrawOrFillRects(L, 0);
}

/*
 * Renderer:getClipRect()
 *
 * Returns:
 *	The clip rectangle
 */
static int
l_renderer_getClipRect(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Rect rect;

	SDL_RenderGetClipRect(rd, &rect);
	videoPushRect(L, &rect);

	return 1;
}

/*
 * Renderer:getDrawBlendMode()
 *
 * Returns:
 *	The blend mode (SDL.blendMode)
 */
static int
l_renderer_getDrawBlendMode(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_BlendMode mode;

	SDL_GetRenderDrawBlendMode(rd, &mode);

	return commonPush(L, "i", mode);
}

/*
 * Renderer:getDrawColor()
 *
 * Returns:
 *	The color (hexadecimal)
 *	The color (table)
 *	The error message
 */
static int
l_renderer_getDrawColor(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Color c;
	Uint32 value;

	if (SDL_GetRenderDrawColor(rd, &c.r, &c.g, &c.b, &c.a) < 0)
		return commonPushSDLError(L, 2);

	value = (c.r << 16) | (c.g << 8) | c.b;

	commonPush(L, "i", value);
	videoPushColorRGB(L, &c);

	return 2;
}

#if SDL_VERSION_ATLEAST(2, 0, 5)
/*
 * Renderer:getIntegerScale(enabled)
 *
 * Returns:
 *	true if enabled, false otherwise
 */
static int
l_renderer_getIntegerScale(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	return commonPush(L, "b", SDL_RenderGetIntegerScale(rd));
}
#endif

/*
 * Renderer:getRendererInfo()
 *
 * Returns:
 *	The renderer info or nil on failure
 *	The error message
 */
static int
l_renderer_getInfo(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_RendererInfo info;

	if (SDL_GetRendererInfo(rd, &info) < 0)
		return commonPushSDLError(L, 1);

	pushRendererInfo(L, &info);

	return 1;
}

/*
 * Renderer:getViewport()
 *
 * Returns:
 *	The rectangle viewport
 */
static int
l_renderer_getViewport(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Rect rect;

	SDL_RenderGetViewport(rd, &rect);
	videoPushRect(L, &rect);

	return 1;
}

/*
 * Renderer:getLogicalSize()
 *
 * Returns:
 *	w width
 *	h height
 */
static int
l_renderer_getLogicalSize(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	int w, h;

	SDL_RenderGetLogicalSize(rd, &w, &h);
	commonPush(L, "i", w);
	commonPush(L, "i", h);

	return 2;
}

/*
 * Renderer:present()
 */
static int
l_renderer_present(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	SDL_RenderPresent(rd);

	return 0;
}

/*
 * Renderer:setClipRect(rect)
 *
 * Arguments:
 *	rect the clip rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_setClipRect(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Rect rect;

	videoGetRect(L, 2, &rect);

	if (SDL_RenderSetClipRect(rd, &rect) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * Renderer:isClipEnabled()
 *
 * Returns:
 *	True if enabled or false otherwise.
 */
static int
l_renderer_isClipEnabled(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	return commonPush(L, "b", SDL_RenderIsClipEnabled(rd));
}
#endif

/*
 * Renderer:setDrawBlendMode(mode)
 *
 * Arguments:
 *	mode the mode (SDL.blendMode)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_setDrawBlendMode(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_BlendMode mode;

	mode = luaL_checkinteger(L, 2);

	if (SDL_SetRenderDrawBlendMode(rd, mode) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:setDrawColor(color)
 *
 * Arguments:
 *	color the color
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_setDrawColor(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Color c;

	c = videoGetColorRGB(L, 2);

	if (SDL_SetRenderDrawColor(rd, c.r, c.g, c.b, c.a) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

#if SDL_VERSION_ATLEAST(2, 0, 5)
/*
 * Renderer:setIntegerScale(enabled)
 *
 * Arguments:
 *	enable enable integer scaling for this renderer
 *
 * Returns:
 *	true on success, nil otherwise
 *	the error message
 */
static int
l_renderer_setIntegerScale(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	int enabled = lua_toboolean(L, 2);

	if (SDL_RenderSetIntegerScale(rd, enabled) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}
#endif

/*
 * Renderer:setTarget(texture)
 *
 * Arguments:
 *	texture (optional) the texture
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_setTarget(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Texture *tex = NULL;

	/* The texture is optional */
	if (lua_type(L, 2) == LUA_TUSERDATA)
		tex = commonGetAs(L, 2, TextureName, SDL_Texture *);

	if (SDL_SetRenderTarget(rd, tex) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:setViewport(rect)
 *
 * Arguments:
 *	rect the rectangle
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_renderer_setViewport(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Rect rect;

	videoGetRect(L, 2, &rect);

	if (SDL_RenderSetViewport(rd, &rect) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Renderer:setLogicalSize(w, h)
 *
 * Arguments:
 *	w the width of the logical resolution
 *	h the height of the logical resolution
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message on failure
 */
static int
l_renderer_setLogicalSize(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	int w = luaL_checkinteger(L, 2);
	int h = luaL_checkinteger(L, 3);

	if (SDL_RenderSetLogicalSize(rd, w, h) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/* --------------------------------------------------------
 * Renderer object metamethods
 * -------------------------------------------------------- */

/*
 * Renderer:__eq()
 */
static int
l_renderer_eq(lua_State *L)
{
	SDL_Renderer *rd1 = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_Renderer *rd2 = commonGetAs(L, 1, RendererName, SDL_Renderer *);

	return commonPush(L, "b", rd1 == rd2);
}

/*
 * Renderer:__gc()
 */
static int
l_renderer_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, RendererName);

	if (udata->mustdelete)
		SDL_DestroyRenderer(udata->data);

	return 0;
}

/*
 * Renderer:__tostring()
 */
static int
l_renderer_tostring(lua_State *L)
{
	SDL_Renderer *rd = commonGetAs(L, 1, RendererName, SDL_Renderer *);
	SDL_RendererInfo info;

	/* What to print? */
	if (SDL_GetRendererInfo(rd, &info) < 0)
		return commonPushSDLError(L, 0);

	lua_pushfstring(L,
	    "renderer %s: flags %d, ntexfmt %d, maxw %d, maxh %d",
	    info.name, info.flags, info.num_texture_formats,
	    info.max_texture_width, info.max_texture_height
	);

	return 1;
}

/* --------------------------------------------------------
 * Renderer object definition
 * -------------------------------------------------------- */

static const luaL_Reg RendererMethods[] = {
	{ "createTexture",		l_renderer_createTexture		},
	{ "createTextureFromSurface",	l_renderer_createTextureFromSuface	},
	{ "clear",			l_renderer_clear			},
	{ "copy",			l_renderer_copy				},
	{ "copyEx",			l_renderer_copyEx			},
	{ "drawLine",			l_renderer_drawLine			},
	{ "drawLines",			l_renderer_drawLines			},
	{ "drawPoint",			l_renderer_drawPoint			},
	{ "drawPoints",			l_renderer_drawPoints			},
	{ "drawRect",			l_renderer_drawRect			},
	{ "drawRects",			l_renderer_drawRects			},
	{ "fillRect",			l_renderer_fillRect			},
	{ "fillRects",			l_renderer_fillRects			},
	{ "getClipRect",		l_renderer_getClipRect			},
	{ "getDrawBlendMode",		l_renderer_getDrawBlendMode		},
	{ "getDrawColor",		l_renderer_getDrawColor			},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "getIntegerScale",		l_renderer_getIntegerScale		},
#endif
	{ "getInfo",			l_renderer_getInfo			},
	{ "getViewport",		l_renderer_getViewport			},
	{ "getLogicalSize",		l_renderer_getLogicalSize		},
	{ "present",			l_renderer_present			},
	{ "setClipRect",		l_renderer_setClipRect			},
	{ "setDrawBlendMode",		l_renderer_setDrawBlendMode		},
	{ "setDrawColor",		l_renderer_setDrawColor			},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "setIntegerScale",		l_renderer_setIntegerScale		},
#endif
	{ "setTarget",			l_renderer_setTarget			},
	{ "setViewport",		l_renderer_setViewport			},
	{ "setLogicalSize",		l_renderer_setLogicalSize		},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "isClipEnabled",		l_renderer_isClipEnabled		},
#endif
	{ NULL,				NULL					}
};

static const luaL_Reg RendererMetamethods[] = {
	{ "__eq",			l_renderer_eq			},
	{ "__gc",			l_renderer_gc			},
	{ "__tostring",			l_renderer_tostring		},
	{ NULL,				NULL				}
};

const CommonObject Renderer = {
	"Renderer",
	RendererMethods,
	RendererMetamethods
};
