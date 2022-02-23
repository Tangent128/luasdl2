/*
 * vulkan.c -- sdl vulkan support
 *
 * Copyright (c) 2022 Sebastien MacDougall-Landry
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

#include <common/surface.h>
#include <common/table.h>
#include <common/video.h>
#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

#include "vulkan.h"

#if SDL_VERSION_ATLEAST(2, 0, 6)

static int l_vulkan_getInstanceExtensions(lua_State *L)
{
	SDL_Window *window	= commonGetAs(L, 1, "Window", SDL_Window *);
	unsigned int pCount;
	const char **names;

	if (!SDL_Vulkan_GetInstanceExtensions(window, &pCount, NULL))
		return commonPushSDLError(L, 1);

	if (!SDL_Vulkan_GetInstanceExtensions(window, &pCount, names))
		return commonPushSDLError(L, 1);

	lua_newtable(L);
	if ( (pCount == 0) || names == NULL ) return 1;
	for (unsigned int i = 0; i < pCount; i++)
	{
		lua_pushstring(L, names[i]);
		lua_rawseti(L, -2, i+1);
	}

	return 1;
}

static int l_vulkan_createSurface(lua_State *L)
{
	SDL_Window *window	= commonGetAs(L, 1, "Window", SDL_Window *);
	VkInstance instance	= commonGetAs(L, 2, "moonvulkan_instance", VkInstance);
	VkSurfaceKHR surface;

	if (!SDL_Vulkan_CreateSurface(window, instance, &surface))
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", surface);
}

#endif

const luaL_Reg VulkanFunctions[] = {
#if SDL_VERSION_ATLEAST(2, 0, 6)
	{ "vulkanGetInstanceExtensions",			l_vulkan_getInstanceExtensions		},
	{ "vulkanCreateSurface",			l_vulkan_createSurface		},
#endif
	{ NULL,				NULL			}
};

