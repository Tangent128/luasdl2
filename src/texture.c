/*
 * texture.c -- textures management
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

#include <common/table.h>
#include <common/surface.h>
#include <common/video.h>

#include "texture.h"

/* --------------------------------------------------------
 * Texture object methods
 * -------------------------------------------------------- */

/*
 * Texture:getAlphaMod()
 *
 * Returns:
 *	The alpha mod integer or nil on failure
 *	The error message
 */
static int
l_texture_getAlphaMod(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);
	Uint8 alpha;

	if (SDL_GetTextureAlphaMod(tex, &alpha) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", alpha);
}

/*
 * Texture:getBlendMode()
 *
 * Returns:
 *	The blend mod integer or nil on failure (SDL.blendMode)
 *	The error message
 */
static int
l_texture_getBlendMode(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);
	SDL_BlendMode mode;

	if (SDL_GetTextureBlendMode(tex, &mode) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", mode);
}

/*
 * Texture:getColorMod()
 *
 * Returns:
 *	The color (hexadecimal) or nil
 *	The color (table) or nil
 *	The error message
 */
static int
l_texture_getColorMod(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);
	SDL_Color c;
	Uint32 hex;

	if (SDL_GetTextureColorMod(tex, &c.r, &c.g, &c.b) < 0)
		return commonPushSDLError(L, 2);

	hex = (c.r << 16) | (c.g << 8) | c.b;

	commonPush(L, "i", hex);
	videoPushColorRGB(L, &c);

	return 2;
}

static int
l_texture_lock(lua_State *L)
{
	/* XXX */

	(void)L;

	return 0;
}

/*
 * Texture:query()
 *
 * Returns:
 *	The format (SDL.pixelFormat) or nil
 *	The access (SDL.textureAccess) or nil
 *	The width or nil
 *	The height or nil
 *	The error message
 */
static int
l_texture_query(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);
	Uint32 format;
	int access, w, h;

	if (SDL_QueryTexture(tex, &format, &access, &w, &h) < 0)
		return commonPushSDLError(L, 4);

	return commonPush(L, "iiii", format, access, w, h);
}

/*
 * Texture:setAlphaMod(value)
 *
 * Arguments:
 *	value the value
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_texture_setAlphaMod(lua_State *L)
{
	SDL_Texture *tex	= commonGetAs(L, 1, TextureName, SDL_Texture *);
	Uint8 alpha		= luaL_checkinteger(L, 2);

	if (SDL_SetTextureAlphaMod(tex, alpha) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Texture:setBlendMode(value)
 *
 * Arguments:
 *	value the value (SDL.blendMode)
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_texture_setBlendMode(lua_State *L)
{
	SDL_Texture *tex	= commonGetAs(L, 1, TextureName, SDL_Texture *);
	SDL_BlendMode mode	= luaL_checkinteger(L, 2);

	if (SDL_SetTextureBlendMode(tex, mode) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Texture:setColorMod(value)
 *
 * Arguments:
 *	value the color
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_texture_setColorMod(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);
	SDL_Color c;

	c = videoGetColorRGB(L, 2);

	if (SDL_SetTextureColorMod(tex, c.r, c.g, c.b) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Texture:unlock()
 */
static int
l_texture_unlock(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);

	SDL_UnlockTexture(tex);

	return 0;
}

static int
l_texture_update(lua_State *L)
{
	/* XXX: IMPLEMENT LATER */

	(void)L;

	return 0;
}

/* --------------------------------------------------------
 * Texture object metamethods
 * -------------------------------------------------------- */

/*
 * Texture:__eq()
 */
static int
l_texture_eq(lua_State *L)
{
	SDL_Texture *o1 = commonGetAs(L, 1, TextureName, SDL_Texture *);
	SDL_Texture *o2 = commonGetAs(L, 1, TextureName, SDL_Texture *);

	return commonPush(L, "b", o1 == o2);
}

/*
 * Texture:__gc()
 */
static int
l_texture_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, TextureName);

	if (udata->mustdelete)
		SDL_DestroyTexture(udata->data);

	return 0;
}

/*
 * Texture:__tostring()
 */
static int
l_texture_tostring(lua_State *L)
{
	SDL_Texture *tex = commonGetAs(L, 1, TextureName, SDL_Texture *);
	Uint32 format;
	int access, w, h;

	if (SDL_QueryTexture(tex, &format, &access, &w, &h) < 0)
		return commonPush(L, "s", SDL_GetError());

	lua_pushfstring(L, "texture: format %d, access %d, w %d, h %d",
	    format, access, w, h);

	return 1;
}

/* --------------------------------------------------------
 * Texture object definition
 * -------------------------------------------------------- */

const luaL_Reg TextureMethods[] = {
	{ "getAlphaMod",		l_texture_getAlphaMod		},
	{ "getBlendMode",		l_texture_getBlendMode		},
	{ "getColorMod",		l_texture_getColorMod		},
	{ "lock",			l_texture_lock			},
	{ "query",			l_texture_query			},
	{ "setAlphaMod",		l_texture_setAlphaMod		},
	{ "setBlendMode",		l_texture_setBlendMode		},
	{ "setColorMod",		l_texture_setColorMod		},
	{ "unlock",			l_texture_unlock		},
	{ "update",			l_texture_update		},
	{ NULL,				NULL				}
};

const luaL_Reg TextureMetamethods[] = {
	{ "__eq",			l_texture_eq			},
	{ "__gc",			l_texture_gc			},
	{ "__tostring",			l_texture_tostring		},
	{ NULL,				NULL				}
};

const CommonObject Texture = {
	"Texture",
	TextureMethods,
	TextureMetamethods
};

/*
 * SDL.textureAccess
 */
const CommonEnum TextureAccess[] = {
	{ "Static",			SDL_TEXTUREACCESS_STATIC	},
	{ "Streaming",			SDL_TEXTUREACCESS_STREAMING	},
	{ "Target",			SDL_TEXTUREACCESS_TARGET	},
	{ NULL,				-1				}
};

/*
 * SDL.textureModulate
 */
const CommonEnum TextureModulate[] = {
	{ "None",			SDL_TEXTUREMODULATE_NONE	},
	{ "Color",			SDL_TEXTUREMODULATE_COLOR	},
	{ "Alpha",			SDL_TEXTUREMODULATE_ALPHA	},
	{ NULL,				-1				}
};
