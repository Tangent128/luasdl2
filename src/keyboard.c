/*
 * keyboard.c -- keyboard event management
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

#include <common/video.h>

#include "keyboard.h"
#include "window.h"

/* --------------------------------------------------------
 * Keyboard private helpers
 * -------------------------------------------------------- */

static int
keysIndex(lua_State *L)
{
	int which = luaL_checkinteger(L, 2);
	int numkeys;
	const Uint8 *keys = SDL_GetKeyboardState(&numkeys);

	if (which >= numkeys)
		return luaL_error(L, "key %d is out of bound", which);

	return commonPush(L, "b", keys[which]);
}

/* --------------------------------------------------------
 * Keyboard functions
 * -------------------------------------------------------- */

/*
 * SDL.getKeyFromName(name)
 *
 * Arguments:
 *	name the key name
 *
 * Returns:
 *	The key code (SDL.key)
 */
static int
l_getKeyFromName(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	return commonPush(L, "i", SDL_GetKeyFromName(name));
}

/*
 * SDL.getKeyFromScancode(sc)
 *
 * Arguments:
 *	sc the scancode (SDL.scancode)
 *
 * Returns:
 *	The key code (SDL.key)
 */
static int
l_getKeyFromScancode(lua_State *L)
{
	SDL_Scancode sc = luaL_checkinteger(L, 2);

	return commonPush(L, "i", SDL_GetKeyFromScancode(sc));
}

/*
 * SDL.getKeyName(code)
 *
 * Arguments:
 *	code the key code (SDL.key)
 *
 * Returns:
 *	The key name (may be empty string)
 */
static int
l_getKeyName(lua_State *L)
{
	SDL_Keycode kc = luaL_checkinteger(L, 1);

	return commonPush(L, "s", SDL_GetKeyName(kc));
}

/*
 * SDL.getKeyboardFocus()
 *
 * Returns:
 *	The window
 */
static int
l_getKeyboardFocus(lua_State *L)
{
	/*
	 * I would never except that a function named "GetKeyboardFocus" would
	 * return a window.
	 */
	SDL_Window *w = SDL_GetKeyboardFocus();
	CommonUserdata *data;

	data = commonPushUserdata(L, WindowName, w);
	data->mustdelete = 0;

	return 1;
}

/*
 * SDL.getKeyboardState()
 *
 * Returns a specific userdata with a __index function which takes the key
 * (SDL.key) to check. The function returns true if the key is pressed.
 *
 * Returns:
 *	A special userdata
 */
static int
l_getKeyboardState(lua_State *L)
{
	/*
	 * Push an empty userdata where a special __index key is set
	 * so user may just write:
	 *
	 * local keys = SDL.getKeyboardState()
	 *
	 * SDL.pumpEvents()
	 * if keys[SDL.keyScancode.Return] then
	 *	...
	 * end
	 */
	(void)lua_newuserdata(L, sizeof (void *));
	lua_createtable(L, 0, 0);
	lua_pushcfunction(L, keysIndex);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	return 1;
}

/*
 * SDL.getModState()
 *
 * Returns:
 *	The modifiers as a enum table (SDL.keymod)
 */
static int
l_getModState(lua_State *L)
{
	SDL_Keymod state = SDL_GetModState();

	commonPushEnum(L, state, KeyboardModifiers);

	return 1;
}

/*
 * SDL.getScancodeFromKey(code)
 *
 * Arguments:
 *	code the key code (SDL.key)
 *
 * Returns:
 *	The scancode (SDL.scancode)
 */
static int
l_getScancodeFromKey(lua_State *L)
{
	SDL_Keycode kc = luaL_checkinteger(L, 1);

	return commonPush(L, "i", SDL_GetScancodeFromKey(kc));
}

/*
 * SDL.getScancodeFromName(name)
 *
 * Arguments:
 *	name the name
 *
 * Returns:
 *	The scancode (SDL.scancode)
 */
static int
l_getScancodeFromName(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	return commonPush(L, "i", SDL_GetScancodeFromName(name));
}

/*
 * SDL.getScancodeName(sc)
 *
 * Arguments:
 *	sc the scancode
 *
 * Returns:
 *	The name (may be empty string)
 */
static int
l_getScancodeName(lua_State *L)
{
	SDL_Scancode code = luaL_checkinteger(L, 1);

	return commonPush(L, "s", SDL_GetScancodeName(code));
}

/*
 * SDL.setModState(modifiers)
 *
 * Arguments:
 *	modifiers the enum table for modifiers
 */
static int
l_setModState(lua_State *L)
{
	SDL_Keymod state = commonGetEnum(L, 1);

	SDL_SetModState(state);

	return 0;
}

/*
 * SDL.setTextInputRect(rect)
 *
 * Arguments:
 *	rect the rectangle
 */
