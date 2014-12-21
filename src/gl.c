/*
 * gl.c -- OpenGL management
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

#include "gl.h"
#include "window.h"

/* --------------------------------------------------------
 * SDL_GLContext object
 * -------------------------------------------------------- */

/*
 * For the moment, dummy object.
 */
const CommonObject GlObject = {
	"GlContext",
	NULL,
	NULL
};

/* --------------------------------------------------------
 * GL Functions
 * -------------------------------------------------------- */

/*
 * SDL.glExtensionSupported(name)
 *
 * Arguments:
 *	name the extension requested
 *
 * Returns:
 *	True if supported
 */
static int
l_glExtensionSupported(lua_State *L)
{
	return commonPush(L, "b", SDL_GL_ExtensionSupported(luaL_checkstring(L, 1)));
}

/*
 * SDL.glSetAttribute(attribute, value)
 *
 * Arguments:
 *	attribute the attribute to set
 *	value the value
 *
 * Returns:
 *	True on success or nil on error
 *	The error message
 */
static int
l_glSetAttribute(lua_State *L)
{
	int attr	= luaL_checkinteger(L, 1);
	int value	= luaL_checkinteger(L, 2);

	if (SDL_GL_SetAttribute(attr, value) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.glGetAttribute(attribute)
 *
 * Arguments:
 *	attribute the attribute to get
 *
 * Returns:
 *	The value or nil on failure
 *	The error message
 */
static int
l_glGetAttribute(lua_State *L)
{
	int attr = luaL_checkinteger(L, 1);
	int value;

	if (SDL_GL_GetAttribute(attr, &value) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", value);
}

/*
 * SDL.glCreateContext(window)
 *
 * Arguments:
 *	window the window
 *
 * Returns:
 *	The context object or nil on failure
 *	The error message
 */
static int
l_glCreateContext(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	SDL_GLContext ctx;

	if ((ctx = SDL_GL_CreateContext(w)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", GlName, ctx);
}

/*
 * SDL.glMakeCurent(window, context)
 *
 * Arguments:
 *	window the window
 *	context the context
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_glMakeCurrent(lua_State *L)
{
	SDL_Window *w		= commonGetAs(L, 1, WindowName, SDL_Window *);
	SDL_GLContext ctx	= commonGetAs(L, 2, GlName, SDL_GLContext);

	if (SDL_GL_MakeCurrent(w, ctx) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.glGetCurrentWindow()
 *
 * Returns:
 *	The current window or nil on failure
 *	The error message
 */
static int
l_glGetCurrentWindow(lua_State *L)
{
	SDL_Window *w = SDL_GL_GetCurrentWindow();

	if (w == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", WindowName, w);
}

/*
 * SDL.glGetCurrentContext()
 *
 * Returns:
 *	The current context or nil on failure
 *	The error message
 */
static int
l_glGetCurrentContext(lua_State *L)
{
	SDL_GLContext c = SDL_GL_GetCurrentContext();	

	if (c == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", GlName, c);
}

/*
 * SDL.glGetDrawableSize(window)
 *
 * Arguments:
 *	window the window
 *
 * Returns:
 *	The width
 *	The height
 */
static int
l_glGetDrawableSize(lua_State *L)
{
	SDL_Window *w = commonGetAs(L, 1, WindowName, SDL_Window *);
	int width, height;

	SDL_GL_GetDrawableSize(w, &width, &height);

	return commonPush(L, "ii", width, height);
}

/*
 * SDL.glSetSwapInterval(interval)
 *
 * Arguments:
 *	interval (optional) the interval, default: -1
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_glSetSwapInterval(lua_State *L)
{
	int interval = luaL_optinteger(L, 1, -1);

	if (SDL_GL_SetSwapInterval(interval) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.glGetSwapInterval()
 *
 * Returns:
 *	The swap interval or 0
 */
static int
l_glGetSwapInterval(lua_State *L)
{
	return commonPush(L, "i", SDL_GL_GetSwapInterval());
}

/*
 * SDL.glSwapWindow(window)
 *
 * Arguments:
 *	window the window
 */
static int
l_glSwapWindow(lua_State *L)
{
	SDL_GL_SwapWindow(commonGetAs(L, 1, WindowName, SDL_Window *));

	return 0;
}

/*
 * SDL.glDeleteContext(context)
 *
 * Arguments:
 *	context the context
 */
static int
l_glDeleteContext(lua_State *L)
{
	SDL_GL_DeleteContext(commonGetAs(L, 1, GlName, SDL_GLContext));

	return 0;
}

const luaL_Reg GlFunctions[] = {
	{ "glExtensionSupported",	l_glExtensionSupported			},
	{ "glSetAttribute",		l_glSetAttribute			},
	{ "glGetAttribute",		l_glGetAttribute			},
	{ "glCreateContext",		l_glCreateContext			},
	{ "glMakeCurrent",		l_glMakeCurrent				},
	{ "glGetCurrentWindow",		l_glGetCurrentWindow			},
	{ "glGetCurrentContext",	l_glGetCurrentContext			},
	{ "glGetDrawableSize",		l_glGetDrawableSize			},
	{ "glSetSwapInterval",		l_glSetSwapInterval			},
	{ "glGetSwapInterval",		l_glGetSwapInterval			},
	{ "glSwapWindow",		l_glSwapWindow				},
	{ "glDeleteContext",		l_glDeleteContext			},
	{ NULL,				NULL					}
};

/*
 * SDL.glAttr
 */
const CommonEnum GlAttr[] = {
	{ "RedSize",			SDL_GL_RED_SIZE				},
	{ "GreenSize",			SDL_GL_GREEN_SIZE			},
	{ "BlueSize",			SDL_GL_BLUE_SIZE			},
	{ "AlphaSize",			SDL_GL_ALPHA_SIZE			},
	{ "BufferSize",			SDL_GL_BUFFER_SIZE			},
	{ "DoubleBuffer",		SDL_GL_DOUBLEBUFFER			},
	{ "DepthSize",			SDL_GL_DEPTH_SIZE			},
	{ "StencilSize",		SDL_GL_STENCIL_SIZE			},
	{ "AccumRedSize",		SDL_GL_ACCUM_RED_SIZE			},
	{ "AccumGreenSize",		SDL_GL_ACCUM_GREEN_SIZE			},
	{ "AccumBlueSize",		SDL_GL_ACCUM_BLUE_SIZE			},
	{ "AccumAlphaSize",		SDL_GL_ACCUM_ALPHA_SIZE			},
	{ "Stereo",			SDL_GL_STEREO				},
	{ "MultiSampleBuffers",		SDL_GL_MULTISAMPLEBUFFERS		},
	{ "MultiSampleSamples",		SDL_GL_MULTISAMPLESAMPLES		},
	{ "AcceleratedVisual",		SDL_GL_ACCELERATED_VISUAL		},
	{ "RetainedBacking",		SDL_GL_RETAINED_BACKING			},
	{ "ContextMajorVersion",	SDL_GL_CONTEXT_MAJOR_VERSION		},
	{ "ContextMinorVersion",	SDL_GL_CONTEXT_MINOR_VERSION		},
	{ "ContextEGL",			SDL_GL_CONTEXT_EGL			},
	{ "ContextFlags",		SDL_GL_CONTEXT_FLAGS			},
	{ "ContextProfileMask",		SDL_GL_CONTEXT_PROFILE_MASK		},
	{ "ShareWithCurrentContext",	SDL_GL_SHARE_WITH_CURRENT_CONTEXT	},
	{ "FramebufferSRGBCapable",	SDL_GL_FRAMEBUFFER_SRGB_CAPABLE		},
	{ NULL,				-1					}
};

/*
 * SDL.glProfile
 */
const CommonEnum GlProfile[] = {
	{ "Core",			SDL_GL_CONTEXT_PROFILE_CORE		},
	{ "Compatibility",		SDL_GL_CONTEXT_PROFILE_COMPATIBILITY	},
	{ "ES",				SDL_GL_CONTEXT_PROFILE_ES		},
	{ NULL,				-1					}
};

/*
 * SDL.glFlags
 */
const CommonEnum GlContextFlags[] = {
	{ "Debug",			SDL_GL_CONTEXT_DEBUG_FLAG		},
	{ "ForwardCompatible",		SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG	},
	{ "RobustAccess",		SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG	},
	{ "ResetIsolation",		SDL_GL_CONTEXT_RESET_ISOLATION_FLAG	},
	{ NULL,				-1					},
};
