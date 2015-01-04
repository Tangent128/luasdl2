/*
 * thread.c -- thread creation management
 *
 * Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
 * Copyright (c) 2014 Joseph Wallace <tangent128@gmail.com>
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

#include <common/array.h>
#include <common/variant.h>

#include "thread.h"

/* --------------------------------------------------------
 * LuaThread private helpers
 * -------------------------------------------------------- */

typedef struct thread {
	lua_State	*L;
	SDL_Thread	*ptr;
	SDL_atomic_t	 ref;
	int		 joined;
} LuaThread;

typedef struct loadstate {
	Array		buffer;
	int		given;
} LoadState;

static int
writer(lua_State *L, const char *data, size_t sz, LoadState *state)
{
	size_t i;

	for (i = 0; i < sz; ++i) {
		if (arrayAppend(&state->buffer, &data[i]) < 0) {
			arrayFree(&state->buffer);
			lua_pushstring(L, strerror(errno));
			return -1;
		}
	}

	return 0;
}

static const char *
reader(lua_State *L, LoadState *state, size_t *size)
{
	(void)L;

	if (state->given) {
		*size = 0;
		state->given = 1;
		return NULL;
	}

	*size = state->buffer.length;

	return state->buffer.data;
}

static void
destroy(LuaThread *t)
{
	(void)SDL_AtomicDecRef(&t->ref);

	if (SDL_AtomicGet(&t->ref) == 0) {
		lua_close(t->L);
		free(t);
	}
}

static int
callback(LuaThread *t)
{
	int ret = -1;

	SDL_AtomicIncRef(&t->ref);

	if (lua_pcall(t->L, lua_gettop(t->L) - 1, 1, 0) != LUA_OK)
		SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", lua_tostring(t->L, -1));
	else
		ret = lua_tointeger(t->L, -1);

	destroy(t);

	return ret;
}

/* --------------------------------------------------------
 * LuaThread functions
 * -------------------------------------------------------- */

/*
 * SDL.createThread(name, source)
 *
 * Create a separate thread of execution. It creates a new Lua state that
 * does not share any data frmo the parent process.
 *
 * Arguments:
 *	name, the thread name
 *	source, a path to a Lua file or a function to call
 *
 * Returns:
 *	The thread object or nil
 *	The error message
 */
static int
l_thread_create(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	int ret, iv;
	LuaThread *thread;

	if ((thread = calloc(1, sizeof (LuaThread))) == NULL)
		return commonPushErrno(L, 1);

	thread->L = luaL_newstate();
	luaL_openlibs(thread->L);

	ret = threadDump(L, thread->L, 2);

	/* If return number is 2, it is nil and the error */
	if (ret == 2)
		goto failure;

	/* Iterate over the arguments to pass to the callback */
	for (iv = 3; iv <= lua_gettop(L); ++iv) {
		Variant *v = variantGet(L, iv);

		if (v == NULL) {
			commonPushErrno(L, 1);
			goto failure;
		}

		variantPush(thread->L, v);
		variantFree(v);
	}

	thread->ptr = SDL_CreateThread((SDL_ThreadFunction)callback, name, thread);
	if (thread->ptr == NULL) {
		commonPushSDLError(L, 1);
		goto failure;
	}

	SDL_AtomicIncRef(&thread->ref);

	return commonPush(L, "p", ThreadName, thread);

failure:
	lua_close(thread->L);
	free(thread);

	return 2;
}

const luaL_Reg ThreadFunctions[] = {
	{ "createThread",		l_thread_create			},
	{ NULL,				NULL				}
};

/* --------------------------------------------------------
 * LuaThread object methods
 * -------------------------------------------------------- */

/*
 * Thread:getId()
 *
 * Returns:
 *	The thread id
 */
static int
l_thread_getId(lua_State *L)
{
	LuaThread *t = commonGetAs(L, 1, ThreadName, LuaThread *);

	return commonPush(L, "i", SDL_GetThreadID(t->ptr));
}

/*
 * Thread:getName()
 *
 * Returns:
 *	The thread name
 */
