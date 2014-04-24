/*
 * array.h -- manipulate dynamic arrays
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

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stddef.h>
#include <stdarg.h>

#ifndef ARRAY_DEFAULT_CHKSIZE
#define ARRAY_DEFAULT_CHKSIZE	128
#endif

typedef enum {
	ARRAY_AUTO		= 0,		/* array grows automatically */
	ARRAY_FIXED		= (1 << 0),	/* fixed size length */
	ARRAY_FASTREMOVE	= (1 << 1),	/* use last object when removing */
	ARRAY_CLEARBITS		= (1 << 2),	/* clear data when inserting/removing */
	ARRAY_INSERTSAFE	= (1 << 3)	/* insertion must have valid indexes */
} ArrayFlags;

typedef struct {
	int			flags;		/* (ro) array flags (default AUTO) */
	void			*data;		/* (rw) array of data */
	int			length;		/* (ro) number of element inside */
	size_t			size;		/* (ro) current buffer size (allocated memory) */
	size_t			unit;		/* (ro) unit size (sizeof the object) */
	int			chksize;	/* (rw) chunk size (used when growing array) */
} Array;

typedef void	(*ArrayMap)(void *, void *);
typedef int	(*ArrayCmp)(const void *, const void *);

#ifdef __cplusplus
extern "C" {
#endif

int
arrayInit(Array *, size_t, size_t);

int
arrayPush(Array *, const void *);

int
arrayInsert(Array *, const void *, int);

int
arrayAppend(Array *, const void *);

void
arrayPop(Array *);

void
arrayUnqueue(Array *);

void
arrayRemovei(Array *, int);

void
arrayRemovep(Array *, const void *);

int
arraySwapi(Array *, int, int);

int
arraySwapp(Array *, const void *, const void *);

void
arrayMap(const Array *, ArrayMap, void *);

void
arraySort(Array *, ArrayCmp);

int
arrayFind(const Array *, ArrayCmp, void *, void *);

void *
arrayFirst(const Array *);

void *
arrayGet(const Array *, int);

void *
arrayLast(const Array *);

void
arrayClear(Array *);

void
arrayFree(Array *);

void *
ArrayTrim(Array *);
	
#define ARRAY_FOREACH(a, var, i)					\
	for (i = 0, (var) = arrayFirst((a));				\
		i < (a)->length;					\
		(var) = arrayGet(a, ++i))

#ifdef __cplusplus
}
#endif

#endif /* _ARRAY_H_ */
