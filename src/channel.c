/*
 * channel.c -- channel management (thread communication)
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

#include <common/variant.h>

#include "channel.h"

/* --------------------------------------------------------
 * Channel private functions
 * -------------------------------------------------------- */

typedef struct channel {
	char			*name;
	VariantQueue		 queue;
	SDL_atomic_t		 ref;
	SDL_mutex		*mutex;
	SDL_cond		*cond;
	unsigned int		 sent;
	unsigned int		 received;

	STAILQ_ENTRY(channel) link;
} Channel;

typedef STAILQ_HEAD(channel_list, channel) ChannelList;

/*
 * Highly based on LÖVE system.
 */
static int
channelGiven(unsigned int target, unsigned int current)
{
	union cv {
		unsigned long u;
		long i;
	} t, c;
	
	if (target > current)
		return 0;
	if (target == current)
		return 1;

	t.u = target;
	c.u = current;

	return !(t.i < 0 && c.i > 0);
}

static const Variant *
channelFirst(const Channel *c)
{
	Variant *v;

	SDL_LockMutex(c->mutex);
	if (STAILQ_EMPTY(&c->queue))
		return NULL;

	v = STAILQ_FIRST(&c->queue);
	SDL_UnlockMutex(c->mutex);

	return v;
}

static const Variant *
channelLast(const Channel *c)
{
	Variant *v;

	SDL_LockMutex(c->mutex);
	if (STAILQ_EMPTY(&c->queue))
		return NULL;

	v = STAILQ_LAST(&c->queue, variant, link);
	SDL_UnlockMutex(c->mutex);

	return v;
}

static int
channelPush(Channel *c, Variant *v)
{
	SDL_LockMutex(c->mutex);

	STAILQ_INSERT_TAIL(&c->queue, v, link);

	SDL_UnlockMutex(c->mutex);
	SDL_CondBroadcast(c->cond);

	return ++c->sent;
}

static const Variant *
channelWait(Channel *c)
{
	SDL_LockMutex(c->mutex);
	while (STAILQ_EMPTY(&c->queue))
		SDL_CondWait(c->cond, c->mutex);

	++ c->received;

	SDL_UnlockMutex(c->mutex);
	SDL_CondBroadcast(c->cond);

	return STAILQ_FIRST(&c->queue);
}

static void
channelSupply(Channel *c, Variant *v)
{
	unsigned id;

	SDL_LockMutex(c->mutex);

	id = channelPush(c, v);
	while (!channelGiven(id, c->received))
		SDL_CondWait(c->cond, c->mutex);
}

static void
channelClear(Channel *c)
{
	Variant *v, *tmp;

	SDL_LockMutex(c->mutex);
	STAILQ_FOREACH_SAFE(v, &c->queue, link, tmp)
		variantFree(v);
	SDL_UnlockMutex(c->mutex);

	SDL_CondBroadcast(c->cond);
}

static void
channelPop(Channel *c)
{
	SDL_LockMutex(c->mutex);
	STAILQ_REMOVE_HEAD(&c->queue, link);
	SDL_UnlockMutex(c->mutex);

	SDL_CondBroadcast(c->cond);
}

static void
channelFree(Channel *c)
{
	channelClear(c);
	free(c);
}

/* --------------------------------------------------------
 * LuaChannel functions
 * -------------------------------------------------------- */

static ChannelList	 g_channels = STAILQ_HEAD_INITIALIZER(g_channels);

/* Mutex initialized at SDL loading */
SDL_mutex		*ChannelMutex = NULL;

/*
 * SDL.getChannel(name)
 *
 * Arguments:
 *	name the channel name
 *
 * Returns:
 *	The channel object or nil on failure
 *	The error message
 */
