/*
 * SDL.c -- main SDL2 module
 *
 * Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
 * Copyright (c) 2014, 2015 Joseph Wallace <tangent128@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>

#include <config.h>

#include <common/rwops.h>
#include <common/surface.h>
#include <common/video.h>
#include <common/table.h>

#include "audio.h"
#include "channel.h"
#include "clipboard.h"
#include "cpu.h"
#include "display.h"
#include "events.h"
#include "filesystem.h"
#include "gamecontroller.h"
#include "gl.h"
#include "haptic.h"
#include "joystick.h"
#include "keyboard.h"
#include "logging.h"
#include "mouse.h"
#include "platform.h"
#include "power.h"
#include "rectangle.h"
#include "renderer.h"
#include "texture.h"
#include "timer.h"
#include "thread.h"
#include "window.h"

/*
 * SDL.flags
 */
static const CommonEnum InitFlags[] = {
	{ "Audio",		SDL_INIT_AUDIO		},
	{ "Events",		SDL_INIT_EVENTS		},
	{ "Everything",		SDL_INIT_EVERYTHING	},
	{ "GameController",	SDL_INIT_GAMECONTROLLER	},
	{ "Haptic",		SDL_INIT_HAPTIC		},
	{ "Joystick",		SDL_INIT_JOYSTICK	},
	{ "NoParachute",	SDL_INIT_NOPARACHUTE	},
	{ "Video",		SDL_INIT_VIDEO		},
	{ NULL,			-1			}
};

typedef int (*InitFunc)(Uint32 flags);

static int
init(lua_State *L, InitFunc func)
{
	Uint32 flags = 0;

	if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TTABLE)
		flags = commonGetEnum(L, 1);
	if (func(flags) == -1)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.init(flags)
 *
 * Arguments:
 *	flags the flags (see SDL.flags)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_init(lua_State *L)
{
	return init(L, SDL_Init);
}

/*
 * SDL.initSubSystem(flags)
 *
 * Arguments:
 *	flags the flags
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_initSubSystem(lua_State *L)
{
	return init(L, SDL_InitSubSystem);
}

/*
 * SDL.quit()
 */
static int
l_quit(lua_State *L)
{
	SDL_Quit();

	(void)L;

	return 0;
}

/*
 * SDL.quitSubSystem(flags)
 *
 * Arguments:
 *	flags the systems to quit
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_quitSubSystem(lua_State *L)
{
	Uint32 flags = 0;

	if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TTABLE)
		flags = commonGetEnum(L, 1);

	SDL_QuitSubSystem(flags);

	return 0;
}

/*
 * SDL.wasInit(flags)
 *
 * Arguments:
 *	flags the systems to check
 *
 * Returns:
 *	The table enumeration of loaded systems
 */
static int
l_wasInit(lua_State *L)
{
	Uint32 ret, flags = 0;

	if (lua_gettop(L) >= 1)
		flags = commonGetEnum(L, 1);

	ret = SDL_WasInit(flags);
	commonPushEnum(L, ret, InitFlags);

	return 1;
}

/*
 * SDL.clearError()
 */
static int
l_clearError(lua_State *L)
{
	SDL_ClearError();

	(void)L;

	return 0;
}

/*
 * SDL.getError()
 */
static int
l_getError(lua_State *L)
{
	return commonPush(L, "s", SDL_GetError());
}

/*
 * SDL.setError(str)
 *
 * Arguments:
 *	str the error
 */
static int
l_setError(lua_State *L)
{
	SDL_SetError("%s", luaL_checkstring(L, 1));

	return 0;
}

/*
 * SDL.clearHints()
 */
static int
l_clearHints(lua_State *L)
{
	SDL_ClearHints();

	(void)L;

	return 0;
}

/*
 * SDL.getHint(name)
 *
 * Arguments:
 *	name the hint name
 *
 * Returns:
 *	The hint value or nil
 */
static int
l_getHint(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	const char *value = SDL_GetHint(name);

	if (value == NULL)
		lua_pushnil(L);
	else
		lua_pushstring(L, value);

	return 1;
}

#if SDL_VERSION_ATLEAST(2, 0, 5)
/*
 * SDL.getHintBoolean(name[, default])
 *
 * Get a hint as a boolean value.
 * If the hint does not exist, returns the 'truthyness'
 * of the 'default' argument.
 *
 * Arguments:
 *	The name of the hint to query.
 *	The default value to return if the hint does not exist.
 *
 * Returns:
 *	The hint value as a boolean.
 */
static int
l_getHintBoolean(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	int val = lua_toboolean(L, 2);

	return commonPush(L, "b", SDL_GetHintBoolean(name, val));
}
#endif

/*
 * SDL.setHint(name, value)
 *
 * Arguments:
 *	name the hint name
 *	value the hint value
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_setHint(lua_State *L)
{
	const char *name	= luaL_checkstring(L, 1);
	const char *value	= luaL_checkstring(L, 2);

	return commonPush(L, "b", SDL_SetHint(name, value));
}

/*
 * SDL.hintPriority
 */
static const CommonEnum HintPriority[] = {
	{ "Default",		SDL_HINT_DEFAULT	},
	{ "Normal",		SDL_HINT_NORMAL		},
	{ "Override",		SDL_HINT_OVERRIDE	},
	{ NULL,			-1			}
};