static int
l_setTextInputRect(lua_State *L)
{
	SDL_Rect rect;

	videoGetRect(L, 1, &rect);
	SDL_SetTextInputRect(&rect);

	return 0;
}

/*
 * SDL.startTextInput()
 */
static int
l_startTextInput(lua_State *L)
{
	SDL_StartTextInput();

	(void)L;

	return 0;
}

/*
 * SDL.stopTextInput()
 */
static int
l_stopTextInput(lua_State *L)
{
	SDL_StopTextInput();

	(void)L;

	return 0;
}

const luaL_Reg KeyboardFunctions[] = {
	{ "getKeyFromName",		l_getKeyFromName		},
	{ "getKeyFromScancode",		l_getKeyFromScancode		},
	{ "getKeyName",			l_getKeyName			},
	{ "getKeyboardFocus",		l_getKeyboardFocus		},
	{ "getKeyboardState",		l_getKeyboardState		},
	{ "getModState",		l_getModState			},
	{ "getScancodeFromKey",		l_getScancodeFromKey		},
	{ "getScancodeFromName",	l_getScancodeFromName		},
	{ "getScancodeName",		l_getScancodeName		},
	{ "setModState",		l_setModState			},
	{ "setTextInputRect",		l_setTextInputRect		},
	{ "startTextInput",		l_startTextInput		},
	{ "stopTextInput",		l_stopTextInput			},
	{ NULL,				NULL				},
};

/* --------------------------------------------------------
 * Keyboard enumerations
 * -------------------------------------------------------- */

/*
 * SDL.key
 */
