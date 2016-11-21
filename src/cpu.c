/*
 * cpu.c -- CPU feature detection
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

#include "cpu.h"

/*
 * SDL.getCPUCacheLineSize()
 *
 * Returns:
 *	The value
 */
static int
l_cpu_getCacheLineSize(lua_State *L)
{
	return commonPush(L, "i",  SDL_GetCPUCacheLineSize());

}

/*
 * SDL.getCPUCount()
 *
 * Returns:
 *	The number of CPU
 */
static int
l_cpu_getCount(lua_State *L)
{
	return commonPush(L, "i", SDL_GetCPUCount());
}

/*
 * SDL.has3DNow()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_has3DNow(lua_State *L)
{
	return commonPush(L, "b", SDL_Has3DNow());
}

/*
 * SDL.hasAltiVec()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasAltiVec(lua_State *L)
{
	return commonPush(L, "b", SDL_HasAltiVec());
}

#if SDL_VERSION_ATLEAST(2, 0, 2)
/*
 * SDL.hasAVX()
 *
 * Returns:
 *	True if the CPU has support for AVX
 */
static int
l_cpu_hasAVX(lua_State *L)
{
	return commonPush(L, "b", SDL_HasAVX());
}
#endif

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.hasAVX2()
 *
 * Returns:
 *	True if the CPU has support for AVX2
 */
static int
l_cpu_hasAVX2(lua_State *L)
{
	return commonPush(L, "b", SDL_HasAVX2());
}
#endif

/*
 * SDL.hasMMX()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasMMX(lua_State *L)
{
	return commonPush(L, "b", SDL_HasMMX());
}

/*
 * SDL.hasRDTSC()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasRDTSC(lua_State *L)
{
	return commonPush(L, "b", SDL_HasRDTSC());
}

/*
 * SDL.hasSSE()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasSSE(lua_State *L)
{
	return commonPush(L, "b", SDL_HasSSE());
}

/*
 * SDL.hasSSE2()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasSSE2(lua_State *L)
{
	return commonPush(L, "b", SDL_HasSSE2());
}

/*
 * SDL.hasSSE41()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasSSE41(lua_State *L)
{
	return commonPush(L, "b", SDL_HasSSE41());
}

/*
 * SDL.hasSSE42()
 *
 * Returns:
 *	True if has
 */
static int
l_cpu_hasSSE42(lua_State *L)
{
	return commonPush(L, "b", SDL_HasSSE42());
}

const luaL_Reg CpuFunctions[] = {
	{ "getCPUCacheLineSize",		l_cpu_getCacheLineSize		},
	{ "getCPUCount",			l_cpu_getCount			},
	{ "has3DNow",				l_cpu_has3DNow			},
	{ "hasAltiVec",				l_cpu_hasAltiVec		},
#if SDL_VERSION_ATLEAST(2, 0, 2)
	{ "hasAVX",				l_cpu_hasAVX			},
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "hasAVX2",				l_cpu_hasAVX2			},
#endif
	{ "hasMMX",				l_cpu_hasMMX			},
	{ "hasRDTSC",				l_cpu_hasRDTSC			},
	{ "hasSSE",				l_cpu_hasSSE			},
	{ "hasSSE2",				l_cpu_hasSSE2			},
	{ "hasSSE41",				l_cpu_hasSSE41			},
	{ "hasSSE42",				l_cpu_hasSSE42			},
	{ NULL,					NULL				}
};
