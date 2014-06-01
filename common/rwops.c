/*
 * rwops.c -- callback managed file operations
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

#include "rwops.h"

/* --------------------------------------------------------
 * RW functions
 * -------------------------------------------------------- */

typedef struct {
	int size;		/*! size ref */
	int seek;		/*! seek ref */
	int read;		/*! read ref */
	int write;		/*! write ref */
	int close;		/*! close */
} Funcs;

static Sint64
rw_size(SDL_RWops *ops)
{
	Funcs *opsref	= ops->hidden.unknown.data1;
	lua_State *L	= ops->hidden.unknown.data2;

	lua_rawgeti(L, LUA_REGISTRYINDEX, opsref->size);
	lua_call(L, 0, 1);

	return lua_tointeger(L, -1);
}

static Sint64
rw_seek(SDL_RWops *ops, Sint64 offset, int whence)
{
	Funcs *opsref	= ops->hidden.unknown.data1;
	lua_State *L	= ops->hidden.unknown.data2;

	lua_rawgeti(L, LUA_REGISTRYINDEX, opsref->seek);
	lua_pushinteger(L, (int)offset);
	lua_pushinteger(L, whence);
	lua_call(L, 2, 1);

	return lua_tointeger(L, -1);
}

static size_t
rw_read(SDL_RWops *ops, void *dst, size_t num, size_t size)
{
	Funcs *opsref	= ops->hidden.unknown.data1;
	lua_State *L	= ops->hidden.unknown.data2;
	int nread	= 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, opsref->read);
	lua_pushinteger(L, num);
	lua_pushinteger(L, size);
	lua_call(L, 2, 2);

	/* Use the content as a string */
	if (lua_type(L, -1) == LUA_TNUMBER) {
		const char *data;

		/* Retrieve length or 0 on error or EOF */
		nread = lua_tointeger(L, -1);

		if (nread != EOF && nread > 0) {
			data = lua_tostring(L, -2);
			memcpy(dst, data, nread);
		}
	}

	return nread;
}

static size_t
rw_write(SDL_RWops *ops, const void *data, size_t num, size_t size)
{
	Funcs *opsref	= ops->hidden.unknown.data1;
	lua_State *L	= ops->hidden.unknown.data2;

	lua_rawgeti(L, LUA_REGISTRYINDEX, opsref->write);
	lua_pushlstring(L, data, num * size);
	lua_pushinteger(L, num);
	lua_pushinteger(L, size);
	lua_call(L, 3, 1);

	return lua_tointeger(L, 1);
}

static int
rw_close(SDL_RWops *ops)
{
	Funcs *opsref	= ops->hidden.unknown.data1;
	lua_State *L	= ops->hidden.unknown.data2;

	lua_rawgeti(L, LUA_REGISTRYINDEX, opsref->close);
	lua_call(L, 0, 0);
	
	return (lua_type(L, -1) == LUA_TNUMBER) ? lua_tointeger(L, -1) : 0;
}

/*
 * SDL.RWCreate(table)
 *
 * Table must have:
 *
 *	function size() -> return the size
 *	function seek(offset, whence) -> return the seek
 *	function read(n, size) -> return the string, the number of bytes read
 *	function write(data, n, size) -> return the number of bytes written
 *	function close()
 *
 * Params:
 *	table the table as above
 *
 * Returns:
 *	The rwops object or nil
 *	The error message
 */
static int
l_rw_create(lua_State *L)
{
#define FIELD_REF(idx, field, ref) do {					\
	lua_getfield(L, idx, field);					\
	if (lua_type(L, -1) != LUA_TFUNCTION)				\
		goto missing;						\
	lua_pushvalue(L, -1);						\
	ref = luaL_ref(L, LUA_REGISTRYINDEX);				\
	lua_pop(L, 1);							\
} while (/* CONSTCOND */ 0)

	SDL_RWops *ops;
	Funcs *opsref;

	/* Only takes a table */
	luaL_checktype(L, 1, LUA_TTABLE);

	ops = SDL_AllocRW();
	if (ops == NULL)
		return commonPushSDLError(L, 1);

	opsref = malloc(sizeof (Funcs));
	if (opsref == NULL) {
		SDL_FreeRW(ops);
		return commonPushErrno(L, 1);
	}

	opsref->size = opsref->seek = opsref->read = opsref->write = LUA_REFNIL;

	/* Check fields, all are mandatory */
	FIELD_REF(1, "size", opsref->size);
	FIELD_REF(1, "seek", opsref->seek);
	FIELD_REF(1, "read", opsref->read);
	FIELD_REF(1, "write", opsref->write);
	FIELD_REF(1, "close", opsref->close);

	/* Use our callback to call Lua functions */
	ops->size = rw_size;
	ops->seek = rw_seek;
	ops->read = rw_read;
	ops->write = rw_write;
	ops->close = rw_close;
	ops->hidden.unknown.data1 = opsref;
	ops->hidden.unknown.data2 = L;

	return commonPush(L, "p", RWOpsName, ops);

