/*
 * clipboard.c -- desktop clipboard manager
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

#include "clipboard.h"

/*
 * SDL.getClipboardText()
 *
 * Returns:
 *	The clipboard text
 */
static int
l_video_getClipboardText(lua_State *L)
{
	char *str;

	str = SDL_GetClipboardText();
	lua_pushstring(L, str);
	SDL_free(str);

	return 1;
}

/*
 * SDL.hasClipboardText()
 *
 * Returns:
 *	True if has clipboard text
 */
static int
l_video_hasClipboardText(lua_State *L)
{
	return commonPush(L, "b", SDL_HasClipboardText());
}

/*
 * SDL.setClipboardText(text)
 *
 * Arguments:
 *	text the text
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_video_setClipboardText(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);

	if (SDL_SetClipboardText(text) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

const luaL_Reg ClipboardFunctions[] = {
	{ "getClipboardText",		l_video_getClipboardText	},
	{ "hasClipboardText",		l_video_hasClipboardText	},
	{ "setClipboardText",		l_video_setClipboardText	},
	{ NULL,				NULL				}
};