static int
l_channel_get(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	Channel *c;
	int found = 0;

	SDL_LockMutex(ChannelMutex);
	STAILQ_FOREACH(c, &g_channels, link) {
		if (strcmp(c->name, name) == 0) {
			found = 1;
			break;
		}
	}

	if (!found) {
		if ((c = calloc(1, sizeof (Channel))) == NULL)
			goto fail;
		if ((c->name = strdup(name)) == NULL)
			goto fail;
		if ((c->mutex = SDL_CreateMutex()) == NULL)
			goto fail;
		if ((c->cond = SDL_CreateCond()) == NULL)
			goto fail;

		STAILQ_INIT(&c->queue);
		STAILQ_INSERT_TAIL(&g_channels, c, link);
	} else
		SDL_AtomicIncRef(&c->ref);

	SDL_UnlockMutex(ChannelMutex);

	return commonPush(L, "p", ChannelName, c);

fail:
	if (c->mutex)
		SDL_DestroyMutex(c->mutex);
	if (c->cond)
		SDL_DestroyCond(c->cond);
	free(c->name);
	free(c);

	SDL_UnlockMutex(ChannelMutex);

	return 2;
}

const luaL_Reg ChannelFunctions[] = {
	{ "getChannel",	l_channel_get		},
	{ NULL,		NULL			}
};

/* --------------------------------------------------------
 * LuaChannel methods
 * -------------------------------------------------------- */

/*
 * Channel:first()
 *
 * Returns:
 *	The first value or nil
 */
static int
l_channel_first(lua_State *L)
{
	Channel *c = commonGetAs(L, 1, ChannelName, Channel *);
	const Variant *v;

	if ((v = channelFirst(c)) == NULL)
		lua_pushnil(L);

	variantPush(L, v);

	return 1;
}

/*
 * Channel:last()
 *
 * Returns:
 *	The last value or nil
 */
static int
l_channel_last(lua_State *L)
{
	Channel *c = commonGetAs(L, 1, ChannelName, Channel *);
	const Variant *v;

	if ((v = channelLast(c)) == NULL)
		lua_pushnil(L);

	variantPush(L, v);

	return 1;
}

/*
 * Channel:push(value)
 *
 * Arguments:
 *	value the value to push (!userdata, !function)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_channel_push(lua_State *L)
{
	Channel *c = commonGetAs(L, 1, ChannelName, Channel *);
	Variant *v = variantGet(L, 2);

	if (v == NULL)
		return commonPushErrno(L, 1);

	channelPush(c, v);

	return commonPush(L, "b", 1);
}

/*
 * Channel:supply(value)
 *
 * Arguments:
 *	value the value to push (!userdata, !function)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_channel_supply(lua_State *L)
{
	Channel *c = commonGetAs(L, 1, ChannelName, Channel *);
	Variant *v = variantGet(L, 2);

	if (v == NULL)
		return commonPushErrno(L, 1);

	channelSupply(c, v);

	return commonPush(L, "b", 1);
}

/*
 * Channel:clear()
 */
static int
l_channel_clear(lua_State *L)
{
	channelClear(commonGetAs(L, 1, ChannelName, Channel *));

	return 0;
}

/*
 * Channel:pop()
 */
static int
l_channel_pop(lua_State *L)
{
	channelPop(commonGetAs(L, 1, ChannelName, Channel *));

	return 0;
}

/*
 * Channel:wait()
 *
 * Returns:
 *	The last value or nil
 */
static int
l_channel_wait(lua_State *L)
{
	Channel *c = commonGetAs(L, 1, ChannelName, Channel *);
	const Variant *v;

	if ((v = channelWait(c)) == NULL)
		lua_pushnil(L);

	variantPush(L, v);
	SDL_CondBroadcast(c->cond);

	return 1;
}

/*
 * Channel:__gc()
 */
static int
l_channel_gc(lua_State *L)
{
	Channel *c = commonGetAs(L, 1, ChannelName, Channel *);

	(void)SDL_AtomicDecRef(&c->ref);
	if (SDL_AtomicGet(&c->ref) == 0)
		channelFree(c);

	return 0;
}

static const luaL_Reg ChannelMethods[] = {
	{ "first",	l_channel_first		},
	{ "last",	l_channel_last		},
	{ "push",	l_channel_push		},
	{ "clear",	l_channel_clear		},
	{ "pop",	l_channel_pop		},
	{ "supply",	l_channel_supply	},
	{ "wait",	l_channel_wait		},
	{ NULL,		NULL			}
};

static const luaL_Reg ChannelMetaMethods[] = {
	{ "__gc",	l_channel_gc		},
	{ NULL,		NULL			}
};

const CommonObject ChannelObject = {
	"Channel",
	ChannelMethods,
	ChannelMetaMethods
};
