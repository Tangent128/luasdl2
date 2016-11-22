/*
 * timer.c -- timer routines
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

#include "timer.h"
#include "thread.h"

typedef struct Timer {
	SDL_TimerID	 id;
	lua_State	*L;
	int		 ref;
} Timer;

/*
 * Timer:remove()
 *
 * Returns:
 *	True if deleted
 */
static int
l_timer_remove(lua_State *L)
{
	Timer *t = commonGetAs(L, 1, TimerName, Timer *);
	int ret;

	ret = SDL_RemoveTimer(t->id);
	t->ref = -1;

	return commonPush(L, "b", ret);
}

/*
 * Timer:id()
 *
 * Returns:
 *	The timer id
 */
static int
l_timer_id(lua_State *L)
{
	Timer *t = commonGetAs(L, 1, TimerName, Timer *);

	return commonPush(L, "i", t->id);
}

/*
 * Timer:__gc()
 */
static int
l_timer_gc(lua_State *L)
{
	Timer *t = commonGetAs(L, 1, TimerName, Timer *);

	if (t->ref > 0)
		SDL_RemoveTimer(t->id);

	lua_close(t->L);

	return 0;
}

static const luaL_Reg TimerMethods[] = {
	{ "remove",			l_timer_remove			},
	{ "id",				l_timer_id			},
	{ NULL,				NULL				}
};

static const luaL_Reg TimerMetamethods[] = {
	{ "__gc",			l_timer_gc			},
	{ NULL,				NULL				}
};

const CommonObject TimerObject = {
	"Timer",
	TimerMethods,
	TimerMetamethods
};

/* ---------------------------------------------------------
 * Timer helpers
 * --------------------------------------------------------- */

static Uint32
timerCallback(Uint32 interval, Timer *t)
{
	Uint32 v = 0;

	lua_rawgeti(t->L, LUA_REGISTRYINDEX, t->ref);
	lua_pushinteger(t->L, interval);

	if (lua_pcall(t->L, 1, 1, 0) != LUA_OK) {
		SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", lua_tostring(t->L, -1));
		lua_pop(t->L, 1);
	} else if (lua_type(t->L, -1) == LUA_TNUMBER)
		v = (Uint32)lua_tonumber(t->L, -1);

	return v;
}

/* ---------------------------------------------------------
 * Timer functions
 * --------------------------------------------------------- */

/*
 * SDL.addTimer(interval, source)
 *
 * Arguments:
 *	interval the interval between calls
 *	source the path to the file or a function
 *
 * Returns:
 *	The timer object or nil on failure
 *	The error message
 */
static int
l_addTimer(lua_State *L)
{
	Uint32 interval = luaL_checkinteger(L, 1);
	Timer *t;

	if ((t = calloc(1, sizeof (Timer))) == NULL)
		return commonPushErrno(L, 1);

	t->L = luaL_newstate();
	luaL_openlibs(t->L);

	/* Dump the function or filepath */
	if (threadDump(L, t->L, 2) == 2)
		goto fail;

	/* Store the function into the ref */
	t->ref = luaL_ref(t->L, LUA_REGISTRYINDEX);

	/* Create the timer */
	if ((t->id = SDL_AddTimer(interval, (SDL_TimerCallback)timerCallback, t)) == 0) {
		commonPushSDLError(L, 1);
		goto fail;
	}

	return commonPush(L, "p", TimerName, t);

fail:
	if (t->L)
		lua_close(t->L);
	free(t);

	return 2;
}

/*
 * SDL.delay(count)
 *
 * Params:
 *	count, the number of milliseconds
 */
static int
l_delay(lua_State *L)
{
	SDL_Delay(luaL_checkinteger(L, 1));

	return 0;
}

/*
 * SDL.getPerformanceCounter()
 *
 * Returns:
 *	The number
 */
static int
l_getPerformanceCounter(lua_State *L)
{
	return commonPush(L, "i", SDL_GetPerformanceCounter());
}

/*
 * SDL.getPerformanceFrequency()
 *
 * Returns:
 *	The number
 */
static int
l_getPerformanceFrequency(lua_State *L)
{
	return commonPush(L, "i", SDL_GetPerformanceFrequency());
}

/*
 * SDL.getTicks()
 *
 * Returns:
 *	The ticks
 */
static int
l_getTicks(lua_State *L)
{
	return commonPush(L, "i", SDL_GetTicks());
}

const luaL_Reg TimerFunctions[] = {
	{ "addTimer",			l_addTimer			},
	{ "delay",			l_delay				},
	{ "getPerformanceCounter",	l_getPerformanceCounter		},
	{ "getPerformanceFrequency",	l_getPerformanceFrequency	},
	{ "getTicks",			l_getTicks			},
	{ NULL,				NULL				}
};
