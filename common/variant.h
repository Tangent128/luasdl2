/*
 * variant.h -- a Lua variant value
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

#ifndef _VARIANT_H_
#define _VARIANT_H_

#include <sys/queue.h>

#include <common/common.h>

typedef struct variant_pair {
	struct variant	*key;
	struct variant	*value;

	STAILQ_ENTRY(variant_pair) link;
} VariantPair;

typedef STAILQ_HEAD(variant_pairs, variant_pair) VariantPairs;

typedef struct variant {
	int type;

	union {
		char		 boolean;
		lua_Number	 number;
		VariantPairs	 table;

		struct {
			char	*data;
			int	 length;
		} string;
	} data;

	/* Link for list */
	STAILQ_ENTRY(variant)	 link;
} Variant;

typedef STAILQ_HEAD(variant_queue, variant) VariantQueue;

Variant *
variantGet(lua_State *L, int index);

void
variantPush(lua_State *L, const Variant *v);

void
variantFree(Variant *v);

#endif /* !_VARIANT_H_ */
