/*
 * gamecontroller.c -- game controllers event management
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

#include "gamecontroller.h"
#include "common/rwops.h"

/* --------------------------------------------------------
 * Gamecontroller functions
 * -------------------------------------------------------- */

/*
 * SDL.gameControllerAddMapping(name)
 *
 * Arguments:
 *	name the mapping string
 *
 * Returns:
 *	1, 0 or -1
 *	The error message
 */
static int
l_gameControllerAddMapping(lua_State *L)
{
	const char *mapping = luaL_checkstring(L, 1);
	int ret;

	ret = SDL_GameControllerAddMapping(mapping);
	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

#if SDL_VERSION_ATLEAST(2, 0, 2)
/*
 * SDL.gameControllerAddMappingsFromFile(filename)
 *
 * Arguments:
 *	filename the file containing the mappings to load
 *
 * Returns:
 *	The number of mappings loaded, or -1 on error
 *	The error message
 */
static int
l_gameControllerAddMappingsFromFile(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);
	int ret;

	ret = SDL_GameControllerAddMappingsFromFile(filename);
	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

/*
 * SDL.gameControllerAddMappingsFromRW(rw)
 *
 * Arguments:
 *	rw the SDL_RWops to load from
 *
 * Returns:
 *	The number of mappings loaded, or -1 on error
 *	The error message
 */
static int
l_gameControllerAddMappingsFromRW(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	int ret;

	ret = SDL_GameControllerAddMappingsFromRW(ops, 0);
	if (ret < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}
#endif

/*
 * SDL.gameControllerOpen(index)
 *
 * Arguments:
 *	index the controller index
 *
 * Returns:
 *	The controller object or nil on failure
 *	The error message
 */
static int
l_gameControllerOpen(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	SDL_GameController *c;

	c = SDL_GameControllerOpen(index);
	if (c == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", GameCtlName, c);
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
/*
 * SDL.gameControllerFromInstanceID(id)
 *
 * Arguments:
 *	id the controller InstanceID
 *
 * Returns:
 *	The controller object or nil on failure
 *	The error message
 */
static int
l_gameControllerFromInstanceID(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	SDL_GameController *c;

	c = SDL_GameControllerFromInstanceID(id);
	if (c == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", GameCtlName, c);
}
#endif

/*
 * SDL.gameControllerNameForIndex(index)
 *
 * Arguments:
 *	index the controller index
 *
 * Returns:
 *	The name or nil on failure
 *	The error message
 */
static int
l_gameControllerNameForIndex(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	const char *name;

	name = SDL_GameControllerNameForIndex(index);
	if (name == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * SDL.isGameController(index)
 *
 * Arguments:
 *	index the controller index
 *
 * Returns:
 *	True if is a game controller
 */
static int
l_isGameController(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);

	return commonPush(L, "b", SDL_IsGameController(index));
}

const luaL_Reg GamectlFunctions[] = {
	{ "gameControllerAddMapping",		l_gameControllerAddMapping		},
#if SDL_VERSION_ATLEAST(2, 0, 2)
	{ "gameControllerAddMappingsFromFile",	l_gameControllerAddMappingsFromFile	},
	{ "gameControllerAddMappingsFromRW",	l_gameControllerAddMappingsFromRW	},
	{ "gameControllerFromInstanceID",	l_gameControllerFromInstanceID		},
#endif
	{ "gameControllerOpen",			l_gameControllerOpen			},
	{ "gameControllerNameForIndex",		l_gameControllerNameForIndex		},
	{ "isGameController",			l_isGameController			},
	{ NULL,					NULL					}
};

/* --------------------------------------------------------
 * Gamecontroller object methods
 * -------------------------------------------------------- */

/*
 * Controller:name()
 *
 * Returns:
 *	The controller name
 */
static int
l_gamectl_name(lua_State *L)
{
	SDL_GameController *c = commonGetAs(L, 1, GameCtlName, SDL_GameController *);
	const char *name;

	name = SDL_GameControllerName(c);
	if (name == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * Controller:getAttached()
 *
 * Returns:
 *	True if attached
 */
static int
l_gamectl_getAttached(lua_State *L)
{
	SDL_GameController *c = commonGetAs(L, 1, GameCtlName, SDL_GameController *);

	return commonPush(L, "b", SDL_GameControllerGetAttached(c));
}

/* --------------------------------------------------------
 * Gamecontroller object metamethods
 * -------------------------------------------------------- */

/*
 * Controller:__eq()
 */
static int
l_gamectl_eq(lua_State *L)
{
	SDL_GameController *c1 = commonGetAs(L, 1, GameCtlName, SDL_GameController *);
	SDL_GameController *c2 = commonGetAs(L, 1, GameCtlName, SDL_GameController *);

	return commonPush(L, "b", c1 == c2);

}

/*
 * Controller:__gc()
 */
static int
l_gamectl_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, GameCtlName);

	if (udata->mustdelete)
		SDL_GameControllerClose(udata->data);

	return 0;
}

/*
 * Controller:__tostring()
 */
static int
l_gamectl_tostring(lua_State *L)
{
	SDL_GameController *c = commonGetAs(L, 1, GameCtlName, SDL_GameController *);
	const char *name;
	const char *attached;

	name = SDL_GameControllerName(c);
	if (name == NULL)
		name = "Unknown";

	attached = SDL_GameControllerGetAttached(c) == 0 ? "false" : "true";
	lua_pushfstring(L, "gamecontroller \"%s\" attached: %s", name, attached);

	return 1;
}

/* --------------------------------------------------------
 * Gamecontroller object definition
 * -------------------------------------------------------- */

static const luaL_Reg GamectlMethods[] = {
	{ "name",			l_gamectl_name				},
	{ "getAttached",		l_gamectl_getAttached			},
	{ NULL,				NULL					}
};

static const luaL_Reg GamectlMetamethods[] = {
	{ "__eq",			l_gamectl_eq				},
	{ "__gc",			l_gamectl_gc				},
	{ "__tostring",			l_gamectl_tostring			},
	{ NULL,				NULL					}
};

const CommonObject GameCtl = {
	"GameController",
	GamectlMethods,
	GamectlMetamethods
};

/*
 * SDL.controllerAxis
 */
const CommonEnum GameCtlAxis[] = {
	{ "LeftX",		SDL_CONTROLLER_AXIS_LEFTX		},
	{ "LeftY",		SDL_CONTROLLER_AXIS_LEFTY		},
	{ "RightX",		SDL_CONTROLLER_AXIS_RIGHTX		},
	{ "RightY",		SDL_CONTROLLER_AXIS_RIGHTY		},
	{ "TriggerLeft",	SDL_CONTROLLER_AXIS_TRIGGERLEFT		},
	{ "TriggerRight",	SDL_CONTROLLER_AXIS_TRIGGERRIGHT	},
	{ NULL,			-1					}
};

/*
 * SDL.controllerButton
 */
const CommonEnum GameCtlButton[] = {
	{ "A",			SDL_CONTROLLER_BUTTON_A			},
	{ "B",			SDL_CONTROLLER_BUTTON_B			},
	{ "X",			SDL_CONTROLLER_BUTTON_X			},
	{ "Y",			SDL_CONTROLLER_BUTTON_Y			},
	{ "Back",		SDL_CONTROLLER_BUTTON_BACK		},
	{ "Guide",		SDL_CONTROLLER_BUTTON_GUIDE		},
	{ "Start",		SDL_CONTROLLER_BUTTON_START		},
	{ "LeftStick",		SDL_CONTROLLER_BUTTON_LEFTSTICK		},
	{ "RightStick",		SDL_CONTROLLER_BUTTON_RIGHTSTICK	},
	{ "LeftShoulder",	SDL_CONTROLLER_BUTTON_LEFTSHOULDER	},
	{ "RightShoulder",	SDL_CONTROLLER_BUTTON_RIGHTSHOULDER	},
	{ "Up",			SDL_CONTROLLER_BUTTON_DPAD_UP		},
	{ "Down",		SDL_CONTROLLER_BUTTON_DPAD_DOWN		},
	{ "Left",		SDL_CONTROLLER_BUTTON_DPAD_LEFT		},
	{ "Right",		SDL_CONTROLLER_BUTTON_DPAD_RIGHT	},
	{ NULL,		-1			}
};
