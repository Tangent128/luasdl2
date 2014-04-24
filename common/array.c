/*
 * array.c -- manipulate dynamic arrays
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

#define OFFSET(x)	(arr->unit * (x))

static int		grow(Array *);

int
arrayInit(Array *arr, size_t unit, size_t chksize)
{
	if (unit == 0)
		return -1;

	arr->length	= 0;
	arr->flags	= 0;
	arr->unit	= unit;
	arr->chksize	= chksize;
	arr->size	= OFFSET(arr->chksize);

	if ((arr->data = malloc(arr->size)) == NULL)
		return -1;

	if (arr->flags & ARRAY_CLEARBITS)
		memset(arr->data, 0, arr->size);

	return 0;
}

/*
 * Add to the head of array. NOTE: this may be very slow when adding a lot
 * of object (about 100000). If you need to add a lot of data please consider
 * using linked list instead. Returns -1 on failure or 0 on success.
 */
int
arrayPush(Array *arr, const void *data)
{
	if (grow(arr) < 0)
		return -1;

	memmove((char *)arr->data + arr->unit, arr->data, OFFSET(arr->length++));
	memcpy((char *)arr->data, data, arr->unit);

	return 0;
}

/*
 * Insert the data at the specified index. The function returns -1 on
 * allocation failure or the position of the added element.
 */
int
arrayInsert(Array *arr, const void *data, int index)
{
	if (arr->flags & ARRAY_INSERTSAFE)
		if (index < 0 || index > arr->length)
			return -1;

	if (index < 0)
		return arrayPush(arr, data);
	if (index >= arr->length)
		return arrayAppend(arr, data);

	/* Good place */
	memmove((char *)arr->data + OFFSET(index + 1),
	    (char *)arr->data + OFFSET(index), OFFSET(arr->length++ - index));
	memcpy((char *)arr->data + OFFSET(index), data, arr->unit);

	return index;
}

/*
 * Append the data to the end of array. Returns -1 on failure or the position
 * of the added element.
 */
int
arrayAppend(Array *arr, const void *data)
{
	if (grow(arr) < 0)
		return -1;

	memcpy((char *)arr->data + OFFSET(arr->length++), data, arr->unit);

	return (arr->length - 1);
}

/*
 * Remove the array's head.
 */
void
arrayPop(Array *arr)
{
	arrayRemovei(arr, 0);
}

/*
 * Remove the array's tail.
 */
void
arrayUnqueue(Array *arr)
{
	arrayRemovei(arr, arr->length - 1);
}

/*
 * Remove the data at the specified index. Bounds are checked.
 */
void
arrayRemovei(Array *arr, int index)
{
	if (arr->length > 0 && index >= 0 && index < arr->length) {
		if (arr->flags & ARRAY_FASTREMOVE)
			memmove((char *)arr->data + OFFSET(index),
			    (char *)arr->data + OFFSET(--arr->length),
			    arr->unit);
		else
			memmove((char *)arr->data + OFFSET(index),
			    (char *)arr->data + OFFSET(index + 1),
			    OFFSET(arr->length-- - index - 1));
	}

	if (arr->flags & ARRAY_CLEARBITS)
		memset((char *)arr->data + OFFSET(arr->length), 0, arr->unit);
}

/*
 * Remove the object referenced by the `data' argument. Useful when you
 * don't know the index.
 */
void
arrayRemovep(Array *arr, const void *data)
{
	void *elm;
	int i;

	for (i = 0; i < arr->length; ++i) {
		elm = (char *)arr->data + OFFSET(i);

		if (memcmp(elm, data, arr->unit) == 0) {
			arrayRemovei(arr, i);
			break;
		}
	}
}

/*
 * Swap the two elements referenced by index `i1' and `i2'. This function needs
 * to allocate data to swap elements thus if the functions fails it returns -1
 * otherwise 0 is returned.
 */
