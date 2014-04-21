/*
 * ttf.c -- main SDL_ttf (2.0) module
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
 
#include <SDL_ttf.h>

#include <common/array.h>
#include <common/common.h>
#include <common/rwops.h>
#include <common/surface.h>
#include <common/table.h>
#include <common/video.h>

/* ---------------------------------------------------------
 * TTF_Font object
 * --------------------------------------------------------- */

#define FontName	Font.name

static const CommonObject Font;

static const CommonEnum FontStyle[] = {
	{ "Bold",			TTF_STYLE_BOLD		},
	{ "Italic",			TTF_STYLE_ITALIC	},
	{ "Underline",			TTF_STYLE_UNDERLINE	},
	{ "StrikeThrough",		TTF_STYLE_STRIKETHROUGH	},
	{ NULL,				-1			}
};

static const CommonEnum FontHinting[] = {
	{ "Normal",			TTF_HINTING_NORMAL	},
	{ "Light",			TTF_HINTING_LIGHT	},
	{ "Mono",			TTF_HINTING_MONO	},
	{ "None",			TTF_HINTING_NONE	},
	{ NULL,				-1			}
};

enum TextType {
	Ascii,
	Utf8,
	Unicode
};

static int
getUnicode(lua_State *L, int index, Array *array)
{
	luaL_checktype(L, index, LUA_TTABLE);

	if (arrayInit(array, sizeof (Uint16), 32) < 0)
		return commonPushErrno(L, 2);

	if (index < 0)
		--index;

	lua_pushnil(L);
	while (lua_next(L, index)) {
		int ch = lua_tointeger(L, -1);

		if (arrayAppend(array, &ch) < 0)
			return commonPushErrno(L, 2);

		lua_pop(L, 1);
	}

	return 0;
}

static int
fontSize(lua_State *L, enum TextType type)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int w, h, result = -1;

	switch (type) {
	case Ascii:
	case Utf8:
	{
		const char *text = luaL_checkstring(L, 2);
		if (type == Ascii)
			result = TTF_SizeText(f, text, &w, &h);
		else
			result = TTF_SizeUTF8(f, text, &w, &h);
	}
		break;
	case Unicode:
	{
		Array array;

		if (getUnicode(L, 2, &array) != 0)
			return 2;

		result = TTF_SizeUNICODE(f, array.data, &w, &h);
		arrayFree(&array);
	}
		break;
	default:
		return luaL_error(L, "invalid type: %d", type);
	}

	if (result < 0)
		return commonPushSDLError(L, 2);

	return commonPush(L, "ii", w, h);
}

static int
fontRender(lua_State *L, enum TextType type)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	const char *style = luaL_checkstring(L, 3);
	SDL_Color fg, bg;
	SDL_Surface *s = NULL;

	union {
		const char *text;
		Array array;
	} data;

	/* foreground color is mandatory */
	fg = videoGetColorRGB(L, 4);

	/* background is only for "shaded" */
	if (strcmp(style, "shaded") == 0)
		bg = videoGetColorRGB(L, 5);

	switch (type) {
	case Ascii:
	case Utf8:
		data.text = luaL_checkstring(L, 2);
		break;
	case Unicode:
		if (getUnicode(L, 2, &data.array) != 0)
			return 2;
		break;
	default:
		return luaL_error(L, "invalid type: %d", type);
	}

	if (strcmp(style, "solid") == 0) {
		if (type == Ascii)
			s = TTF_RenderText_Solid(f, data.text, fg);
		else if (type == Utf8)
			s = TTF_RenderUTF8_Solid(f, data.text, fg);
		else
			s = TTF_RenderUNICODE_Solid(f, data.array.data, fg);
	} else if (strcmp(style, "blended") == 0) {
		if (type == Ascii)
			s = TTF_RenderText_Blended(f, data.text, fg);
		else if (type == Utf8)
			s = TTF_RenderUTF8_Blended(f, data.text, fg);
		else
			s = TTF_RenderUNICODE_Blended(f, data.array.data, fg);
	} else if (strcmp(style, "shaded") == 0) {
		if (type == Ascii)
			s = TTF_RenderText_Shaded(f, data.text, fg, bg);
		else if (type == Utf8)
			s = TTF_RenderUTF8_Shaded(f, data.text, fg, bg);
		else
			s = TTF_RenderUNICODE_Shaded(f, data.array.data, fg, bg);
	}

	if (type == Unicode)
		arrayFree(&data.array);
	if (s == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, s);
}

static int
l_font_getStyle(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int style = TTF_GetFontStyle(f);

	if (style == TTF_STYLE_NORMAL)
		return commonPush(L, "n");

	commonPushEnum(L, style, FontStyle);

	return 1;
}

static int
l_font_setStyle(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int style = commonGetEnum(L, 2);

	TTF_SetFontStyle(f, style);

	return 0;
}

static int
l_font_getOutline(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_GetFontOutline(f));
}

static int
l_font_setOutline(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int outline = luaL_checkinteger(L, 2);

	TTF_SetFontOutline(f, outline);

	return 0;
}

static int
l_font_getHinting(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_GetFontOutline(f));
}

static int
l_font_setHinting(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int hinting = luaL_checkinteger(L, 2);

	TTF_SetFontHinting(f, hinting);

	return 0;
}

static int
l_font_getKerning(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "b", TTF_GetFontKerning(f) > 0);
}

