/*
 * table.h -- table helpers
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

#ifndef _TABLE_H_
#define _TABLE_H_

#include "common.h"

/**
 * Check if a table field is of a type.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param type the Lua type to test
 * @return true if they equals
 */
int
tableIsType(lua_State *L, int idx, const char *name, int type);

/**
 * Just like luaL_checktype() but for table fields. Also raises an error
 * if not the wanted userdata.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param field the field name
 * @param tname the metatable name
 * @return the object
 */
CommonUserdata *
tableGetUserdata(lua_State *L, int idx, const char *field, const char *tname);

/**
 * Get a integer from a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @return the value or 0
 */
int
tableGetInt(lua_State *L, int idx, const char *name);

/**
 * Get a double from a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @return the value or 0
 */
double
tableGetDouble(lua_State *L, int idx, const char *name);

/**
 * Similar to tableGetEnum() but from a table field.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @return the value or 0
 * @see tableGetEnum
 */
int
tableGetEnum(lua_State *L, int idx, const char *name);

/**
 * Get a string from a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @return the value or NULL
 */
const char *
tableGetString(lua_State *L, int idx, const char *name);

/**
 * Get a string from a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param length the string length
 * @return the value or NULL
 */
const char *
tableGetStringl(lua_State *L, int idx, const char *name, size_t *length);

/**
 * Get a bool from a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @return the value or 0
 */
int
tableGetBool(lua_State *L, int idx, const char *name);

/**
 * Set a integer field to a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param value the value
 */
void
tableSetInt(lua_State *L, int idx, const char *name, int value);

/**
 * Set a double as a field
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param value the value
 */
void
tableSetDouble(lua_State *L, int idx, const char *name, double value);

/**
 * Set a string field to a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param value the value
 */
void
tableSetString(lua_State *L, int idx, const char *name, const char *value);

/**
 * Set a string field to a table but with specific length. Useful for string
 * which don't finish by '\0'.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param value the value
 * @param length the string length
 */
void
tableSetStringl(lua_State *L, int idx, const char *name, const char *value, int length);

/**
 * Set a boolean field to a table.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param name the field name
 * @param value the value
 */
void
tableSetBool(lua_State *L, int idx, const char *name, int value);

/**
 * Set an enumeration to a table field.
 *
 * @param L the Lua state
 * @param idx the table index
 * @param value the value
 * @param evalue the enumeration to check
 * @param name the field name
 */
void
tableSetEnum(lua_State *L,
	     int idx,
	     int value,
	     const CommonEnum *evalue,
	     const char *name);

#endif /* !_TABLE_H_ */