int
arraySwapi(Array *arr, int i1, int i2)
{
	void *tmp;

	/* Out of bounds */
	if (i1 >= arr->length || i1 < 0 || i2 >= arr->length || i2 < 0)
		return -1;

	if ((tmp = malloc(arr->unit)) == NULL)
		return -1;

	memcpy((char *)tmp, (char *)arr->data + OFFSET(i1), arr->unit);
	memcpy((char *)arr->data + OFFSET(i1), (char *)arr->data + OFFSET(i2),
	    arr->unit);
	memcpy((char *)arr->data + OFFSET(i2), (char *)tmp, arr->unit);

	/*
	 * Clear bytes for safety you probably don't want a password or
	 * secure data to be left somewhere in the memory.
	 */

	if (arr->flags & ARRAY_CLEARBITS)
		memset(tmp, 0, arr->unit);
	free(tmp);

	return 0;
}

/*
 * Swap the two elements referenced by data `o1' and `o2'. This function
 * may be slow on large arrays since it must travel all the object
 * to find the indexes.
 */
int
arraySwapp(Array *arr, const void *o1, const void *o2)
{
	int found, i1, i2;

	for (i1 = found = 0; !found && i1 < arr->length; ++i1)
		found = memcmp((char *)arr->data + OFFSET(i1), o1, arr->unit) == 0;

	if (!found)
		return -1;

	for (i2 = found = 0; !found && i2 < arr->length; ++i2)
		found = memcmp((char *)arr->data + OFFSET(i2), o2, arr->unit) == 0;

	if (!found)
		return -1;

	return arraySwapi(arr, --i1, --i2);
}

/*
 * Apply the function `fn' on each object and give the optional `udata'
 * argument to the function too.
 */
void
arrayMap(const Array *arr, ArrayMap fn, void *udata)
{
	int i;

	for (i = 0; i < arr->length; ++i)
		fn((char *)arr->data + OFFSET(i), udata);
}

/*
 * Call qsort function to sort the array.
 */
void
arraySort(Array *arr, ArrayCmp fn)
{
	qsort(arr->data, arr->length, arr->unit, fn);
}

/*
 * Compare each object with the user supplied function. If the `fn' function
 * returns 1, 1 is returned and dst points to the correct object, dst should
 * be a pointer to a pointer of object, like (int **) for a array of int.
 */
int
arrayFind(const Array *arr, ArrayCmp fn, void *dst, void *u)
{
	int st, i;

	for (i = st = 0; i < arr->length && st != 1; ++i)
		st = fn((char *)arr->data + OFFSET(i), u);

	if (st && dst)
		*(char **)dst = (char *)arr->data + OFFSET(i - 1);

	return (st) ? i - 1 : -1;
}

void *
arrayFirst(const Array *arr)
{
	return arr->data;
}

void *
arrayLast(const Array *arr)
{
	if (arr->length == 0)
		return arrayFirst(arr);

	return (char *)arr->data + OFFSET(arr->length - 1);

}

void *
arrayGet(const Array *arr, int idx)
{
	if (idx < 0)
		return arrayFirst(arr);
	if (idx >= arr->length)
		return arrayLast(arr);

	return (char *)arr->data + OFFSET(idx);
}

/*
 * Erase every bytes and set the length to 0.
 */
void
arrayClear(Array *arr)
{
	memset(arr->data, 0, arr->size);
	arr->length = 0;
}

/*
 * Same as array_clear except it also free the array object.
 */
void
arrayFree(Array *arr)
{
	if (arr->flags & ARRAY_CLEARBITS)
		arrayClear(arr);

	free(arr->data);

	arr->length	= 0;
	arr->data	= NULL;
	arr->size	= 0;
}

/*
 * Trim down the array to the correct size.
 */
void *
ArrayTrim(Array *arr)
{
	return realloc(arr->data, arr->length * arr->unit);
}

/*
 * Increate the array storage when it is full. If the buffer is fixed size
 * it returns -1 on full buffer otherwise 0 is returned if allocation
 * succeeded.
 */
static int
grow(Array *arr)
{
	if ((arr->size / arr->unit) > (size_t)arr->length)
		return 0;

	if (!(arr->flags & ARRAY_FIXED)) {
		if ((arr->data = realloc(arr->data, arr->size +
		    OFFSET(arr->chksize))) == NULL) {
			arr->size = arr->length = 0;
			return -1;
		}

		arr->size += OFFSET(arr->chksize);
	} else
		return -1;

	return 0;
}
