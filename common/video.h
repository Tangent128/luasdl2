/*
 * video.h -- video initialization and display management
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

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <common/array.h>
#include <common/common.h>

/* --------------------------------------------------------
 * Shared functions
 * -------------------------------------------------------- */

/**
 * @struct line
 * @brief Get line coordinates
 */
typedef struct line {
	int	x1;
	int	y1;
	int	x2;
	int	y2;
} Line;

/**
 * Push a SDL_Color as a table with three fields r, g, b to Lua.
 *
 * @param L the Lua state
 * @param color the color
 */
void
videoPushColorRGB(lua_State *L, const SDL_Color *color);

/**
 * Push a SDL_Rect as a table to Lua.
 *
 * @param L the Lua state
 * @param rect the rectangle to push
 */
void
videoPushRect(lua_State *L, const SDL_Rect *rect);

/**
 * Get a SDL_Rect from a Lua table. If the value at index is not a
 * table, raises an error.
 *
 * @param L the Lua state
 * @param index the table index
 * @param rect the result
 */
void
videoGetRect(lua_State *L, int index, SDL_Rect *rect);

/**
 * Get a list of rects from table of tables. Does not raises an error.
 *
 * @param L the Lua state
 * @param index the table index
 * @param array the array to fill (will be initialized)
 * @return 0 on success or -1 on failure
 */
int
videoGetRects(lua_State *L, int index, Array *rects);

/**
 * Push a SDL_Point as a table to Lua.
 *
 * @param L the Lua state
 * @param point the point to push
 */
void
videoPushPoint(lua_State *L, const SDL_Point *point);

/**
 * Get a point from a Lua table. If the value at index is not a
 * table, raises an error.
 *
 * @param L the Lua state
 * @param index the table index
 * @param point the result
 */
void
videoGetPoint(lua_State *L, int index, SDL_Point *point);

/**
 * Get a list of points from table of tables. Does not raises an error.
 *
 * @param L the Lua state
 * @param index the table index
 * @param array the array to fill (will be initialized)
 * @return 0 on success or -1 on failure
 */
int
videoGetPoints(lua_State *L, int index, Array *array);

/**
 * Get a line as 4 int or a table of 4 fields.
 *
 * @param L the Lua state
 * @param index the index
 * @param line the line coords
 */
void
videoGetLine(lua_State *L, int index, Line *line);

/**
 * Get a color from a Lua table or a hexadecimal number.
 *
 * @param L the Lua state
 * @param index the value index
 * @return the color as hexadecimal
 */
Uint32
videoGetColorHex(lua_State *L, int index);

/**
 * Get a color from a Lua table or a hexadecimal number.
 *
 * @param L the Lua state
 * @param index the value index
 * @return the color as RGB structure
 */
SDL_Color
videoGetColorRGB(lua_State *L, int index);

/**
 * Read colors from Lua and sets them to the colors array as SDL_Color
 * structure.
 *
 * @param L the Lua state
 * @param index the colors index
 * @param colors the sequence of colors
 */
int
videoGetColorsRGB(lua_State *L, int index, Array *colors);

/**
 * Get a SDL_DisplayMode from a Lua table. Raises an error if
 * idx is not a table.
 *
 * @param L the Lua state
 * @param idx the value index
 * @param mode the destination vaule
 */
void
videoGetDisplayMode(lua_State *L, int idx, SDL_DisplayMode *mode);

/**
 * Push a SDL_DisplayMode as a table.
 *
 * @param L the Lua state
 * @param mode the mode
 */
void
videoPushDisplayMode(lua_State *L, const SDL_DisplayMode *mode);

#endif /* !_VIDEO_H_ */