static const luaL_Reg functions[] = {
	{ "init",		l_init			},
	{ "initSubSystem",	l_initSubSystem		},
	{ "quit",		l_quit			},
	{ "quitSubSystem",	l_quitSubSystem		},
	{ "wasInit",		l_wasInit		},
	{ "clearError",		l_clearError		},
	{ "getError",		l_getError		},
	{ "setError",		l_setError		},
	{ "clearHints",		l_clearHints		},
	{ "getHint",		l_getHint		},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "getHintBoolean",	l_getHintBoolean	},
#endif
	{ "setHint",		l_setHint		},
	{ NULL,			NULL			}
};

static const struct {
	const luaL_Reg *functions;
} libraries[] = {
	{ CpuFunctions					},
	{ FilesystemFunctions				},
	{ LoggingFunctions				},
	{ PlatformFunctions				},
	{ PowerFunctions				},
	{ RWOpsFunctions				},

	/* Thread and mutexes */
	{ ThreadFunctions				},
	{ ChannelFunctions				},

	/* Event group */
	{ GamectlFunctions				},
	{ JoystickFunctions				},
	{ KeyboardFunctions				},
	{ MouseFunctions				},
	{ EventFunctions				},

	/* Haptic */
	{ HapticFunctions				},

	/* Video */
	{ ClipboardFunctions				},
	{ DisplayFunctions				},
	{ RectangleFunctions				},
	{ RendererFunctions				},
	{ SurfaceFunctions				},
	{ WindowFunctions				},

	/* Audio */
	{ AudioFunctions				},

	/* Timer */
	{ TimerFunctions				},

	/* OpenGL */
	{ GlFunctions					},
	{ NULL						}
};

static const struct {
	const char	*name;
	const CommonEnum *values;
} enums[] = {
	/* Logging */
	{ "logCategory",	LoggingCategory			},
	{ "logPriority",	LoggingPriority			},

	/* Power management */
	{ "powerState",		PowerState			},

	/* Hints */
	{ "hintPriority",	HintPriority			},

	/* RW operations */
	{ "rwopsType",		RWOpsType			},
	{ "rwopsSeek",		RWOpsSeek			},

	/* Joystick values */
	{ "joyHat",		EventJoyHat			},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "joystickPowerLevel",	JoystickPowerLevels		},
#endif

	/* Game Controller values */
	{ "controllerAxis",	GameCtlAxis			},
	{ "controllerButton",	GameCtlButton			},

	/* Keyboard */
	{ "key",		KeyboardCodes			},
	{ "keymod",		KeyboardModifiers		},
	{ "scancode",		KeyboardScancodes		},

	/* Window */
	{ "window",		WindowFlags			},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "hitTestResult",	HitTestResults			},
#endif

	/* General */
	{ "flags",		InitFlags			},

	{ "mouseButton",	MouseButtons			},
	{ "mouseMask",		MouseMask			},
#if SDL_VERSION_ATLEAST(2, 0, 2)
	{ "mouseClick",		MouseClick			},
#endif
	{ "event",		EventType			},
	{ "eventAction",	EventAction			},
	{ "eventWindow",	EventWindow			},

	/* Audio */
	{ "audioFormat",	AudioFormat			},
	{ "audioStatus",	AudioStatus			},

	/* Video */
	{ "pixelFormat",	PixelFormat			},
	{ "blendMode",		BlendMode			},

	/* Renderer */
	{ "rendererFlags",	RendererFlags			},
	{ "rendererFlip",	RendererFlip			},

	/* Texture */
	{ "textureAccess",	TextureAccess			},
	{ "textureModulate",	TextureModulate			},

	/* Haptic */
	{ "hapticType",		HapticType			},
	{ "hapticDirection",	HapticDirection			},

	/* OpenGL */
	{ "glAttr",		GlAttr				},
	{ "glProfile",		GlProfile			},
	{ "glFlags",		GlContextFlags			},
	{ NULL,			NULL				}
};

static const struct {
	const CommonObject *object;
} objects[] = {
	{ &EventFilter						},
	{ &GameCtl						},
	{ &Joystick						},
	{ &Renderer						},
	{ &Surface						},
	{ &Texture						},
	{ &Window						},
	{ &RWOps						},
	{ &Thread						},
	{ &ChannelObject					},
	{ &AudioObject						},
	{ &Haptic						},
	{ &TimerObject						},
	{ &GlObject						},
	{ NULL							}
};

int EXPORT
luaopen_SDL(lua_State *L)
{
	int i;
	SDL_version ver;

	/* General functions */
	commonNewLibrary(L, functions);

	/* Library categories */
	for (i = 0; libraries[i].functions != NULL; ++i)
		commonBindLibrary(L, libraries[i].functions);

	/* Enumerations */
	for (i = 0; enums[i].values != NULL; ++i)
		commonBindEnum(L, -1, enums[i].name, enums[i].values);

	/* Object oriented data */
	for (i = 0; objects[i].object != NULL; ++i)
		commonBindObject(L, objects[i].object);

	/* Store the version */
	SDL_GetVersion(&ver);

	tableSetInt(L, -1, "VERSION_MAJOR", ver.major);
	tableSetInt(L, -1, "VERSION_MINOR", ver.minor);
	tableSetInt(L, -1, "VERSION_PATCH", ver.patch);

	tableSetInt(L, -1, "VERSION_BINDING", VERSION_BINDING);
	tableSetInt(L, -1, "VERSION_BINDING_PATCH", VERSION_BINDING_PATCH);

	if (ChannelMutex == NULL && (ChannelMutex = SDL_CreateMutex()) == NULL)
		return luaL_error(L, SDL_GetError());

	return 1;
}
