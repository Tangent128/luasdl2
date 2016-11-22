/*
 * events-joystick.c -- joystick event management
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

#include "joystick.h"

/* --------------------------------------------------------
 * Joystick functions
 * -------------------------------------------------------- */

/*
 * SDL.joystickOpen(index)
 *
 * Arguments:
 *	index the joystick index
 *
 * Returns:
 *	The joystick object or nil on failure
 *	The error message
 */
static int
l_joystickOpen(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_Joystick *j;

	j = SDL_JoystickOpen(index);
	if (j == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", JoystickName, j);
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.joystickFromInstanceID(id)
 *
 * Arguments:
 *	id the Joystick's InstanceID
 *
 * Returns:
 *	the joystick object or nil on failure
 *	The error message
 */
 static int
 l_joystickFromInstanceID(lua_State *L)
 {
	 int id = luaL_checkinteger(L, 1);
	 SDL_Joystick *j;

	 j = SDL_JoystickFromInstanceID(id);
	 if (j == NULL)
	 return commonPushSDLError(L, 1);

	 return commonPush(L, "p", JoystickName, j);
 }
#endif

/*
 * SDL.joystickEventState(index)
 *
 * Arguments:
 *	state the state
 *
 * Returns:
 *	The status or nil on failure
 *	The error message
 */
static int
l_joystickEventState(lua_State *L)
{
	int state = luaL_checkinteger(L, 1);
	int ret;

	ret = SDL_JoystickEventState(state);
	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

/*
 * SDL.numJoysticks()
 *
 * Returns:
 *	The number of joysticks or nil on failure
 *	The error message
 */
static int
l_numJoysticks(lua_State *L)
{
	int ret = SDL_NumJoysticks();

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

/*
 * SDL.joystickNameForIndex(index)
 *
 * Arguments:
 *	index the joystick index
 *
 * Returns:
 *	The name or nil on failure
 *	The error message
 */
static int
l_joystickNameForIndex(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	const char *name;

	name = SDL_JoystickNameForIndex(index);
	if (name == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * SDL.joystickUpdate()
 */
static int
l_joystickUpdate(lua_State *L)
{
	SDL_JoystickUpdate();

	(void)L;

	return 0;
}

const luaL_Reg JoystickFunctions[] = {
	{ "joystickOpen",		l_joystickOpen			},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "joystickFromInstanceID",	l_joystickFromInstanceID	},
#endif
	{ "joystickEventState",		l_joystickEventState		},
	{ "numJoysticks",		l_numJoysticks			},
	{ "joystickNameForIndex",	l_joystickNameForIndex		},
	{ "joystickUpdate",		l_joystickUpdate		},
	{ NULL,				NULL				}
};

/* --------------------------------------------------------
 * Joystick object private helpers
 * -------------------------------------------------------- */

typedef int (NumFunc)(SDL_Joystick *);

static int
joystickGetNum(lua_State *L, NumFunc func)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	int value;

	value = func(j);
	if (value < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", value);
}

/* --------------------------------------------------------
 * Joystick object methods
 * -------------------------------------------------------- */

/*
 * Joystick:currentPowerlevel()
 *
 * Returns:
 *	The current powerlevel of the joystick, or SDL.joystickPowerLevel.Unknown if unknown
 *	An error message if unknown
 */
static int
l_joystick_currentPowerLevel(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	SDL_JoystickPowerLevel pl;

	if ((pl = SDL_JoystickCurrentPowerLevel(j)) == SDL_JOYSTICK_POWER_UNKNOWN)
		return commonPush(L, "is", pl, SDL_GetError());

	return commonPush(L, "i", pl);
}

/*
 * Joystick:getAttached()
 *
 * Returns:
 *	True if attached
 */
static int
l_joystick_getAttached(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);

	if (!SDL_JoystickGetAttached(j))
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Joystick:getAxis(axis)
 *
 * Arguments:
 *	axis the axis number
 *
 * Returns:
 *	The value
 */
static int
l_joystick_getAxis(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	int axis	= luaL_checkinteger(L, 2);

	return commonPush(L, "i", SDL_JoystickGetAxis(j, axis));
}

/*
 * Joystick:getBall(ball)
 *
 * Arguments:
 *	ball the ball index
 *
 * Returns:
 *	The x
 *	The y
 */
static int
l_joystick_getBall(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	int ball	= luaL_checkinteger(L, 2);
	int dx, dy;

	if (SDL_JoystickGetBall(j, ball, &dx, &dy) < 0)
		return commonPushSDLError(L, 2);

	return commonPush(L, "ii", dx, dy);
}

/*
 * Joystick:getButton(which)
 *
 * Arguments:
 *	which the button number
 *
 * Returns:
 *	The value
 */
static int
l_joystick_getButton(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	int button	= luaL_checkinteger(L, 2);

	return commonPush(L, "b", SDL_JoystickGetButton(j, button));
}

/*
 * Joystick:getHat(which)
 *
 * Arguments:
 *	which the hat index
 *
 * Returns:
 *	The value (SDL.joyHat)
 */
static int
l_joystick_getHat(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	int button	= luaL_checkinteger(L, 2);

	return commonPush(L, "i", SDL_JoystickGetHat(j, button));

}

/*
 * Joystick:instanceID()
 *
 * Returns:
 *	The id
 */
static int
l_joystick_instanceID(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);

	return commonPush(L, "i", SDL_JoystickInstanceID(j));
}

/*
 * Joystick:name()
 *
 * Returns:
 *	The name as a string or nil on failure
 *	The error message
 */
static int
l_joystick_name(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	const char *name;

	name = SDL_JoystickName(j);
	if (name == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * Joystick:numAxes()
 *
 * Returns:
 *	The number of axes or nil on failure
 *	The error message
 */
static int
l_joystick_numAxes(lua_State *L)
{
	return joystickGetNum(L, SDL_JoystickNumAxes);
}

/*
 * Joystick:numBalls()
 *
 * Returns:
 *	The number of balls or nil on failure
 *	The error message
 */
static int
l_joystick_numBalls(lua_State *L)
{
	return joystickGetNum(L, SDL_JoystickNumBalls);
}

/*
 * Joystick:numButtons()
 *
 * Returns:
 *	The number of buttons or nil on failure
 *	The error message
 */
static int
l_joystick_numButtons(lua_State *L)
{
	return joystickGetNum(L, SDL_JoystickNumButtons);
}

/*
 * Joystick:numHats()
 *
 * Returns:
 *	The number of hats or nil on failure
 */
static int
l_joystick_numHats(lua_State *L)
{
	return joystickGetNum(L, SDL_JoystickNumHats);
}

/* --------------------------------------------------------
 * Joystick object metamethods
 * -------------------------------------------------------- */

/*
 * Joystick:__eq()
 */
static int
l_joystick_eq(lua_State *L)
{
	SDL_Joystick *j1 = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	SDL_Joystick *j2 = commonGetAs(L, 1, JoystickName, SDL_Joystick *);

	return commonPush(L, "b", j1 == j2);
}

/*
 * Joystick:__gc()
 */
static int
l_joystick_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, JoystickName);

	if (udata->mustdelete)
		SDL_JoystickClose(udata->data);

	return 0;
}

/*
 * Joystick:__tostring()
 */
static int
l_joystick_tostring(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	SDL_JoystickID id;
	const char *name;
	const char *attached;

	id = SDL_JoystickInstanceID(j);

	/* Has a name? */
	name = SDL_JoystickName(j);
	if (name == NULL)
		name = "Unknown";

	attached = SDL_JoystickGetAttached(j) == 0 ? "false" : "true";

	lua_pushfstring(L, "joystick %d: \"%s\" attached: %s, axes: %d, buttons: %d, balls: %d, hats: %d",
	    id,
	    name,
	    attached,
	    SDL_JoystickNumAxes(j),
	    SDL_JoystickNumButtons(j),
	    SDL_JoystickNumBalls(j),
	    SDL_JoystickNumHats(j)
	);

	return 1;
}

/* --------------------------------------------------------
 * Joystick object definition
 * -------------------------------------------------------- */

static const luaL_Reg JoystickMethods[] = {
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "currentPowerlevel",		l_joystick_currentPowerLevel	},
#endif
	{ "getAttached",		l_joystick_getAttached		},
	{ "getAxis",			l_joystick_getAxis		},
	{ "getBall",			l_joystick_getBall		},
	{ "getButton",			l_joystick_getButton		},
	{ "getHat",			l_joystick_getHat		},
	{ "instanceID",			l_joystick_instanceID		},
	{ "name",			l_joystick_name			},
	{ "numAxes",			l_joystick_numAxes		},
	{ "numBalls",			l_joystick_numBalls		},
	{ "numButtons",			l_joystick_numButtons		},
	{ "numHats",			l_joystick_numHats		},
	{ NULL,				NULL				}
};

static const luaL_Reg JoystickMetamethods[] = {
	{ "__eq",			l_joystick_eq			},
	{ "__gc",			l_joystick_gc			},
	{ "__tostring",			l_joystick_tostring		},
	{ NULL,				NULL				}
};

const CommonObject Joystick = {
	"Joystick",
	JoystickMethods,
	JoystickMetamethods
};

/*
 * SDL.joyHat
 */
const CommonEnum EventJoyHat[] = {
	{ "Left",	SDL_HAT_LEFT		},
	{ "LeftUp",	SDL_HAT_LEFTUP		},
	{ "Up",		SDL_HAT_UP		},
	{ "RightUp",	SDL_HAT_RIGHTUP		},
	{ "Right",	SDL_HAT_RIGHT		},
	{ "RightDown",	SDL_HAT_RIGHTDOWN	},
	{ "Down",	SDL_HAT_DOWN		},
	{ "LeftDown",	SDL_HAT_LEFTDOWN	},
	{ NULL,		-1			}
};

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.joystickPowerLevel
 */
const CommonEnum JoystickPowerLevels[] = {
	{ "Unknown",	SDL_JOYSTICK_POWER_UNKNOWN	},
	{ "Empty",	SDL_JOYSTICK_POWER_EMPTY	},
	{ "Low",	SDL_JOYSTICK_POWER_LOW		},
	{ "Medium",	SDL_JOYSTICK_POWER_MEDIUM	},
	{ "Full",	SDL_JOYSTICK_POWER_FULL		},
	{ "Wired",	SDL_JOYSTICK_POWER_WIRED	},
	{ "Max",	SDL_JOYSTICK_POWER_MAX		},
	{ NULL,		-1				}
};
#endif
