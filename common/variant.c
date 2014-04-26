/*
 * variant.c -- a Lua variant value
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

#include <stddef.h>
#include <string.h>

#include "variant.h"

Variant *
variantGet(lua_State *L, int index)
{
	Variant *v;
	int type;

	if ((type = lua_type(L, index)) == LUA_TNIL)
		return NULL;

	if ((v = calloc(1, sizeof (Variant))) == NULL)
		return NULL;

	v->type = type;
	switch (v->type) {
	case LUA_TNUMBER:
		v->data.number = lua_tonumber(L, index);
		break;
	case LUA_TSTRING:
	{
		const char *str;
		size_t length;

		str = lua_tolstring(L, index, &length);
		if ((v->data.string.data = malloc(length)) == NULL) {
			free(v);
			return NULL;
		}

		/* Copy the string which may have embedded '\0' */
		v->data.string.length = length;
		memcpy(v->data.string.data, str, length);
	}
		break;
	case LUA_TBOOLEAN:
		v->data.boolean = lua_toboolean(L, index);
		break;
	case LUA_TTABLE:
	{
		STAILQ_INIT(&v->data.table);

		if (index < 0)
			-- index;

		lua_pushnil(L);
		while (lua_next(L, index)) {
			VariantPair *pair = malloc(sizeof (VariantPair));

			if (pair == NULL) {
				lua_pop(L, 1);
				variantFree(v);
				return NULL;
			}

			pair->key = variantGet(L, -2);
			pair->value = variantGet(L, -1);

			if (pair->key == NULL || pair->value == NULL) {
				lua_pop(L, 1);
				variantFree(pair->key);
				variantFree(pair->value);
				variantFree(v);
				free(pair);
				break;
			}

			lua_pop(L, 1);
			STAILQ_INSERT_TAIL(&v->data.table, pair, link);
		}
	}
		break;
	}

	return v;
}

void
variantPush(lua_State *L, const Variant *v)
{
	if (v == NULL)
		return;

	switch (v->type) {
	case LUA_TNUMBER:
		lua_pushnumber(L, v->data.number);
		break;
	case LUA_TSTRING:
		lua_pushlstring(L, v->data.string.data, v->data.string.length);
		break;
	case LUA_TBOOLEAN:
		lua_pushboolean(L, v->data.boolean);
		break;
	case LUA_TTABLE:
	{
		VariantPair *pair;

		lua_createtable(L, 0, 0);

		STAILQ_FOREACH(pair, &v->data.table, link) {
			variantPush(L, pair->key);
			variantPush(L, pair->value);
			lua_settable(L, -3);
		}
	}
		break;
	default:
		break;
	}
}

void
variantFree(Variant *v)
{
	VariantPair *t, *tmp;

	if (v == NULL)
		return;

	switch (v->type) {
	case LUA_TSTRING:
		free(v->data.string.data);
		break;
	case LUA_TTABLE:
		STAILQ_FOREACH_SAFE(t, &v->data.table, link, tmp) {
			variantFree(t->key);
			variantFree(t->value);
			free(t);
		}
		break;
	default:
		break;
	}

	free(v);
}
