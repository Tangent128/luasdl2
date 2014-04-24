/*
 * thread.h -- thread creation management
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

#ifndef _THREAD_H_
#define _THREAD_H_

#include <common/common.h>

#define ThreadName	Thread.name

extern const luaL_Reg ThreadFunctions[];

extern const CommonObject Thread;

/**
 * Dump a file or a function at the given index from the owner Lua state and
 * pushes the result to the th state.
 *
 * If everything goes wrong, it returns 2, the number of objects (to the owner
 * state) pushed which are nil + the error message, otherwise 0 is returned.
 *
 * @param owner the Lua state which owns the thread
 * @param th the Lua state where the function dumped should be pushed
 * @param index the function / filepath index
 * @return 0 or 2
 */
int
threadDump(lua_State *owner, lua_State *th, int index);

#endif /* !_THREAD_H_ */