static int
l_font_setKerning(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int enable = lua_toboolean(L, 2);

	TTF_SetFontKerning(f, enable);

	return 0;
}

static int
l_font_height(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_FontHeight(f));
}

static int
l_font_ascent(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_FontAscent(f));
}

static int
l_font_descent(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_FontDescent(f));
}

static int
l_font_lineSkip(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_FontLineSkip(f));
}

static int
l_font_faces(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", (int)TTF_FontFaces(f));
}

static int
l_font_faceIsFixedWidth(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "i", TTF_FontFaceIsFixedWidth(f));
}

static int
l_font_faceFamilyName(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "s", TTF_FontFaceFamilyName(f));
}

static int
l_font_faceStyleName(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);

	return commonPush(L, "s", TTF_FontFaceStyleName(f));
}

static int
l_font_glyphIsProvided(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int ch = luaL_checkinteger(L, 2);

	return commonPush(L, "i", TTF_GlyphIsProvided(f, ch));
}

static int
l_font_glyphMetrics(lua_State *L)
{
	TTF_Font *f = commonGetAs(L, 1, FontName, TTF_Font *);
	int ch = luaL_checkinteger(L, 2);
	int minx, maxx, miny, maxy, advance;

	if (TTF_GlyphMetrics(f, ch, &minx, &maxx, &miny, &maxy, &advance) < 0)
		return commonPushSDLError(L, 1);

	lua_createtable(L, 0, 5);
	tableSetInt(L, -1, "minx", minx);
	tableSetInt(L, -1, "maxx", maxx);
	tableSetInt(L, -1, "miny", miny);
	tableSetInt(L, -1, "maxy", maxy);
	tableSetInt(L, -1, "advance", advance);

	return 1;
}

static int
l_font_sizeText(lua_State *L)
{
	return fontSize(L, Ascii);
}

static int
l_font_sizeUTF8(lua_State *L)
{
	return fontSize(L, Utf8);
}

static int
l_font_sizeUnicode(lua_State *L)
{
	return fontSize(L, Unicode);
}

static int
l_font_renderText(lua_State *L)
{
	return fontRender(L, Ascii);
}

static int
l_font_renderUtf8(lua_State *L)
{
	return fontRender(L, Utf8);
}

static int
l_font_renderUnicode(lua_State *L)
{
	return fontRender(L, Unicode);
}

static int
l_font_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, FontName);

	if (udata->mustdelete)
		TTF_CloseFont(udata->data);

	return 0;
}

static const luaL_Reg FontMethods[] = {
	{ "getStyle",			l_font_getStyle		},
	{ "setStyle",			l_font_setStyle		},
	{ "getOutline",			l_font_getOutline	},
	{ "setOutline",			l_font_setOutline	},
	{ "getHinting",			l_font_getHinting	},
	{ "setHinting",			l_font_setHinting	},
	{ "getKerning",			l_font_getKerning	},
	{ "setKerning",			l_font_setKerning	},
	{ "height",			l_font_height		},
	{ "ascent",			l_font_ascent		},
	{ "descent",			l_font_descent		},
	{ "lineSkip",			l_font_lineSkip		},
	{ "faces",			l_font_faces		},
	{ "faceIsFixedWidth",		l_font_faceIsFixedWidth	},
	{ "faceFamilyName",		l_font_faceFamilyName	},
	{ "faceStyleName",		l_font_faceStyleName	},
	{ "glyphIsProvided",		l_font_glyphIsProvided	},
	{ "glyphMetrics",		l_font_glyphMetrics	},
	{ "sizeText",			l_font_sizeText		},
	{ "sizeUTF8",			l_font_sizeUTF8		},
	{ "sizeUnicode",		l_font_sizeUnicode	},
	{ "renderText",			l_font_renderText	},
	{ "renderUtf8",			l_font_renderUtf8	},
	{ "renderUnicode",		l_font_renderUnicode	},
	{ NULL,				NULL			}
};

static const luaL_Reg FontMetamethods[] = {
	{ "__gc",			l_font_gc		},
	{ NULL,				NULL			}
};

static const CommonObject Font = {
	"Font",
	FontMethods,
	FontMetamethods
};

/* ---------------------------------------------------------
 * SDL_ttf functions
 * --------------------------------------------------------- */

static int
l_init(lua_State *L)
{
	if (TTF_Init() < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static int
l_open(lua_State *L)
{
	int ptsize = luaL_checkinteger(L, 2);
	TTF_Font *f;

	if (lua_type(L, 1) == LUA_TSTRING)
		f = TTF_OpenFont(lua_tostring(L, 1), ptsize);
	else if (lua_type(L, 1) == LUA_TUSERDATA) {
		SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);

		f = TTF_OpenFontRW(ops, 0, ptsize);
	} else
		return luaL_error(L, "expected a string or a RWops");

	if (f == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", FontName, f);
}

static int
l_quit(lua_State *L)
{
	TTF_Quit();

	(void)L;

	return 0;
}

static const luaL_Reg functions[] = {
	{ "init",			l_init				},
	{ "open",			l_open				},
	{ "quit",			l_quit				},
	{ NULL,				NULL				}
};

int EXPORT
luaopen_SDL_ttf(lua_State *L)
{
	luaL_newlib(L, functions);

	commonBindEnum(L, -1, "style", FontStyle);
	commonBindEnum(L, -1, "hinting", FontHinting);

	/* Font object */
	commonBindObject(L, &Font);

	return 1;
}
