/*
 * image.c -- main SDL_image (2.0) module
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

#include <SDL_image.h>

#include <common/common.h>
#include <common/rwops.h>
#include <common/surface.h>

typedef SDL_Surface *	(*LoadFunction)(SDL_RWops *);
typedef int		(*TypeFunction)(SDL_RWops *);

/*
 * Image.flags
 */
static const CommonEnum ImageFlags[] = {
	{ "JPG",			IMG_INIT_JPG			},
	{ "PNG",			IMG_INIT_PNG			},
	{ "TIF",			IMG_INIT_TIF,			},
	{ NULL,				-1				}
};

static const struct ImgInfo {
	const char	*name;
	LoadFunction	 load;
	TypeFunction	 type;
} loaders[] = {
	{ "CUR",	IMG_LoadCUR_RW,	IMG_isCUR	},
	{ "ICO",	IMG_LoadICO_RW,	IMG_isICO	},
	{ "BMP",	IMG_LoadBMP_RW,	IMG_isBMP	},
	{ "PNM",	IMG_LoadPNM_RW,	IMG_isPNM	},
	{ "XPM",	IMG_LoadXPM_RW,	IMG_isXPM	},
	{ "XCF",	IMG_LoadXCF_RW,	IMG_isXCF	},
	{ "PCX",	IMG_LoadPCX_RW,	IMG_isPCX	},
	{ "GIF",	IMG_LoadGIF_RW,	IMG_isGIF	},
	{ "JPG",	IMG_LoadJPG_RW,	IMG_isJPG	},
	{ "TIF",	IMG_LoadTIF_RW,	IMG_isTIF	},
	{ "PNG",	IMG_LoadPNG_RW,	IMG_isPNG	},
	{ "TGA",	IMG_LoadTGA_RW,	NULL /* WHY? */	},
	{ "LBM",	IMG_LoadLBM_RW,	IMG_isLBM	},
	{ "XV",		IMG_LoadXV_RW,	IMG_isXV	},
	{ NULL,		NULL,		NULL		}
};

/*
 * Image.init(flags)
 *
 * Arguments:
 *	flags the formats requested
 *
 * Returns:
 *	The initialized formats
 *	Nil if some have not been supported
 *	The error message
 */
static int
l_image_init(lua_State *L)
{
	int flags = commonGetEnum(L, 1);
	int ret;

	ret = IMG_Init(flags);
	commonPushEnum(L, ret, ImageFlags);

	if ((ret & flags) != flags)
		return commonPush(L, "n s", IMG_GetError()) + 1;

	return commonPush(L, "b", 1) + 1;
}

/*
 * Image.quit()
 */
static int
l_image_quit(lua_State *L)
{
	IMG_Quit();

	(void)L;

	return 0;
}

/*
 * Image.load(path)
 *
 * Arguments:
 *	path the path to the image
 *
 * Returns:
 *	The surface or nil on failure
 *	The error message
 */
static int
l_image_load(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	SDL_Surface *surf;

	surf = IMG_Load(path);
	if (surf == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, surf);
}

/*
 * Image.load_RW(rwops, name)
 *
 * Arguments:
 *	rwops the RWops
 *	name (optional) the type name
 *
 * Returns:
 *	The surface or nil
 *	The error message
 */
static int
l_image_load_RW(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	SDL_Surface *surf;

	/* If name is specified we use a specific function */
	if (lua_gettop(L) >= 2) {
		const char *name = luaL_checkstring(L, 2);
		const struct ImgInfo *info;

		for (info = loaders; info->name != NULL; ++info) {
			if (strcmp(info->name, name) == 0) {
				surf = info->load(ops);
				break;
			}
		}

		/* Reached mean bad type requested */
		return luaL_error(L, "invalid image type %s", name);
	} else {
		surf = IMG_Load_RW(ops, 0);
	}

	if (surf == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SurfaceName, surf);
}

/*
 * Image.is(rwops, type)
 *
 * Arguments:
 *	rwops the RWops
 *	type the type
 *
 * Returns:
 *	True if the file is one requested or nil on failure
 *	The error message
 */
static int
l_image_is(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	const char *name = luaL_checkstring(L, 2);
	const struct ImgInfo *info;

	for (info = loaders; info->name != NULL; ++info)
		if (strcmp(info->name, name) == 0 && info->type != NULL)
			return commonPush(L, "b", info->type(ops));

	return commonPush(L, "ns", "invalid type given");
}

static const luaL_Reg ImageFunctions[] = {
	{ "init",			l_image_init			},
	{ "quit",			l_image_quit			},
	{ "load",			l_image_load			},
	{ "load_RW",			l_image_load_RW			},
	{ "is",				l_image_is			},
	{ NULL,				NULL				}
};

int EXPORT
luaopen_SDL_image(lua_State *L)
{
	/* New SDL.image library */
	commonNewLibrary(L, ImageFunctions);

	/* Flags for IMG_Init() */
	commonBindEnum(L, -1, "flags", ImageFlags);

	return 1;
}