const CommonEnum KeyboardCodes[] = {
	{ "Unknown",			SDLK_UNKNOWN			},
	{ "Return",			SDLK_RETURN			},
	{ "Escape",			SDLK_ESCAPE			},
	{ "Backspace",			SDLK_BACKSPACE			},
	{ "Tab",			SDLK_TAB			},
	{ "Space",			SDLK_SPACE			},
	{ "Exclaim",			SDLK_EXCLAIM			},
	{ "Quotedbl",			SDLK_QUOTEDBL			},
	{ "Hash",			SDLK_HASH			},
	{ "Percent",			SDLK_PERCENT			},
	{ "Dollar",			SDLK_DOLLAR			},
	{ "Ampersand",			SDLK_AMPERSAND			},
	{ "Quote",			SDLK_QUOTE			},
	{ "LeftParen",			SDLK_LEFTPAREN			},
	{ "RightParen",			SDLK_RIGHTPAREN			},
	{ "Asterisk",			SDLK_ASTERISK			},
	{ "Plus",			SDLK_PLUS			},
	{ "Comma",			SDLK_COMMA			},
	{ "Minus",			SDLK_MINUS			},
	{ "Period",			SDLK_PERIOD			},
	{ "Slash",			SDLK_SLASH			},
	{ "0",				SDLK_0				},
	{ "1",				SDLK_1				},
	{ "2",				SDLK_2				},
	{ "3",				SDLK_3				},
	{ "4",				SDLK_4				},
	{ "5",				SDLK_5				},
	{ "6",				SDLK_6				},
	{ "7",				SDLK_7				},
	{ "8",				SDLK_8				},
	{ "9",				SDLK_9				},
	{ "Colon",			SDLK_COLON			},
	{ "Semicolon",			SDLK_SEMICOLON			},
	{ "Less",			SDLK_LESS			},
	{ "Equals",			SDLK_EQUALS			},
	{ "Greater",			SDLK_GREATER			},
	{ "Question",			SDLK_QUESTION			},
	{ "At",				SDLK_AT				},
	{ "LeftBracked",		SDLK_LEFTBRACKET		},
	{ "Backslash",			SDLK_BACKSLASH			},
	{ "RightBracket",		SDLK_RIGHTBRACKET		},
	{ "Caret",			SDLK_CARET			},
	{ "Underscore",			SDLK_UNDERSCORE			},
	{ "Backquote",			SDLK_BACKQUOTE			},
	{ "a",				SDLK_a				},
	{ "b",				SDLK_b				},
	{ "c",				SDLK_c				},
	{ "d",				SDLK_d				},
	{ "e",				SDLK_e				},
	{ "f",				SDLK_f				},
	{ "g",				SDLK_g				},
	{ "h",				SDLK_h				},
	{ "i",				SDLK_i				},
	{ "j",				SDLK_j				},
	{ "k",				SDLK_k				},
	{ "l",				SDLK_l				},
	{ "m",				SDLK_m				},
	{ "n",				SDLK_n				},
	{ "o",				SDLK_o				},
	{ "p",				SDLK_p				},
	{ "q",				SDLK_q				},
	{ "r",				SDLK_r				},
	{ "s",				SDLK_s				},
	{ "t",				SDLK_t				},
	{ "u",				SDLK_u				},
	{ "v",				SDLK_v				},
	{ "w",				SDLK_w				},
	{ "x",				SDLK_x				},
	{ "y",				SDLK_y				},
	{ "z",				SDLK_z				},
	{ "Capslock",			SDLK_CAPSLOCK			},
	{ "F1",				SDLK_F1				},
	{ "F2",				SDLK_F2				},
	{ "F3",				SDLK_F3				},
	{ "F4",				SDLK_F4				},
	{ "F5",				SDLK_F5				},
	{ "F6",				SDLK_F6				},
	{ "F7",				SDLK_F7				},
	{ "F8",				SDLK_F8				},
	{ "F9",				SDLK_F9				},
	{ "F10",			SDLK_F10			},
	{ "F11",			SDLK_F11			},
	{ "F11",			SDLK_F12			},
	{ "Printscreen",		SDLK_PRINTSCREEN		},
	{ "ScrollLock",			SDLK_SCROLLLOCK			},
	{ "Pause",			SDLK_PAUSE			},
	{ "Insert",			SDLK_INSERT			},
	{ "Home",			SDLK_HOME			},
	{ "PageUp",			SDLK_PAGEUP			},
	{ "Delete",			SDLK_DELETE			},
	{ "End",			SDLK_END			},
	{ "PageDown",			SDLK_PAGEDOWN			},
	{ "Right",			SDLK_RIGHT			},
	{ "Left",			SDLK_LEFT			},
	{ "Down",			SDLK_DOWN			},
	{ "Up",				SDLK_UP				},
	{ "NumlockClear",		SDLK_NUMLOCKCLEAR		},
	{ "KPDivide",			SDLK_KP_DIVIDE			},
	{ "KPMultiply",			SDLK_KP_MULTIPLY		},
	{ "KPMinus",			SDLK_KP_MINUS			},
	{ "KPPlus",			SDLK_KP_PLUS			},
	{ "KPEnter",			SDLK_KP_ENTER			},
	{ "KP1",			SDLK_KP_1			},
	{ "KP2",			SDLK_KP_2			},
	{ "KP3",			SDLK_KP_3			},
	{ "KP4",			SDLK_KP_4			},
	{ "KP5",			SDLK_KP_5			},
	{ "KP6",			SDLK_KP_6			},
	{ "KP7",			SDLK_KP_7			},
	{ "KP8",			SDLK_KP_8			},
	{ "KP9",			SDLK_KP_9			},
	{ "KP0",			SDLK_KP_0			},
	{ "KPPeriod",			SDLK_KP_PERIOD			},
	{ "Application",		SDLK_APPLICATION		},
	{ "Power",			SDLK_POWER			},
	{ "KPEquals",			SDLK_KP_EQUALS			},
	{ "F13",			SDLK_F13			},
	{ "F14",			SDLK_F14			},
	{ "F15",			SDLK_F15			},
	{ "F16",			SDLK_F16			},
	{ "F17",			SDLK_F17			},
	{ "F18",			SDLK_F18			},
	{ "F19",			SDLK_F19			},
	{ "F20",			SDLK_F20			},
	{ "F21",			SDLK_F21			},
	{ "F22",			SDLK_F22			},
	{ "F23",			SDLK_F23			},
	{ "F24",			SDLK_F24			},
	{ "Execute",			SDLK_EXECUTE			},
	{ "Help",			SDLK_HELP			},
	{ "Menu",			SDLK_MENU			},
	{ "Select",			SDLK_SELECT			},
	{ "Stop",			SDLK_STOP			},
	{ "Again",			SDLK_AGAIN			},
	{ "Undo",			SDLK_UNDO			},
	{ "Cut",			SDLK_CUT			},
	{ "Copy",			SDLK_COPY			},
	{ "Paste",			SDLK_PASTE			},
	{ "Find",			SDLK_FIND			},
	{ "Mute",			SDLK_MUTE			},
	{ "VolumeUp",			SDLK_VOLUMEUP			},
	{ "VolumeDown",			SDLK_VOLUMEDOWN			},
	{ "KPComma",			SDLK_KP_COMMA			},
	{ "KPEqualsAS400",		SDLK_KP_EQUALSAS400		},
	{ "Alterase",			SDLK_ALTERASE			},
	{ "SysReq",			SDLK_SYSREQ			},
	{ "Cancel",			SDLK_CANCEL			},
	{ "Clear",			SDLK_CLEAR			},
	{ "Prior",			SDLK_PRIOR			},
	{ "Return2",			SDLK_RETURN2			},
	{ "Separator",			SDLK_SEPARATOR			},
	{ "Out",			SDLK_OUT			},
	{ "Oper",			SDLK_OPER			},
	{ "Clearagain",			SDLK_CLEARAGAIN			},
	{ "CrSel",			SDLK_CRSEL			},
	{ "Exsel",			SDLK_EXSEL			},
	{ "KP00",			SDLK_KP_00			},
	{ "KP000",			SDLK_KP_000			},
	{ "ThousandsSeparator",		SDLK_THOUSANDSSEPARATOR		},
	{ "DecimalSeparator",		SDLK_DECIMALSEPARATOR		},
	{ "CurrencyUnit",		SDLK_CURRENCYUNIT		},
	{ "CurrencySubUnit",		SDLK_CURRENCYSUBUNIT		},
	{ "KPLeftParen",		SDLK_KP_LEFTPAREN		},
	{ "KPRightParen",		SDLK_KP_RIGHTPAREN		},
	{ "KPLeftBrace",		SDLK_KP_LEFTBRACE		},
	{ "KPRightBrace",		SDLK_KP_RIGHTBRACE		},
	{ "KPTab",			SDLK_KP_TAB			},
	{ "KPBackSpace",		SDLK_KP_BACKSPACE		},
	{ "KPA",			SDLK_KP_A			},
	{ "KPB",			SDLK_KP_B			},
	{ "KPC",			SDLK_KP_C			},
	{ "KPD",			SDLK_KP_D			},
	{ "KPE",			SDLK_KP_E			},
	{ "KPF",			SDLK_KP_F			},
	{ "KPXor",			SDLK_KP_XOR			},
	{ "KPPower",			SDLK_KP_POWER			},
	{ "KPPercent",			SDLK_KP_PERCENT			},
	{ "KPLess",			SDLK_KP_LESS			},
	{ "KPGreater",			SDLK_KP_GREATER			},
	{ "KPAmpersand",		SDLK_KP_AMPERSAND		},
	{ "KPDblAmpersand",		SDLK_KP_DBLAMPERSAND		},
	{ "KPVerticalBar",		SDLK_KP_VERTICALBAR		},
	{ "KPDblVerticalBar",		SDLK_KP_DBLVERTICALBAR		},
	{ "KPColon",			SDLK_KP_COLON			},
	{ "KPHash",			SDLK_KP_HASH			},
	{ "KPSpace",			SDLK_KP_SPACE			},
	{ "KPAt",			SDLK_KP_AT			},
	{ "KPExclam",			SDLK_KP_EXCLAM			},
	{ "KPMemStore",			SDLK_KP_MEMSTORE		},
	{ "KPMemRecall",		SDLK_KP_MEMRECALL		},
	{ "KPMemClear",			SDLK_KP_MEMCLEAR		},
	{ "KPMemAdd",			SDLK_KP_MEMADD			},
	{ "KPMemSubstract",		SDLK_KP_MEMSUBTRACT		},
	{ "KPMemMultiply",		SDLK_KP_MEMMULTIPLY		},
	{ "KPMemDivide",		SDLK_KP_MEMDIVIDE		},
	{ "KPPlusMinus",		SDLK_KP_PLUSMINUS		},
	{ "KPClear",			SDLK_KP_CLEAR			},
	{ "KPClearEntry",		SDLK_KP_CLEARENTRY		},
	{ "KPBinary",			SDLK_KP_BINARY			},
	{ "KPOctal",			SDLK_KP_OCTAL			},
	{ "KPDecimal",			SDLK_KP_DECIMAL			},
	{ "KPHexadecimal",		SDLK_KP_HEXADECIMAL		},
	{ "LeftControl",		SDLK_LCTRL			},
	{ "LeftShift",			SDLK_LSHIFT			},
	{ "LeftAlt",			SDLK_LALT			},
	{ "LeftGUI",			SDLK_LGUI			},
	{ "RightControl",		SDLK_RCTRL			},
	{ "RightShift",			SDLK_RSHIFT			},
	{ "RightAlt",			SDLK_RALT			},
	{ "RGUI",			SDLK_RGUI			},
	{ "Mode",			SDLK_MODE			},
	{ "AudioNext",			SDLK_AUDIONEXT			},
	{ "AudioPrev",			SDLK_AUDIOPREV			},
	{ "AudioStop",			SDLK_AUDIOSTOP			},
	{ "AudioPlay",			SDLK_AUDIOPLAY			},
	{ "AudioMute",			SDLK_AUDIOMUTE			},
	{ "MediaSelect",		SDLK_MEDIASELECT		},
	{ "WWW",			SDLK_WWW			},
	{ "Mail",			SDLK_MAIL			},
	{ "Calculator",			SDLK_CALCULATOR			},
	{ "Computer",			SDLK_COMPUTER			},
	{ "ACSearch",			SDLK_AC_SEARCH			},
	{ "ACHome",			SDLK_AC_HOME			},
	{ "ACBack",			SDLK_AC_BACK			},
	{ "ACForward",			SDLK_AC_FORWARD			},
	{ "ACStop",			SDLK_AC_STOP			},
	{ "ACRefresh",			SDLK_AC_REFRESH			},
	{ "ACBookmarks",		SDLK_AC_BOOKMARKS		},
	{ "BrightnessDown",		SDLK_BRIGHTNESSDOWN		},
	{ "BrightnessUp",		SDLK_BRIGHTNESSUP		},
	{ "DisplaySwitch",		SDLK_DISPLAYSWITCH		},
	{ "KBDIllumToggle",		SDLK_KBDILLUMTOGGLE		},
	{ "KBDIllumDown",		SDLK_KBDILLUMDOWN		},
	{ "KBDIllumUp",			SDLK_KBDILLUMUP			},
	{ "Eject",			SDLK_EJECT			},
	{ "Sleep",			SDLK_SLEEP			},
	{ NULL,				-1				}
};