missing:
	SDL_SetError("invalid table given");

	if (opsref->size != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, opsref->size);
	if (opsref->seek != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, opsref->seek);
	if (opsref->read != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, opsref->read);
	if (opsref->write != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, opsref->write);

	SDL_FreeRW(ops);
	free(opsref);

	return commonPushSDLError(L, 1);
}

/*
 * SDL.RWFromFile(path, mode)
 *
 * Params:
 *	path the path
 *	mode the mode
 *
 * Returns:
 *	The rwops object or nil
 *	The error message
 */
static int
l_rw_fromFile(lua_State *L)
{
	const char *file = luaL_checkstring(L, 1);
	const char *mode = luaL_checkstring(L, 2);
	SDL_RWops *ops;

	ops = SDL_RWFromFile(file, mode);
	if (ops == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", RWOpsName, ops);	
}

/* --------------------------------------------------------
 * RW object methods
 * -------------------------------------------------------- */

const luaL_Reg RWOpsFunctions[] = {
	{ "RWCreate",		l_rw_create		},
	{ "RWFromFile",		l_rw_fromFile		},
	{ NULL,			NULL			}
};

/* --------------------------------------------------------
 * RW object private helpers
 * -------------------------------------------------------- */

typedef enum endian {
	Big = 1,
	Little
} Endian;

typedef enum action {
	Read,
	Write
} Action;

typedef int (*ConvFunc)(lua_State *, SDL_RWops *, Endian);

static int
rw_read16(lua_State *L, SDL_RWops *ops, Endian endian)
{
	Uint16 b = (endian == Big) ? SDL_ReadBE16(ops) : SDL_ReadLE16(ops);

	return commonPush(L, "i", b);
}

static int
rw_read32(lua_State *L, SDL_RWops *ops, Endian endian)
{
	Uint32 b = (endian == Big) ? SDL_ReadBE32(ops) : SDL_ReadLE32(ops);

	return commonPush(L, "i", b);
}

static int
rw_write16(lua_State *L, SDL_RWops *ops, Endian endian)
{
	Uint16 b = luaL_checkinteger(L, 2);
	size_t nr;

	nr = (endian == Big) ? SDL_WriteBE16(ops, b) : SDL_WriteLE16(ops, b);
	if (nr == 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static int
rw_write32(lua_State *L, SDL_RWops *ops, Endian endian)
{
	Uint32 b = luaL_checkinteger(L, 2);
	size_t nr;

	nr = (endian == Big) ? SDL_WriteBE32(ops, b) : SDL_WriteLE32(ops, b);
	if (nr == 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static void
rw_readparams(lua_State *L, Action action, int size, const char *mode, ConvFunc *func, Endian *endian)
{
	/* Retrieve the integer size */
	if (size == 16)
		*func = (action == Read) ? rw_read16 : rw_write16;
	else if(size == 32)
		*func = (action == Read) ? rw_read32 : rw_write32;
	else
		*func = NULL;

	if (*func == NULL)
		(void)luaL_error(L, "invalid size given %d", size);

	/* Retrieve the endian mode */
	if (strcmp(mode, "BE") == 0)
		*endian = Big;
	else if (strcmp(mode, "LE") == 0)
		*endian = Little;
	else
		*endian = 0;

	if (*endian == 0)
		(void)luaL_error(L, "invalid endian mode %s", mode);
}

/* --------------------------------------------------------
 * RW object metamethods
 * -------------------------------------------------------- */

/*
 * RWOps:close()
 */
static int
l_rw_close(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);

	if (SDL_RWclose(ops) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * RWOps:read(size, num)
 *
 * Arguments:
 *	size each object size
 *	num number of object to read
 *
 * Returns:
 *	The read string or nil
 *	The error message
 */
static int
l_rw_read(lua_State *L)
{
	SDL_RWops *ops	= commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	size_t size	= luaL_checkinteger(L, 2);
	size_t num	= luaL_checkinteger(L, 3);
	size_t nread;
	int nret;
	void *ptr;

	if ((ptr = malloc(size * num)) == NULL)
		return commonPushErrno(L, 1);

	nread = SDL_RWread(ops, ptr, size, num);

	if (nread == 0)
		nret = commonPush(L, "nis", nread, SDL_GetError());
	else {
		lua_pushlstring(L, ptr, nread * size);
		lua_pushinteger(L, nread);

		nret = 2;
	}

	free(ptr);

	return nret;
}

/*
 * RWOps:readByte(size, mode)
 *
 * Arguments:
 *	size the size (16, 32 or 64)
 *	mode the endian mode "BE" or "LE
 *
 * Returns:
 *	The byte read
 */
static int
l_rw_readByte(lua_State *L)
{
	SDL_RWops *ops		= commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	int size		= luaL_checkinteger(L, 2);
	const char *mode	= luaL_checkstring(L, 3);
	Endian endian;
	ConvFunc func;

	rw_readparams(L, Read, size, mode, &func, &endian);

	return func(L, ops, endian);
}

/*
 * RWOps:seek(offset, whence)
 *
 * Arguments:
 *	offset the offset size
 *	whence see SDL.rwopsSeek
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_rw_seek(lua_State *L)
{
	SDL_RWops *ops	= commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	Sint64 offset	= luaL_checkinteger(L, 2);
	int whence	= luaL_checkinteger(L, 3);

	if (SDL_RWseek(ops, offset, whence) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * RWOps:tell()
 *
 * Returns:
 *	The current offset
 */
static int
l_rw_tell(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);

	return commonPush(L, "l", SDL_RWtell(ops));
}

/*
 * RWOps:write(string)
 *
 * Arguments:
 *	data the data to write
 *
 * Returns:
 *	The number of bytes written or nil
 *	The error message
 */
static int
l_rw_write(lua_State *L)
{
	SDL_RWops *ops	= commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	size_t length;
	const char *src	= luaL_checklstring(L, 2, &length);
	int nret, nwritten;

	nwritten = SDL_RWwrite(ops, src, length, 1);
	if (nwritten == 0)
		nret = commonPush(L, "i", nwritten);
	else
		nret = commonPush(L, "ns", SDL_GetError());

	return nret;
}

/*
 * RWOps:writeByte(byte, size, mode)
 *
 * Arguments:
 *	byte the byte to write
 *	size the size (16, 32 or 64)
 *	mode the mode "LE" or "BE"
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_rw_writeByte(lua_State *L)
{
	SDL_RWops *ops		= commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	int size		= luaL_checkinteger(L, 3);
	const char *mode	= luaL_checkstring(L, 4);
	ConvFunc func;
	Endian endian;

	rw_readparams(L, Write, size, mode, &func, &endian);

	return func(L, ops, endian);
}

/*
 * RWOps:__eq
 */
static int
l_rw_eq(lua_State *L)
{
	SDL_RWops *o1 = commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	SDL_RWops *o2 = commonGetAs(L, 2, RWOpsName, SDL_RWops *);

	return commonPush(L, "b", o1 == o2);
}

/*
 * RWOps:__gc
 */
static int
l_rw_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, RWOpsName);

	if (udata->mustdelete) {
		SDL_RWops *ops = udata->data;

		if (ops->type == SDL_RWOPS_UNKNOWN) {
			Funcs *opsref = ops->hidden.unknown.data1;

			luaL_unref(L, LUA_REGISTRYINDEX, opsref->size);
			luaL_unref(L, LUA_REGISTRYINDEX, opsref->seek);
			luaL_unref(L, LUA_REGISTRYINDEX, opsref->read);
			luaL_unref(L, LUA_REGISTRYINDEX, opsref->write);
			luaL_unref(L, LUA_REGISTRYINDEX, opsref->close);

			free(opsref);
		}

		SDL_FreeRW(ops);
	}

	return 0;
}

/*
 * RWOps:__tostring
 */
static int
l_rw_tostring(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);

	lua_pushfstring(L, "RWops %p: type %d", ops, ops->type);

	return 1;
}

static const luaL_Reg methods[] = {
	{ "close", 		l_rw_close		},
	{ "read",		l_rw_read		},
	{ "readByte",		l_rw_readByte		},
	{ "seek",		l_rw_seek		},
	{ "tell",		l_rw_tell		},
	{ "write",		l_rw_write		},
	{ "writeByte",		l_rw_writeByte		},
	{ NULL,			NULL			}
};

static const luaL_Reg metamethods[] = {
	{ "__eq",		l_rw_eq			},
	{ "__gc",		l_rw_gc			},
	{ "__tostring",		l_rw_tostring		},
	{ NULL,			NULL			}
};

/* --------------------------------------------------------
 * RW object definition
 * -------------------------------------------------------- */

const CommonObject RWOps = {
	"RWOps",
	methods,
	metamethods
};

const CommonEnum RWOpsSeek[] = {
	{ "Set",		RW_SEEK_SET		},
	{ "Current",		RW_SEEK_CUR		},
	{ "End",		RW_SEEK_END		},
	{ NULL,			-1			}
};

const CommonEnum RWOpsType[] = {
	{ "Unknown",		SDL_RWOPS_UNKNOWN	},
	{ "WinFile",		SDL_RWOPS_WINFILE	},
	{ "StdFile",		SDL_RWOPS_STDFILE	},
	{ "JNIFile",		SDL_RWOPS_JNIFILE	},
	{ "Memory",		SDL_RWOPS_MEMORY	},
	{ "MemoryRO",		SDL_RWOPS_MEMORY_RO	},
	{ NULL,			-1			}
};
