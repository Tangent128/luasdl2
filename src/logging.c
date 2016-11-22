/*
 * logging.c -- logging routines
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

#include <stdarg.h>

#include "logging.h"

/* --------------------------------------------------------
 * Logging private helpers
 * -------------------------------------------------------- */

typedef void (*LogFunc)(int, const char *, ...);

static int loggingOutputFunc = LUA_REFNIL;

static void *
loggingCustomOutput(lua_State *L, int category, int priority, const char *msg)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, loggingOutputFunc);
	lua_pushinteger(L, category);
	lua_pushinteger(L, priority);
	lua_pushstring(L, msg);
	lua_call(L, 3, 0);

	/* What is the use of that return? */
	return NULL;
}

static int
loggingLog(lua_State *L, LogFunc func)
{
	int category	= luaL_checkinteger(L, 1);
	const char *msg	= luaL_checkstring(L, 2);

	func(category, "%s", msg);

	return 0;
}

/* --------------------------------------------------------
 * Logging functions
 * -------------------------------------------------------- */

/*
 * SDL.log(msg)
 *
 * Arguments:
 *	msg the message
 */
static int
l_log(lua_State *L)
{
	SDL_Log("%s", luaL_checkstring(L, 1));

	return 0;
}

/*
 * SDL.logCritical(category, msg)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	msg the message
 */
static int
l_logCritical(lua_State *L)
{
	return loggingLog(L, SDL_LogCritical);
}

/*
 * SDL.logDebug(category, msg)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	msg the message
 */
static int
l_logDebug(lua_State *L)
{
	return loggingLog(L, SDL_LogDebug);
}

/*
 * SDL.logError(category, msg)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	msg the message
 */
static int
l_logError(lua_State *L)
{
	return loggingLog(L, SDL_LogError);
}

/*
 * SDL.logGetOutputFunction()
 *
 * Returns:
 *	The function or nil
 */
static int
l_logGetOutputFunction(lua_State *L)
{
	if (loggingOutputFunc == LUA_REFNIL)
		lua_pushnil(L);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, loggingOutputFunc);

	return 1;
}

/*
 * SDL.logGetPriority(category)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *
 * Returns:
 *	The priority (SDL.logPriority)
 */
static int
l_logGetPriority(lua_State *L)
{
	int category = luaL_checkinteger(L, 1);

	return commonPush(L, "i", SDL_LogGetPriority(category));
}

/*
 * SDL.logInfo(category, msg)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	msg the message
 */
static int
l_logInfo(lua_State *L)
{
	return loggingLog(L, SDL_LogInfo);
}

/*
 * SDL.logMessage(category, priority, msg)
 *
 * Arguments:
 *	category the category
 *	priority the priority
 *	msg the message
 */
static int
l_logMessage(lua_State *L)
{
	int category	= luaL_checkinteger(L, 1);
	int priority	= luaL_checkinteger(L, 2);
	const char *msg	= luaL_checkstring(L, 3);

	SDL_LogMessage(category, priority, "%s", msg);

	return 0;
}

/*
 * SDL.logResetPriorities()
 */
static int
l_logResetPriorities(lua_State *L)
{
	SDL_LogResetPriorities();

	(void)L;

	return 0;
}

/*
 * SDL.logSetAllPriority(priority)
 *
 * Arguments:
 *	priority (SDL.logPriority)
 */
static int
l_logSetAllPriority(lua_State *L)
{
	int priority = luaL_checkinteger(L, 1);

	SDL_LogSetAllPriority(priority);

	return 0;
}

/*
 * SDL.logSetOutputFunction(func)
 *
 * The function must have the following signature:
 *	function log(category, priority, message)
 *
 * Arguments:
 *	func the function
 */
static int
l_logSetOutputFunction(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);

	/* Remove old one if needed */
	if (loggingOutputFunc != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, loggingOutputFunc);

	lua_pushvalue(L, 1);
	loggingOutputFunc = luaL_ref(L, LUA_REGISTRYINDEX);

	SDL_LogSetOutputFunction((SDL_LogOutputFunction)loggingCustomOutput, L);

	return 0;
}

/*
 * SDL.logSetPriority(category, priority)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	priority the priority (SDL.logPriority)
 */
static int
l_logSetPriority(lua_State *L)
{
	int category	= luaL_checkinteger(L, 1);
	int priority	= luaL_checkinteger(L, 2);

	SDL_LogSetPriority(category, priority);

	(void)L;

	return 0;
}

/*
 * SDL.logVerbose(category, msg)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	msg the message
 */
static int
l_logVerbose(lua_State *L)
{
	return loggingLog(L, SDL_LogVerbose);
}

/*
 * SDL.logWarn(category, msg)
 *
 * Arguments:
 *	category the category (SDL.logCategory)
 *	msg the message
 */
static int
l_logWarn(lua_State *L)
{
	return loggingLog(L, SDL_LogWarn);
}

const luaL_Reg LoggingFunctions[] = {
	{ "log",			l_log				},
	{ "logCritical",		l_logCritical			},
	{ "logDebug",			l_logDebug			},
	{ "logError",			l_logError			},
	{ "logGetOutputFunction",	l_logGetOutputFunction		},
	{ "logGetPriority",		l_logGetPriority		},
	{ "logInfo",			l_logInfo			},
	{ "logMessage",			l_logMessage			},
	{ "logResetPriorities",		l_logResetPriorities		},
	{ "logSetAllPriority",		l_logSetAllPriority		},
	{ "logSetOutputFunction",	l_logSetOutputFunction		},
	{ "logSetPriority",		l_logSetPriority		},
	{ "logVerbose",			l_logVerbose			},
	{ "logWarn",			l_logWarn			},
	{ NULL,				NULL				}
};

/*
 * SDL.logCategory
 */
const CommonEnum LoggingCategory[] = {
	{ "Application",		SDL_LOG_CATEGORY_APPLICATION	},
	{ "Error",			SDL_LOG_CATEGORY_ERROR		},
	{ "System",			SDL_LOG_CATEGORY_SYSTEM		},
	{ "Audio",			SDL_LOG_CATEGORY_AUDIO		},
	{ "Video",			SDL_LOG_CATEGORY_VIDEO		},
	{ "Render",			SDL_LOG_CATEGORY_RENDER		},
	{ "Input",			SDL_LOG_CATEGORY_INPUT		},
	{ "Custom",			SDL_LOG_CATEGORY_CUSTOM		},
	{ NULL,				-1				}
};

/*
 * SDL.logPriority
 */
const CommonEnum LoggingPriority[] = {
	{ "Verbose",			SDL_LOG_PRIORITY_VERBOSE	},
	{ "Debug",			SDL_LOG_PRIORITY_DEBUG		},
	{ "Info",			SDL_LOG_PRIORITY_INFO		},
	{ "Warn",			SDL_LOG_PRIORITY_WARN		},
	{ "Error",			SDL_LOG_PRIORITY_ERROR		},
	{ "Critical",			SDL_LOG_PRIORITY_CRITICAL	},
	{ NULL,				-1				}
};
