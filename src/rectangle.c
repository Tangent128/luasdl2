/*
 * rectangle.c -- rectangle, overlap and merges
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/video.h>

#include "rectangle.h"

/*
 * SDL.enclosePoints(points, clip)
 *
 * Arguments:
 *	points a sequence table of points
 *	clip (optional) a clipping rectangle
 *
 * Returns:
 *	True on success or false on failure
 *	The rectangle or nil
 *	The error message
 */
static int
l_enclosePoints(lua_State *L)
{
	SDL_Rect result, clip, *clipptr = NULL;

	int ret;
	Array points;

	/* Get / check arguments */
	luaL_checktype(L, 1, LUA_TTABLE);

	if (lua_gettop(L) >= 2) {
		videoGetRect(L, 2, &clip);
		clipptr = &clip;
	}

	if (videoGetPoints(L, 1, &points) < 0)
		return commonPushErrno(L, 2);

	ret = SDL_EnclosePoints(points.data, points.length, clipptr, &result);

	lua_pushboolean(L, ret);
	videoPushRect(L, &result);

	/* Get rid of points */
	arrayFree(&points);

	return 2;
}

/*
 * SDL.hasIntersection(r1, r2)
 *
 * Arguments:
 *	r1 the first rectangle
 *	r2 the second rectangle
 *
 * Returns:
 *	True on intersections
 */
static int
l_hasIntersection(lua_State *L)
{
	SDL_Rect a, b;

	videoGetRect(L, 1, &a);
	videoGetRect(L, 2, &b);

	return commonPush(L, "b", SDL_HasIntersection(&a, &b));
}

/*
 * SDL.intersectRect(r1, r2)
 *
 * Arguments:
 *	r1 the first rectangle
 *	r2 the second rectangle
 *
 * Returns:
 *	True if has intersection
 *	The result rectangle
 */
static int
l_intersectRect(lua_State *L)
{
	SDL_Rect a, b, result;
	int rvalue;

	videoGetRect(L, 1, &a);
	videoGetRect(L, 2, &b);

	rvalue = SDL_IntersectRect(&a, &b, &result);

	lua_pushboolean(L, rvalue);
	videoPushRect(L, &result);

	return 2;
}

/*
 * SDL.intersectRectAndLine(rect, x1, y1, x2, y2)
 *
 * Arguments:
 *	rect the rectangle
 *	x1 the starting x
 *	y1 the starting y
 *	x2 the ending x
 *	y2 the ending y
 *
 * Returns:
 *	The rectangle
 *	The updated x1,
 *	The updated y1,
 *	The updated x2,
 *	The updated y2
 */
static int
l_intersectRectAndLine(lua_State *L)
{
	SDL_Rect rect;
	int x1, y1, x2, y2, rvalue;

	videoGetRect(L, 1, &rect);
	x1 = luaL_checkinteger(L, 2);
	y1 = luaL_checkinteger(L, 3);
	x2 = luaL_checkinteger(L, 4);
	y2 = luaL_checkinteger(L, 5);

	rvalue = SDL_IntersectRectAndLine(&rect, &x1, &y1, &x2, &y2);

	return commonPush(L, "biiii", rvalue, x1, y1, x2, y2);
}

/*
 * SDL.rectEmpty(rect)
 *
 * Arguments:
 *	rect the rectangle
 *
 * Returns:
 *	True if empty
 */
static int
l_rectEmpty(lua_State *L)
{
	SDL_Rect rect;

	videoGetRect(L, 1, &rect);

	return commonPush(L, "b", SDL_RectEmpty(&rect));
}

/*
 * SDL.rectEquals(r1, r2)
 *
 * Arguments:
 *	r1 the first rectangle
 *	r2 the second rectangle
 *
 * Returns:
 *	True if equals
 */
static int
l_rectEquals(lua_State *L)
{
	SDL_Rect a, b;

	videoGetRect(L, 1, &a);
	videoGetRect(L, 2, &a);

	return commonPush(L, "b", SDL_RectEquals(&a, &b));
}

/*
 * SDL.unionRect(r1, r2)
 *
 * Arguments:
 *	r1 the first rectangle
 *	r2 the second rectangle
 *
 * Returns:
 *	The union rectangle
 */
static int
l_unionRect(lua_State *L)
{
	SDL_Rect a, b, result;

	videoGetRect(L, 1, &a);
	videoGetRect(L, 2, &a);

	SDL_UnionRect(&a, &b, &result);
	videoPushRect(L, &result);

	return 1;
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.pointInRect(p, r)
 *
 * Arguments:
 *	p the point
 *	r the rectangle
 *
 * Returns:
 *	true if p lies within r
 */
static int
l_pointInRect(lua_State *L)
{
	SDL_Point p;
	SDL_Rect r;

	videoGetPoint(L, 1, &p);
	videoGetRect(L, 2, &r);

	return commonPush(L, "b", SDL_PointInRect(&p, &r));
}
#endif

const luaL_Reg RectangleFunctions[] = {
	{ "enclosePoints",		l_enclosePoints		},
	{ "hasIntersection",		l_hasIntersection	},
	{ "intersectRect",		l_intersectRect		},
	{ "intersectRectAndLine",	l_intersectRectAndLine	},
	{ "rectEmpty",			l_rectEmpty		},
	{ "rectEquals",			l_rectEquals		},
	{ "unionRect",			l_unionRect		},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "pointInRect",		l_pointInRect		},
#endif
	{ NULL,				NULL			}
};