/*
 * SDL.scancode
 */
const CommonEnum KeyboardScancodes[] = {
	{ "Unknown",			SDL_SCANCODE_UNKNOWN		},
	{ "A",				SDL_SCANCODE_A			},
	{ "B",				SDL_SCANCODE_B			},
	{ "C",				SDL_SCANCODE_C			},
	{ "D",				SDL_SCANCODE_D			},
	{ "E",				SDL_SCANCODE_E			},
	{ "F",				SDL_SCANCODE_F			},
	{ "G",				SDL_SCANCODE_G			},
	{ "H",				SDL_SCANCODE_H			},
	{ "I",				SDL_SCANCODE_I			},
	{ "J",				SDL_SCANCODE_J			},
	{ "K",				SDL_SCANCODE_K			},
	{ "L",				SDL_SCANCODE_L			},
	{ "M",				SDL_SCANCODE_M			},
	{ "N",				SDL_SCANCODE_N			},
	{ "O",				SDL_SCANCODE_O			},
	{ "P",				SDL_SCANCODE_P			},
	{ "Q",				SDL_SCANCODE_Q			},
	{ "R",				SDL_SCANCODE_R			},
	{ "S",				SDL_SCANCODE_S			},
	{ "T",				SDL_SCANCODE_T			},
	{ "U",				SDL_SCANCODE_U			},
	{ "V",				SDL_SCANCODE_V			},
	{ "W",				SDL_SCANCODE_W			},
	{ "X",				SDL_SCANCODE_X			},
	{ "Y",				SDL_SCANCODE_Y			},
	{ "Z",				SDL_SCANCODE_Z			},
	{ "1",				SDL_SCANCODE_1			},
	{ "2",				SDL_SCANCODE_2			},
	{ "3",				SDL_SCANCODE_3			},
	{ "4",				SDL_SCANCODE_4			},
	{ "5",				SDL_SCANCODE_5			},
	{ "6",				SDL_SCANCODE_6			},
	{ "7",				SDL_SCANCODE_7			},
	{ "8",				SDL_SCANCODE_8			},
	{ "9",				SDL_SCANCODE_9			},
	{ "0",				SDL_SCANCODE_0			},
	{ "Return",			SDL_SCANCODE_RETURN		},
	{ "Escape",			SDL_SCANCODE_ESCAPE		},
	{ "Backspace",			SDL_SCANCODE_BACKSPACE		},
	{ "Tab",			SDL_SCANCODE_TAB		},
	{ "Space",			SDL_SCANCODE_SPACE		},
	{ "Minus",			SDL_SCANCODE_MINUS		},
	{ "Equals",			SDL_SCANCODE_EQUALS		},
	{ "LeftBracket",		SDL_SCANCODE_LEFTBRACKET	},
	{ "RightBracked",		SDL_SCANCODE_RIGHTBRACKET	},
	{ "Backslash",			SDL_SCANCODE_BACKSLASH		},
	{ "NonUShash",			SDL_SCANCODE_NONUSHASH		},
	{ "SemiColon",			SDL_SCANCODE_SEMICOLON		},
	{ "Apostrophe",			SDL_SCANCODE_APOSTROPHE		},
	{ "Grave",			SDL_SCANCODE_GRAVE		},
	{ "Comma",			SDL_SCANCODE_COMMA		},
	{ "Period",			SDL_SCANCODE_PERIOD		},
	{ "Slash",			SDL_SCANCODE_SLASH		},
	{ "CapsLock",			SDL_SCANCODE_CAPSLOCK		},
	{ "F1",				SDL_SCANCODE_F1			},
	{ "F2",				SDL_SCANCODE_F2			},
	{ "F3",				SDL_SCANCODE_F3			},
	{ "F4",				SDL_SCANCODE_F4			},
	{ "F5",				SDL_SCANCODE_F5			},
	{ "F6",				SDL_SCANCODE_F6			},
	{ "F7",				SDL_SCANCODE_F7			},
	{ "F8",				SDL_SCANCODE_F8			},
	{ "F9",				SDL_SCANCODE_F9			},
	{ "F10",			SDL_SCANCODE_F10		},
	{ "F11",			SDL_SCANCODE_F11		},
	{ "F12",			SDL_SCANCODE_F12		},
	{ "PrintScreen",		SDL_SCANCODE_PRINTSCREEN	},
	{ "ScrollLock",			SDL_SCANCODE_SCROLLLOCK		},
	{ "Pause",			SDL_SCANCODE_PAUSE		},
	{ "Insert",			SDL_SCANCODE_INSERT		},
	{ "Home",			SDL_SCANCODE_HOME		},
	{ "PageUp",			SDL_SCANCODE_PAGEUP		},
	{ "Delete",			SDL_SCANCODE_DELETE		},
	{ "End",			SDL_SCANCODE_END		},
	{ "PageDown",			SDL_SCANCODE_PAGEDOWN		},
	{ "Right",			SDL_SCANCODE_RIGHT		},
	{ "Left",			SDL_SCANCODE_LEFT		},
	{ "Down",			SDL_SCANCODE_DOWN		},
	{ "Up",				SDL_SCANCODE_UP			},
	{ "NumlockClear",		SDL_SCANCODE_NUMLOCKCLEAR	},
	{ "KPDivide",			SDL_SCANCODE_KP_DIVIDE		},
	{ "KPMultiply",			SDL_SCANCODE_KP_MULTIPLY	},
	{ "KPMinus",			SDL_SCANCODE_KP_MINUS		},
	{ "KPPlus",			SDL_SCANCODE_KP_PLUS		},
	{ "KPEnter",			SDL_SCANCODE_KP_ENTER		},
	{ "KP1",			SDL_SCANCODE_KP_1		},
	{ "KP2",			SDL_SCANCODE_KP_2		},
	{ "KP3",			SDL_SCANCODE_KP_3		},
	{ "KP4",			SDL_SCANCODE_KP_4		},
	{ "KP5",			SDL_SCANCODE_KP_5		},
	{ "KP6",			SDL_SCANCODE_KP_6		},
	{ "KP7",			SDL_SCANCODE_KP_7		},
	{ "KP8",			SDL_SCANCODE_KP_8		},
	{ "KP9",			SDL_SCANCODE_KP_9		},
	{ "KP0",			SDL_SCANCODE_KP_0		},
	{ "KPPeriod",			SDL_SCANCODE_KP_PERIOD		},
	{ "NonUSBackslash",		SDL_SCANCODE_NONUSBACKSLASH	},
	{ "Application",		SDL_SCANCODE_APPLICATION	},
	{ "Power",			SDL_SCANCODE_POWER		},
	{ "KPEquals",			SDL_SCANCODE_KP_EQUALS		},
	{ "F13",			SDL_SCANCODE_F13		},
	{ "F14",			SDL_SCANCODE_F14		},
	{ "F15",			SDL_SCANCODE_F15		},
	{ "F16",			SDL_SCANCODE_F16		},
	{ "F17",			SDL_SCANCODE_F17		},
	{ "F18",			SDL_SCANCODE_F18		},
	{ "F19",			SDL_SCANCODE_F19		},
	{ "F20",			SDL_SCANCODE_F20		},
	{ "F21",			SDL_SCANCODE_F21		},
	{ "F22",			SDL_SCANCODE_F22		},
	{ "F23",			SDL_SCANCODE_F23		},
	{ "F24",			SDL_SCANCODE_F24		},
	{ "Execute",			SDL_SCANCODE_EXECUTE		},
	{ "Help",			SDL_SCANCODE_HELP		},
	{ "Menu",			SDL_SCANCODE_MENU		},
	{ "Select",			SDL_SCANCODE_SELECT		},
	{ "Stop",			SDL_SCANCODE_STOP		},
	{ "Again",			SDL_SCANCODE_AGAIN		},
	{ "Undo",			SDL_SCANCODE_UNDO		},
	{ "Cut",			SDL_SCANCODE_CUT		},
	{ "Copy",			SDL_SCANCODE_COPY		},
	{ "Paste",			SDL_SCANCODE_PASTE		},
	{ "Find",			SDL_SCANCODE_FIND		},
	{ "Mute",			SDL_SCANCODE_MUTE		},
	{ "VolumeUp",			SDL_SCANCODE_VOLUMEUP		},
	{ "VolumeDown",			SDL_SCANCODE_VOLUMEDOWN		},
	{ "Comma",			SDL_SCANCODE_KP_COMMA		},
	{ "KPEqualsAS400",		SDL_SCANCODE_KP_EQUALSAS400	},
	{ "International1",		SDL_SCANCODE_INTERNATIONAL1	},
	{ "International2",		SDL_SCANCODE_INTERNATIONAL2	},
	{ "International3",		SDL_SCANCODE_INTERNATIONAL3	},
	{ "International4",		SDL_SCANCODE_INTERNATIONAL4	},
	{ "International5",		SDL_SCANCODE_INTERNATIONAL5	},
	{ "International6",		SDL_SCANCODE_INTERNATIONAL6	},
	{ "International7",		SDL_SCANCODE_INTERNATIONAL7	},
	{ "International8",		SDL_SCANCODE_INTERNATIONAL8	},
	{ "International9",		SDL_SCANCODE_INTERNATIONAL9	},
	{ "Lang1",			SDL_SCANCODE_LANG1		},
	{ "Lang2",			SDL_SCANCODE_LANG2		},
	{ "Lang3",			SDL_SCANCODE_LANG3		},
	{ "Lang4",			SDL_SCANCODE_LANG4		},
	{ "Lang5",			SDL_SCANCODE_LANG5		},
	{ "Lang6",			SDL_SCANCODE_LANG6		},
	{ "Lang7",			SDL_SCANCODE_LANG7		},
	{ "Lang8",			SDL_SCANCODE_LANG8		},
	{ "Lang9",			SDL_SCANCODE_LANG9		},
	{ "Alterase",			SDL_SCANCODE_ALTERASE		},
	{ "Sysreq",			SDL_SCANCODE_SYSREQ		},
	{ "Cancel",			SDL_SCANCODE_CANCEL		},
	{ "Clear",			SDL_SCANCODE_CLEAR		},
	{ "Prior",			SDL_SCANCODE_PRIOR		},
	{ "Return2",			SDL_SCANCODE_RETURN2		},
	{ "Separator",			SDL_SCANCODE_SEPARATOR		},
	{ "Out",			SDL_SCANCODE_OUT		},
	{ "Oper",			SDL_SCANCODE_OPER		},
	{ "Clearagain",			SDL_SCANCODE_CLEARAGAIN		},
	{ "CrSel",			SDL_SCANCODE_CRSEL		},
	{ "Exsel",			SDL_SCANCODE_EXSEL		},
	{ "KP00",			SDL_SCANCODE_KP_00		},
	{ "KP000",			SDL_SCANCODE_KP_000		},
	{ "ThousandsSeparator",		SDL_SCANCODE_THOUSANDSSEPARATOR	},
	{ "DecimalSeparator",		SDL_SCANCODE_DECIMALSEPARATOR	},
	{ "CurrencyUnit",		SDL_SCANCODE_CURRENCYUNIT	},
	{ "CurrencySubUnit",		SDL_SCANCODE_CURRENCYSUBUNIT	},
	{ "LeftParen",			SDL_SCANCODE_KP_LEFTPAREN	},
	{ "RightParen",			SDL_SCANCODE_KP_RIGHTPAREN	},
	{ "LeftBrace",			SDL_SCANCODE_KP_LEFTBRACE	},
	{ "RightBrace",			SDL_SCANCODE_KP_RIGHTBRACE	},
	{ "KPTab",			SDL_SCANCODE_KP_TAB		},
	{ "KPBackspace",		SDL_SCANCODE_KP_BACKSPACE	},
	{ "KPA",			SDL_SCANCODE_KP_A		},
	{ "KPB",			SDL_SCANCODE_KP_B		},
	{ "KPC",			SDL_SCANCODE_KP_C		},
	{ "KPD",			SDL_SCANCODE_KP_D		},
	{ "KPE",			SDL_SCANCODE_KP_E		},
	{ "KPF",			SDL_SCANCODE_KP_F		},
	{ "KPXor",			SDL_SCANCODE_KP_XOR		},
	{ "KPPower",			SDL_SCANCODE_KP_POWER		},
	{ "KPPercent",			SDL_SCANCODE_KP_PERCENT		},
	{ "KPLess",			SDL_SCANCODE_KP_LESS		},
	{ "KPGreater",			SDL_SCANCODE_KP_GREATER		},
	{ "KPAmpersand",		SDL_SCANCODE_KP_AMPERSAND	},
	{ "KPDblAmpersand",		SDL_SCANCODE_KP_DBLAMPERSAND	},
	{ "KPVerticalBar",		SDL_SCANCODE_KP_VERTICALBAR	},
	{ "KPDblVerticalBar",		SDL_SCANCODE_KP_DBLVERTICALBAR	},
	{ "KPColon",			SDL_SCANCODE_KP_COLON		},
	{ "KPHash",			SDL_SCANCODE_KP_HASH		},
	{ "KPSpace",			SDL_SCANCODE_KP_SPACE		},
	{ "KPAt",			SDL_SCANCODE_KP_AT		},
	{ "KPExclam",			SDL_SCANCODE_KP_EXCLAM		},
	{ "KPMemStore",			SDL_SCANCODE_KP_MEMSTORE	},
	{ "KPMemRecall",		SDL_SCANCODE_KP_MEMRECALL	},
	{ "KPMemClear",			SDL_SCANCODE_KP_MEMCLEAR	},
	{ "KPMemAdd",			SDL_SCANCODE_KP_MEMADD		},
	{ "KPMemSubstract",		SDL_SCANCODE_KP_MEMSUBTRACT	},
	{ "KPMemMultiply",		SDL_SCANCODE_KP_MEMMULTIPLY	},
	{ "KPMemDivide",		SDL_SCANCODE_KP_MEMDIVIDE	},
	{ "KPPlusMinus",		SDL_SCANCODE_KP_PLUSMINUS	},
	{ "KPClear",			SDL_SCANCODE_KP_CLEAR		},
	{ "KPClearEntry",		SDL_SCANCODE_KP_CLEARENTRY	},
	{ "KPBinary",			SDL_SCANCODE_KP_BINARY		},
	{ "KPOctal",			SDL_SCANCODE_KP_OCTAL		},
	{ "KPDecimal",			SDL_SCANCODE_KP_DECIMAL		},
	{ "KPHexadecimal",		SDL_SCANCODE_KP_HEXADECIMAL	},
	{ "LeftControl",		SDL_SCANCODE_LCTRL		},
	{ "LeftShift",			SDL_SCANCODE_LSHIFT		},
	{ "LeftAlt",			SDL_SCANCODE_LALT		},
	{ "LeftGUI",			SDL_SCANCODE_LGUI		},
	{ "RightControl",		SDL_SCANCODE_RCTRL		},
	{ "RightShift",			SDL_SCANCODE_RSHIFT		},
	{ "RightAlt",			SDL_SCANCODE_RALT		},
	{ "RGUI",			SDL_SCANCODE_RGUI		},
	{ "Mode",			SDL_SCANCODE_MODE		},
	{ "AudioNext",			SDL_SCANCODE_AUDIONEXT		},
	{ "AudioPrev",			SDL_SCANCODE_AUDIOPREV		},
	{ "AudioStop",			SDL_SCANCODE_AUDIOSTOP		},
	{ "AudioPlay",			SDL_SCANCODE_AUDIOPLAY		},
	{ "AudioMute",			SDL_SCANCODE_AUDIOMUTE		},
	{ "MediaSelect",		SDL_SCANCODE_MEDIASELECT	},
	{ "WWW",			SDL_SCANCODE_WWW		},
	{ "Mail",			SDL_SCANCODE_MAIL		},
	{ "Calculator",			SDL_SCANCODE_CALCULATOR		},
	{ "Computer",			SDL_SCANCODE_COMPUTER		},
	{ "ACSearch",			SDL_SCANCODE_AC_SEARCH		},
	{ "ACHome",			SDL_SCANCODE_AC_HOME		},
	{ "ACBack",			SDL_SCANCODE_AC_BACK		},
	{ "ACForward",			SDL_SCANCODE_AC_FORWARD		},
	{ "ACStop",			SDL_SCANCODE_AC_STOP		},
	{ "ACRefresh",			SDL_SCANCODE_AC_REFRESH		},
	{ "ACBookmarks",		SDL_SCANCODE_AC_BOOKMARKS	},
	{ "BrightnessDown",		SDL_SCANCODE_BRIGHTNESSDOWN	},
	{ "BrightnessUp",		SDL_SCANCODE_BRIGHTNESSUP	},
	{ "DisplaySwitch",		SDL_SCANCODE_DISPLAYSWITCH	},
	{ "KBDIllumToggle",		SDL_SCANCODE_KBDILLUMTOGGLE	},
	{ "KBDIllumDown",		SDL_SCANCODE_KBDILLUMDOWN	},
	{ "KBDIllumUp",			SDL_SCANCODE_KBDILLUMUP		},
	{ "Eject",			SDL_SCANCODE_EJECT		},
	{ "Sleep",			SDL_SCANCODE_SLEEP		},
	{ "App1",			SDL_SCANCODE_APP1		},
	{ "App2",			SDL_SCANCODE_APP2		},
	{ NULL,				-1				}
};

/*
 * SDL.keymod
 */
const CommonEnum KeyboardModifiers[] = {
	{ "None",			KMOD_NONE			},
	{ "LeftShift",			KMOD_LSHIFT			},
	{ "RightShift",			KMOD_RSHIFT			},
	{ "LeftControl",		KMOD_LCTRL			},
	{ "RightControl",		KMOD_RCTRL			},
	{ "LeftAlt",			KMOD_LALT			},
	{ "RightAlt",			KMOD_RALT			},
	{ "LGUI",			KMOD_LGUI			},
	{ "RGUI",			KMOD_RGUI			},
	{ "Num",			KMOD_NUM			},
	{ "Caps",			KMOD_CAPS			},
	{ "Mode",			KMOD_MODE			},
	{ NULL,				-1				}
};
