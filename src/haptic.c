/*
 * haptic.c -- force feedback control
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

#include <string.h>

#include <common/table.h>

#include "haptic.h"
#include "joystick.h"

/* --------------------------------------------------------
 * Private helpers
 * -------------------------------------------------------- */

typedef int (*NumFunc)(SDL_Haptic *);
typedef int (*ToggleFunc)(SDL_Haptic *);
typedef int (*SetFunc)(SDL_Haptic *, int);

static int hapticNum(lua_State *L, NumFunc func)
{
	SDL_Haptic *h = commonGetAs(L, 1, HapticName, SDL_Haptic *);
	int num = func(h);

	if (num < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", num);
}

static int
hapticToggle(lua_State *L, ToggleFunc func)
{
	SDL_Haptic *h = commonGetAs(L, 1, HapticName, SDL_Haptic *);

	if (func(h) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static int
hapticSet(lua_State *L, SetFunc func)
{
	SDL_Haptic *h	= commonGetAs(L, 1, HapticName, SDL_Haptic *);
	int value	= luaL_checkinteger(L, 2);

	if (func(h, value) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static void
getDirection(lua_State *L, int hindex, int index, SDL_HapticDirection *direction)
{
	SDL_zero(direction);

	direction->type = tableGetInt(L, index, "type");

	/* Iterate over the dir fields */
	lua_getfield(L, index, "direction");

	if (lua_type(L, -1) == LUA_TTABLE) {
		int i;

		for (i = 0; i < 3; ++i) {
			lua_rawgeti(L, -1, i + 1);

			if (lua_type(L, -1) == LUA_TNUMBER)
				direction->dir[i] = (Sint32)lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);

	(void)hindex;
}

static void
getConstant(lua_State *L, int hindex, int index, SDL_HapticConstant *constant)
{
	constant->length	= tableGetInt(L, index, "length");
	constant->delay		= tableGetInt(L, index, "delay");
	constant->button	= tableGetInt(L, index, "button");
	constant->interval	= tableGetInt(L, index, "interval");
	constant->level		= tableGetInt(L, index, "level");
	constant->attack_length	= tableGetInt(L, index, "attackLength");
	constant->attack_level	= tableGetInt(L, index, "attackLevel");
	constant->fade_length	= tableGetInt(L, index, "fadeLength");
	constant->fade_level	= tableGetInt(L, index, "fadeLevel");

	/* Direction */
	lua_getfield(L, index, "direction");

	if (lua_type(L, -1) != LUA_TTABLE)
		luaL_error(L, "direction field must be table");

	getDirection(L, hindex, -1, &constant->direction);
	lua_pop(L, 1);

	(void)hindex;
}

static void
getPeriodic(lua_State *L, int hindex, int index, SDL_HapticPeriodic *periodic)
{
	periodic->length	= tableGetInt(L, index, "length");
	periodic->delay		= tableGetInt(L, index, "delay");
	periodic->button	= tableGetInt(L, index, "button");
	periodic->interval	= tableGetInt(L, index, "interval");
	periodic->period	= tableGetInt(L, index, "period");
	periodic->magnitude	= tableGetInt(L, index, "magnitude");
	periodic->offset	= tableGetInt(L, index, "offset");
	periodic->phase		= tableGetInt(L, index, "phase");
	periodic->attack_length	= tableGetInt(L, index, "attackLength");
	periodic->attack_level	= tableGetInt(L, index, "attackLevel");
	periodic->fade_length	= tableGetInt(L, index, "fadeLength");
	periodic->fade_level	= tableGetInt(L, index, "fadeLevel");

	/* Direction */
	lua_getfield(L, index, "direction");

	if (lua_type(L, -1) != LUA_TTABLE)
		luaL_error(L, "direction field must be table");

	getDirection(L, hindex, -1, &periodic->direction);
	lua_pop(L, 1);

	(void)hindex;
}

static void
getCondition(lua_State *L, int hindex, int index, SDL_HapticCondition *condition)
{
	struct {
		const char	*name;
		Uint16		*array;
	} map[] = {
		{ "rightSat",	(Uint16 *)condition->right_sat		},
		{ "leftSat",	(Uint16 *)condition->left_sat		},
		{ "rightCoeff",	(Uint16 *)condition->right_coeff	},
		{ "leftCoeff",	(Uint16 *)condition->left_coeff		},
		{ "deadband",	(Uint16 *)condition->deadband		},
		{ "center",	(Uint16 *)condition->center		},
		{ NULL,		NULL					}
	};
	int i, j;

	condition->length	= tableGetInt(L, index, "length");
	condition->delay	= tableGetInt(L, index, "delay");
	condition->button	= tableGetInt(L, index, "button");
	condition->interval	= tableGetInt(L, index, "interval");

	/*
	 * Iterate over the tables
	 */
	for (i = 0; map[i].name != NULL; ++i) {
		memset(map[i].array, 0, sizeof (Uint16) * 3);

		if (!tableIsType(L, index, map[i].name, LUA_TTABLE))
			continue;

		lua_getfield(L, index, map[i].name);
		for (j = 0; j < 3; ++j) {
			lua_rawgeti(L, -1, i + 1);

			if (lua_type(L, -1) == LUA_TNUMBER)
				map[i].array[j] = (Uint16)lua_tonumber(L, -1);

			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	/* Direction */
	lua_getfield(L, index, "direction");

	if (lua_type(L, -1) != LUA_TTABLE)
		luaL_error(L, "direction field must be table");

	getDirection(L, hindex, -1, &condition->direction);
	lua_pop(L, 1);

	(void)hindex;
}

static void
getRamp(lua_State *L, int hindex, int index, SDL_HapticRamp *ramp)
{
	ramp->length		= tableGetInt(L, index, "length");
	ramp->delay		= tableGetInt(L, index, "delay");
	ramp->button		= tableGetInt(L, index, "button");
	ramp->interval		= tableGetInt(L, index, "interval");
	ramp->start		= tableGetInt(L, index, "start");
	ramp->end		= tableGetInt(L, index, "end");
	ramp->attack_length	= tableGetInt(L, index, "attackLength");
	ramp->attack_level	= tableGetInt(L, index, "attackLevel");
	ramp->fade_length	= tableGetInt(L, index, "fadeLength");
	ramp->fade_level	= tableGetInt(L, index, "fadeLevel");

	/* Direction */
	lua_getfield(L, index, "direction");

	if (lua_type(L, -1) != LUA_TTABLE)
		luaL_error(L, "direction field must be table");

	getDirection(L, hindex, -1, &ramp->direction);
	lua_pop(L, 1);

	(void)hindex;
}

static void
getLeftRight(lua_State *L, int hindex, int index, SDL_HapticLeftRight *lr)
{
	lr->length		= tableGetInt(L, index, "length");
	lr->large_magnitude	= tableGetInt(L, index, "largeMagnitude");
	lr->small_magnitude	= tableGetInt(L, index, "smallMagnitude");

	(void)hindex;
}

#if 0

/*
 * Need to understand how SDL_HapticCustom fields work
 */

static void
getCustom(lua_State *L, int hindex, int index, SDL_HapticCustom *custom)
{
	custom->length		= tableGetInt(L, index, "length");
	custom->delay		= tableGetInt(L, index, "delay");
	custom->button		= tableGetInt(L, index, "button");
	custom->interval	= tableGetInt(L, index, "interval");

}

#endif

static void
getEffect(lua_State *L, int hindex, int index, SDL_HapticEffect *effect)
{
	luaL_checktype(L, index, LUA_TTABLE);

	effect->type = tableGetInt(L, index, "type");

	switch (effect->type) {
	case SDL_HAPTIC_CONSTANT:
		getConstant(L, hindex, index, &effect->constant);
		break;
	case SDL_HAPTIC_SINE:
#if defined(SDL_HAPTIC_SQUARE)
	case SDL_HAPTIC_SQUARE:
#endif
	case SDL_HAPTIC_TRIANGLE:
	case SDL_HAPTIC_SAWTOOTHUP:
	case SDL_HAPTIC_SAWTOOTHDOWN:
		getPeriodic(L, hindex, index, &effect->periodic);
		break;
	case SDL_HAPTIC_SPRING:
	case SDL_HAPTIC_DAMPER:
	case SDL_HAPTIC_INERTIA:
	case SDL_HAPTIC_FRICTION:
		getCondition(L, hindex, index, &effect->condition);
		break;
	case SDL_HAPTIC_RAMP:
		getRamp(L, hindex, index, &effect->ramp);
		break;
	case SDL_HAPTIC_LEFTRIGHT:
		getLeftRight(L, hindex, index, &effect->leftright);
		break;
	case SDL_HAPTIC_CUSTOM:
		luaL_error(L, "custom currently not implemented");
		break;
	default:
		luaL_error(L, "unknown type %d", effect->type);
		break;
	}
}


/* --------------------------------------------------------
 * Haptic methods
 * -------------------------------------------------------- */

/*
 * Haptic:destroyEffect(effect)
 *
 * Arguments:
 *	effect the effect id
 */
static int
l_haptic_destroyEffect(lua_State *L)
{
	SDL_Haptic *h	= commonGetAs(L, 1, HapticName, SDL_Haptic *);
	int effect	= luaL_checkinteger(L, 2);

	SDL_HapticDestroyEffect(h, effect);
	
	return 0;
}

/*
 * Haptic:effectSupported(effect)
 *
 * Arguments:
 *	effect the effect definition
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_effectSupported(lua_State *L)
{
	SDL_Haptic *h = commonGetAs(L, 1, HapticName, SDL_Haptic *);
	SDL_HapticEffect e;
	int ret;

	getEffect(L, 1, 2, &e);
	if ((ret = SDL_HapticEffectSupported(h, &e)) < 0)
		return commonPushSDLError(L, 1);

	return 0;
}

/*
 * Haptic:getEffectStatus(effect)
 *
 * Arguments:
 *	effect the effect
 *
 * Returns:
 *	true if playing
 */
static int
l_haptic_getEffectStatus(lua_State *L)
{
	SDL_Haptic *h	= commonGetAs(L, 1, HapticName, SDL_Haptic *);
	int effect	= luaL_checkinteger(L, 2);

	return commonPush(L, "b", SDL_HapticGetEffectStatus(h, effect));
}

/*
 * Haptic:index()
 *
 * Returns:
 *	The haptic index
 */
static int
l_haptic_index(lua_State *L)
{
	SDL_Haptic *h = commonGetAs(L, 1, HapticName, SDL_Haptic *);

	return commonPush(L, "i", SDL_HapticIndex(h));
}

/*
 * Haptic:newEffect(effect)
 *
 * Arguments:
 *	effect the effect description
 *
 * Returns:
 *	The effect id or nil on failure
 *	The error message
 */
static int
l_haptic_newEffect(lua_State *L)
{
	SDL_Haptic *h = commonGetAs(L, 1, HapticName, SDL_Haptic *);
	SDL_HapticEffect e;
	int ret;

	getEffect(L, 1, 2, &e);

	if ((ret = SDL_HapticNewEffect(h, &e)) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

/*
 * Haptic:numAxes()
 *
 * Returns:
 *	The number of axis or nil on failure
 *	The error message
 */
static int
l_haptic_numAxes(lua_State *L)
{
	return hapticNum(L, SDL_HapticNumAxes);
}

/*
 * Haptic:numEffects()
 *
 * Returns:
 *	The number of effects or nil on failure
 *	The error message
 */
static int
l_haptic_numEffects(lua_State *L)
{
	return hapticNum(L, SDL_HapticNumEffects);
}

/*
 * Haptic:numEffectsPlaying()
 *
 * Returns:
 *	The number of effects playing or nil on failure
 *	The error message
 */
static int
l_haptic_numEffectsPlaying(lua_State *L)
{
	return hapticNum(L, SDL_HapticNumEffectsPlaying);
}

/*
 * Haptic:pause()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_pause(lua_State *L)
{
	return hapticToggle(L, SDL_HapticPause);
}

/*
 * Haptic:rumbleInit()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_rumbleInit(lua_State *L)
{
	return hapticToggle(L, SDL_HapticRumbleInit);
}

/*
 * Haptic:rumblePlay(strength, length)
 *
 * Arguments:
 *	strength the strength
 *	length the duration
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_rumblePlay(lua_State *L)
{
	SDL_Haptic *h	= commonGetAs(L, 1, HapticName, SDL_Haptic *);
	float strength	= (float)luaL_checknumber(L, 2);
	int length	= luaL_checkinteger(L, 3);

	if (SDL_HapticRumblePlay(h, strength, length) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Haptic:rumbleStop()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_rumbleStop(lua_State *L)
{
	return hapticToggle(L, SDL_HapticRumbleStop);
}

/*
 * Haptic:rumbleSupported()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_rumbleSupported(lua_State *L)
{
	return hapticToggle(L, SDL_HapticRumbleSupported);
}

/*
 * Haptic:runEffect(effect, iterations)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_runEffect(lua_State *L)
{
	SDL_Haptic *h	= commonGetAs(L, 1, HapticName, SDL_Haptic *);
	int effect	= luaL_checkinteger(L, 2);
	int iterations	= luaL_checkinteger(L, 3);

	if (SDL_HapticRunEffect(h, effect, iterations) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * HaptiC:setAutocenter(value)
 *
 * Arguments:
 *	value the center value
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_setAutocenter(lua_State *L)
{
	return hapticSet(L, SDL_HapticSetAutocenter);
}

/*
 * HaptiC:setGain(value)
 *
 * Arguments:
 *	value the gain value
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_setGain(lua_State *L)
{
	return hapticSet(L, SDL_HapticSetGain);
}

/*
 * Haptic:stopAll()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_stopAll(lua_State *L)
{
	return hapticToggle(L, SDL_HapticStopAll);
}

/*
 * Haptic:stopEffect(effect)
 *
 * Arguments:
 *	effect the effect number
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_stopEffect(lua_State *L)
{
	return hapticSet(L, SDL_HapticStopEffect);
}

/*
 * Haptic:unpause()
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_unpause(lua_State *L)
{
	return hapticToggle(L, SDL_HapticUnpause);
}

/*
 * Haptic:updateEffect(index, e)
 *
 * Arguments:
 *	index the effect index
 *	e the new info
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_haptic_updateEffect(lua_State *L)
{
	SDL_Haptic *h	= commonGetAs(L, 1, HapticName, SDL_Haptic *);
	int index	= luaL_checkinteger(L, 2);
	SDL_HapticEffect e;

	getEffect(L, 1, 3, &e);

	if (SDL_HapticUpdateEffect(h, index, &e) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/* --------------------------------------------------------
 * Haptic metamethods
 * -------------------------------------------------------- */

static int
l_haptic_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, HapticName);

	if (udata->mustdelete)
		SDL_HapticClose(udata->data);

	return 0;
}

static const luaL_Reg HapticMethods[] = {
	{ "destroyEffect",	l_haptic_destroyEffect		},
	{ "effectSupported",	l_haptic_effectSupported	},
	{ "getEffectStatus",	l_haptic_getEffectStatus	},
	{ "index",		l_haptic_index			},
	{ "newEffect",		l_haptic_newEffect		},
	{ "numAxes",		l_haptic_numAxes		},
	{ "numEffects",		l_haptic_numEffects		},
	{ "numEffectsPlaying",	l_haptic_numEffectsPlaying	},
	{ "pause",		l_haptic_pause			},
	{ "rumbleInit",		l_haptic_rumbleInit		},
	{ "rumblePlay",		l_haptic_rumblePlay		},
	{ "rumbleStop",		l_haptic_rumbleStop		},
	{ "rumbleSupported",	l_haptic_rumbleSupported	},
	{ "runEffect",		l_haptic_runEffect		},
	{ "setAutocenter",	l_haptic_setAutocenter		},
	{ "setGain",		l_haptic_setGain		},
	{ "stopAll",		l_haptic_stopAll		},
	{ "stopEffect",		l_haptic_stopEffect		},
	{ "unpause",		l_haptic_unpause		},
	{ "updateEffect",	l_haptic_updateEffect		},
	{ NULL,			NULL				},
};

static const luaL_Reg HapticMetamethods[] = {
	{ "__gc",		l_haptic_gc			},
	{ NULL,			NULL				}
};

const CommonObject Haptic = {
	"Haptic",
	HapticMethods,
	HapticMetamethods
};

/* --------------------------------------------------------
 * Haptic functions
 * -------------------------------------------------------- */

/*
 * SDL.hapticOpen(index)
 *
 * Arguments:
 *	index the index
 *
 * Returns:
 *	The haptic objhect or nil on failure
 *	The error message
 */
static int
l_hapticOpen(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_Haptic *h;

	if ((h = SDL_HapticOpen(index)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", HapticName, h);
}

/*
 * SDL.hapticOpenFromJoystick(joystick)
 *
 * Arguments:
 *	joystick the joystick
 *
 * Returns:
 *	The haptic objhect or nil on failure
 *	The error message
 */
static int
l_hapticOpenFromJoystick(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	SDL_Haptic *h;

	if ((h = SDL_HapticOpenFromJoystick(j)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", HapticName, h);
}

/*
 * SDL.hapticOpenFromMouse()
 *
 * Returns:
 *	The haptic objhect or nil on failure
 *	The error message
 */
static int
l_hapticOpenFromMouse(lua_State *L)
{
	SDL_Haptic *h;

	if ((h = SDL_HapticOpenFromMouse()) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", HapticName, h);
}

/*
 * SDL.hapticOpened(index)
 *
 * Arguments:
 *	index the haptic index
 *
 * Returns:
 *	true if opened or false
 *	The error message
 */
static int
l_hapticOpened(lua_State *L)
{
	return commonPush(L, "b", SDL_HapticOpened(luaL_checkinteger(L, 1)) == 1);
}

/*
 * SDL.mouseIsHaptic()
 *
 * Returns:
 *	true if haptic or nil on failure
 *	The error message
 */
static int
l_mouseIsHaptic(lua_State *L)
{
	int ret = SDL_MouseIsHaptic();

	if (ret < 0)
		commonPushSDLError(L, 1);

	return commonPush(L, "b", ret == 1);
}

/*
 * SDL.joystickIsHaptic(joystick)
 *
 * Arguments:
 *	joystick the joystick
 *
 * Returns:
 *	true if haptic or nil on failure
 *	The error message
 */
static int
l_joystickIsHaptic(lua_State *L)
{
	SDL_Joystick *j = commonGetAs(L, 1, JoystickName, SDL_Joystick *);
	int ret = SDL_JoystickIsHaptic(j);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", ret == 1);
}

/*
 * SDL.numHaptics()
 *
 * Returns:
 *	The number of haptics or nil on failure
 *	The error message
 */
static int
l_numHaptics(lua_State *L)
{
	int ret = SDL_NumHaptics();

	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

const luaL_Reg HapticFunctions[] = {
	{ "hapticOpen",			l_hapticOpen			},
	{ "hapticOpenFromJoystick",	l_hapticOpenFromJoystick	},
	{ "hapticOpenFromMouse",	l_hapticOpenFromMouse		},
	{ "hapticOpened",		l_hapticOpened			},
	{ "mouseIsHaptic",		l_mouseIsHaptic			},
	{ "joystickIsHaptic",		l_joystickIsHaptic		},
	{ "numHaptics",			l_numHaptics			},
	{ NULL,				NULL				}
};

/*
 * SDL.hapticType
 */
const CommonEnum HapticType[] = {
	{ "Constant",			SDL_HAPTIC_CONSTANT		},
	{ "Sine",			SDL_HAPTIC_SINE			},
#if defined(SDL_HAPTIC_SQUARE)
	{ "Square",			SDL_HAPTIC_SQUARE		},
#endif
	{ "Triangle",			SDL_HAPTIC_TRIANGLE		},
	{ "SawToothUp",			SDL_HAPTIC_SAWTOOTHUP		},
	{ "SawToothDown",		SDL_HAPTIC_SAWTOOTHDOWN		},
	{ "Spring",			SDL_HAPTIC_SPRING		},
	{ "Damper",			SDL_HAPTIC_DAMPER		},
	{ "Inertia",			SDL_HAPTIC_INERTIA		},
	{ "Friction",			SDL_HAPTIC_FRICTION		},
	{ "Ramp",			SDL_HAPTIC_RAMP			},
	{ "LeftRight",			SDL_HAPTIC_LEFTRIGHT		},
	{ "Custom",			SDL_HAPTIC_CUSTOM		},
	{ NULL,				-1				}
};

/*
 * SDL.hapticDirection
 */
const CommonEnum HapticDirection[] = {
	{ "Polar",			SDL_HAPTIC_POLAR		},
	{ "Cartesian",			SDL_HAPTIC_CARTESIAN		},
	{ "Spherical",			SDL_HAPTIC_SPHERICAL		},
	{ NULL,				-1				}
};