static int
l_thread_getName(lua_State *L)
{
	LuaThread *t = commonGetAs(L, 1, ThreadName, LuaThread *);

	return commonPush(L, "s", SDL_GetThreadName(t->ptr));
}

/*
 * Thread:wait()
 *
 * Returns:
 *	The return code from the thread
 */
static int
l_thread_wait(lua_State *L)
{
	LuaThread *t = commonGetAs(L, 1, ThreadName, LuaThread *);
	int status;

	SDL_WaitThread(t->ptr, &status);
	t->joined = 1;

	return commonPush(L, "i", status);
}

/* --------------------------------------------------------
 * LuaThread object metamethods
 * -------------------------------------------------------- */

/*
 * Thread:__eq()
 */
static int
l_thread_eq(lua_State *L)
{
	LuaThread *t1 = commonGetAs(L, 1, ThreadName, LuaThread *);
	LuaThread *t2 = commonGetAs(L, 2, ThreadName, LuaThread *);

	return commonPush(L, "b", t1 == t2);
}

/*
 * Thread:__gc()
 */
static int
l_thread_gc(lua_State *L)
{
	LuaThread *t = commonGetAs(L, 1, ThreadName, LuaThread *);

#if SDL_PATCHLEVEL >= 2
	if (!t->joined)
		SDL_DetachThread(t->ptr);
#endif

	destroy(t);

	return 0;
}

/*
 * Thread:__tostring()
 */
static int
l_thread_tostring(lua_State *L)
{
	LuaThread *t = commonGetAs(L, 1, ThreadName, LuaThread *);

	lua_pushfstring(L, "thread %d", SDL_GetThreadID(t->ptr));

	return 1;
}

/* --------------------------------------------------------
 * LuaThread object definition
 * -------------------------------------------------------- */

static const luaL_Reg ThreadMethods[] = {
	{ "getId",			l_thread_getId			},
	{ "getName",			l_thread_getName		},
	{ "wait",			l_thread_wait			},
	{ NULL,				NULL				}
};

static const luaL_Reg ThreadMetamethods[] = {
	{ "__eq",			l_thread_eq			},
	{ "__gc",			l_thread_gc			},
	{ "__tostring",			l_thread_tostring		},
	{ NULL,				NULL				}
};

const CommonObject Thread = {
	"LuaThread",
	ThreadMethods,
	ThreadMetamethods
};

/* ---------------------------------------------------------
 * Thread functions
 * --------------------------------------------------------- */

static int
loadfunction(lua_State *owner, lua_State *thread, int index)
{
	LoadState state;
	int ret = 0;

	memset(&state, 0, sizeof (LoadState));

	if (arrayInit(&state.buffer, sizeof (char), 32) < 0) {
		ret = commonPushErrno(owner, 1);
		goto cleanup;
	}

	/* Copy the function at the top of the stack */
	lua_pushvalue(owner, index);

	if (lua_dump(owner, (lua_Writer)writer, &state, 0) != LUA_OK) {
		ret = commonPush(owner, "ns", "failed to dump function");
		goto cleanup;
	}

	/*
	 * If it fails, it pushes the error into the new state, move it to our
	 * state.
	 */
	if (lua_load(thread, (lua_Reader)reader, &state, "thread", NULL) != LUA_OK) {
		ret = commonPush(owner, "ns", lua_tostring(thread, -1));
		goto cleanup;
	}

cleanup:
	arrayFree(&state.buffer);

	return ret;
}

static int
loadfile(lua_State *owner, lua_State *thread, const char *path)
{
	if (luaL_loadfile(thread, path) != LUA_OK)
		return commonPush(owner, "ns", lua_tostring(thread, -1));

	return 0;
}

int
threadDump(lua_State *L, lua_State *th, int index)
{
	int ret;

	if (lua_type(L, index) == LUA_TSTRING)
		ret = loadfile(L, th, lua_tostring(L, index));
	else if (lua_type(L, index) == LUA_TFUNCTION)
		ret = loadfunction(L, th, index);
	else
		return luaL_error(L, "expected a file path or a function");

	return ret;
}
