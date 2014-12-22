/*
 * mixer.c -- main SDL_mixer (2.0) module
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

#include <common/common.h>
#include <common/rwops.h>

#include <SDL_mixer.h>

/* ---------------------------------------------------------
 * Mix_Chunk object
 * --------------------------------------------------------- */

#define MixChunk	MixChunkObject.name

static const CommonObject MixChunkObject;

/*
 * Chunk:volume(volume)
 *
 * Arguments:
 *	volume the volume
 *
 * Returns:
 *	The previous volume
 */
static int
l_chunk_volume(lua_State *L)
{
	Mix_Chunk *c	= commonGetAs(L, 1, MixChunk, Mix_Chunk *);
	int volume	= luaL_checkinteger(L, 2);

	return commonPush(L, "i", Mix_VolumeChunk(c, volume));
}

/*
 * Chunk:playChannel(channel, loops, ticks)
 *
 * Arguments:
 *	channel the channel
 *	loops the number of iterations
 *	ticks the delay in milliseconds
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_chunk_playChannel(lua_State *L)
{
	Mix_Chunk *c	= commonGetAs(L, 1, MixChunk, Mix_Chunk *);
	int channel	= luaL_checkinteger(L, 2);
	int loops	= luaL_optinteger(L, 3, -1);
	int ticks	= luaL_optinteger(L, 4, -1);

	if (Mix_PlayChannelTimed(channel, c, loops, ticks) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Chunk:fadeInChannel(channel, loops, delay, ticks)
 *
 * Arguments:
 *	channel the channel
 *	loops the number of loops
 *	delay the delay in milliseconds
 *	ticks (optional) the milliseconds limits, default: -1
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_chunk_fadeInChannel(lua_State *L)
{
	Mix_Chunk *c	= commonGetAs(L, 1, MixChunk, Mix_Chunk *);
	int channel	= luaL_checkinteger(L, 2);
	int loops	= luaL_checkinteger(L, 3);
	int ms		= luaL_checkinteger(L, 4);
	int ticks	= luaL_optinteger(L, 5, -1);

	if (Mix_FadeInChannelTimed(channel, c, loops, ms, ticks) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Chunk:__gc()
 */
static int
l_chunk_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, MixChunk);

	if (udata->mustdelete)
		Mix_FreeChunk(udata->data);

	return 0;
}

static const luaL_Reg MixMethods[] = {
	{ "volume",		l_chunk_volume				},
	{ "playChannel",	l_chunk_playChannel			},
	{ "fadeInChannel",	l_chunk_fadeInChannel			},
	{ NULL,			NULL					}
};

static const luaL_Reg MixMetamethods[] = {
	{ "__gc",		l_chunk_gc				},
	{ NULL,			NULL					}
};

static const CommonObject MixChunkObject = {
	"Chunk",
	MixMethods,
	MixMetamethods
};

/* ---------------------------------------------------------
 * Mix_Music object
 * --------------------------------------------------------- */

#define MixMusicName		MixMusic.name

static const CommonObject	MixMusic;

/*
 * mixer.type
 */
static const CommonEnum MusicType[] = {
	{ "None",		MUS_NONE				},
	{ "WAV",		MUS_WAV					},
	{ "MOD",		MUS_MOD					},
	{ "MID",		MUS_MID					},
	{ "OGG",		MUS_OGG					},
	{ "MP3",		MUS_MP3					},
	{ NULL,			-1					}
};

/*
 * Music:play(loops)
 *
 * Arguments:
 *	loops (optional) the number of loops, default: -1
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_music_play(lua_State *L)
{
	Mix_Music *m = commonGetAs(L, 1, MixMusicName, Mix_Music *);
	int loops = luaL_optinteger(L, 2, -1);

	if (Mix_PlayMusic(m, loops) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Music:fadeIn(loops, delay, position)
 *
 * Arguments:
 *	loops number of iterations
 *	delay the delay in milliseconds
 *	position (optional) the position
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_music_fadeIn(lua_State *L)
{
	Mix_Music *m	= commonGetAs(L, 1, MixMusicName, Mix_Music *);
	int loops	= luaL_checkinteger(L, 2);
	int ms		= luaL_checkinteger(L, 3);
	double pos;

	/* if we have four arguments, we use Mix_FadeInMusicPos instead */
	if (lua_gettop(L) >= 4) {
		pos = luaL_checknumber(L, 4);

		if (Mix_FadeInMusicPos(m, loops, ms, pos) < 0)
			return commonPushSDLError(L, 1);
	} else if (Mix_FadeInMusic(m, loops, ms) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Music:volume(volume)
 *
 * Arguments:
 *	volume the volume
 *
 * Returns
 *	The previous volume
 */
static int
l_music_volume(lua_State *L)
{
	return commonPush(L, "i", Mix_VolumeMusic(luaL_checkinteger(L, 1)));
}

/*
 * Music:resume()
 */
static int
l_music_resume(lua_State *L)
{
	Mix_ResumeMusic();

	(void)L;

	return 0;
}

/*
 * Music:rewind()
 */
static int
l_music_rewind(lua_State *L)
{
	Mix_RewindMusic();

	(void)L;

	return 0;
}

/*
 * Music:setPosition(pos)
 *
 * Arguments:
 *	pos the position
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_music_setPosition(lua_State *L)
{
	if (Mix_SetMusicPosition(luaL_checknumber(L, 1)) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Music:halt()
 */
static int
l_music_halt(lua_State *L)
{
	Mix_HaltMusic();

	(void)L;

	return 0;
}

/*
 * Music:fadeOut(delay)
 *
 * Arguments:
 *	delay the delay in milliseconds
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_music_fadeOut(lua_State *L)
{
	if (Mix_FadeOutMusic(luaL_checkinteger(L, 1)) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * Music:type()
 *
 * Returns:
 *	The music type (mixer.type)
 */
static int
l_music_getType(lua_State *L)
{
	return commonPush(L, "i", Mix_GetMusicType(commonGetAs(L, 1, MixMusicName, Mix_Music *)));
}

/*
 * Music:playing()
 *
 * Returns:
 *	True if playing
 */
static int
l_music_playing(lua_State *L)
{
	return commonPush(L, "b", Mix_PlayingMusic());
}

/*
 * Music:paused()
 *
 * Returns:
 *	True if paused
 */
static int
l_music_paused(lua_State *L)
{
	return commonPush(L, "b", Mix_PausedMusic());
}

/*
 * Music:fading()
 *
 * Returns:
 *	The fading status
 */
static int
l_music_fading(lua_State *L)
{
	return commonPush(L, "i", Mix_FadingMusic());
}

/*
 * Music:__gc()
 */
static int
l_music_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, MixMusicName);

	if (udata->mustdelete)
		Mix_FreeMusic(udata->data);

	return 0;
}

static const luaL_Reg MusicMethods[] = {
	{ "play",		l_music_play				},
	{ "fadeIn",		l_music_fadeIn				},
	{ "volume",		l_music_volume				},
	{ "resume",		l_music_resume				},
	{ "rewind",		l_music_rewind				},
	{ "setPosition",	l_music_setPosition			},
	{ "halt",		l_music_halt				},
	{ "fadeOut",		l_music_fadeOut				},
	{ "getType",		l_music_getType				},
	{ "playing",		l_music_playing				},
	{ "paused",		l_music_paused				},
	{ "fading",		l_music_fading				},
	{ NULL,			NULL					}
};

static const luaL_Reg MusicMetamethods[] = {
	{ "__gc",		l_music_gc				},
	{ NULL,			NULL					}
};

static const CommonObject Music = {
	"Music",
	MusicMethods,
	MusicMetamethods
};

/* ---------------------------------------------------------
 * Private helpers
 * --------------------------------------------------------- */

typedef int (*GroupFunction)(int);

/*
 * Some group functions have exactly the same signature. Use this function which
 * return the integer value.
 */
static int
groupFunction(lua_State *L, GroupFunction func)
{
	return commonPush(L, "i", func(luaL_optinteger(L, 1, -1)));
}

/* ---------------------------------------------------------
 * SDL_mixer functions
 * --------------------------------------------------------- */

/*
 * mixer.flags
 */
static const CommonEnum MixerFlags[] = {
	{ "FLAC",			MIX_INIT_FLAC			},
	{ "MOD",			MIX_INIT_MOD			},
	{ "MP3",			MIX_INIT_MP3			},
	{ "OGG",			MIX_INIT_OGG			},
	{ NULL,				-1				}
};

/*
 * mixer.fading
 */
static const CommonEnum MixerFading[] = {
	{ "None",			MIX_NO_FADING			},
	{ "Out",			MIX_FADING_OUT			},
	{ "In",				MIX_FADING_IN			},
	{ NULL,				-1				}
};

/*
 * mixer.init(flags)
 *
 * Arguments:
 *	flags the flags (mixer.flags)
 *
 * Returns:
 *	The same table or the new one
 *	True on success or nil on failure
 *	The error message
 */
static int
l_mixer_init(lua_State *L)
{
	int flags = commonGetEnum(L, 1);
	int ret;

	ret = Mix_Init(flags);
	commonPushEnum(L, ret, MixerFlags);

	if ((ret & flags) != flags)
		return commonPush(L, "n s", Mix_GetError()) + 1;

	return commonPush(L, "b", 1) + 1;
}

/*
 * mixer.openAudio(frequency, format, channels, chksize)
 *
 * Arguments:
 *	frequency the frequency
 *	format the format (SDL.audioFormat)
 *	channels the number of channels
 *	chksize the chunk size
 *
 * Returns:
 *	True on success or nil on failure
 *	The error message
 */
static int
l_mixer_openAudio(lua_State *L)
{
	int frequency	= luaL_checkinteger(L, 1);
	int format	= luaL_checkinteger(L, 2);
	int channels	= luaL_checkinteger(L, 3);
	int chksize	= luaL_checkinteger(L, 4);

	if (Mix_OpenAudio(frequency, format, channels, chksize) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * mixer.getNumChunkDecoders()
 *
 * Returns:
 *	The number of chunk decoders
 */
static int
l_mixer_getNumChunkDecoders(lua_State *L)
{
	return commonPush(L, "i", Mix_GetNumChunkDecoders());
}

/*
 * mixer.getChunkDecoder(index)
 *
 * Arguments:
 *	index (optional) the index, default: 0
 *
 * Returns:
 *	The decoder name
 */
static int
l_mixer_getChunkDecoder(lua_State *L)
{
	return commonPush(L, "s", Mix_GetChunkDecoder(luaL_optinteger(L, 1, 0)));
}

/*
 * mixer.loadWAV(path)
 *
 * Arguments:
 *	path the path
 *
 * Returns:
 *	The chunk object or nil on failure
 *	The error message
 */
static int
l_mixer_loadWAV(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	Mix_Chunk *c;

	if ((c = Mix_LoadWAV(path)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", MixChunk, c);
}

/*
 * mixer.loadWAV_RW(rw)
 *
 * Arguments:
 *	rw the RWops
 *
 * Returns:
 *	The chunk object or nil on failure
 *	The error message
 */
static int
l_mixer_loadWAV_RW(lua_State *L)
{
	SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);
	Mix_Chunk *c;

	if ((c = Mix_LoadWAV_RW(ops, 0)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", MixChunk, c);
}

/*
 * mixer.allocateChannels(num)
 *
 * Arguments:
 *	num the number to allocate
 *
 * Returns:
 *	The number allocated
 */
static int
l_mixer_allocateChannels(lua_State *L)
{
	int num = luaL_checkinteger(L, 1);

	return commonPush(L, "i", Mix_AllocateChannels(num));
}

/*
 * mixer.volume(channel, volume)
 *
 * Arguments:
 *	channel the channel
 *	volume the volume
 *
 * Returns:
 *	The current volume
 */
static int
l_mixer_volume(lua_State *L)
{
	int channel	= luaL_checkinteger(L, 1);
	int volume	= luaL_checkinteger(L, 2);

	return commonPush(L, "i", Mix_Volume(channel, volume));
}

/*
 * mixer.pause(channel)
 *
 * Arguments:
 *	channel the channel
 */
static int
l_mixer_pause(lua_State *L)
{
	Mix_Pause(luaL_checkinteger(L, 1));

	return 0;
}

/*
 * mixer.resume(channel)
 *
 * Arguments:
 *	channel the channel
 */
static int
l_mixer_resume(lua_State *L)
{
	Mix_Resume(luaL_checkinteger(L, 1));

	return 0;
}

/*
 * mixer.haltChannel(channel)
 *
 * Arguments:
 *	channel (optional) the channel, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_haltChannel(lua_State *L)
{
	return commonPush(L, "i", Mix_HaltChannel(luaL_checkinteger(L, 1)));
}

/*
 * mixer.expireChannel(channel, delay)
 *
 * Arguments:
 *	channel the channel
 *	delay the delay in milliseconds
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_expireChannel(lua_State *L)
{
	int channel	= luaL_checkinteger(L, 1);
	int ticks	= luaL_checkinteger(L, 2);

	return commonPush(L, "i", Mix_ExpireChannel(channel, ticks));
}

/*
 * mixer.fadeOutChannels(channel, ms)
 *
 * Arguments:
 *	channel the channel
 *	ms the number in milliseconds
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_fadeOutChannel(lua_State *L)
{
	int channel	= luaL_checkinteger(L, 1);
	int ms		= luaL_checkinteger(L, 2);

	return commonPush(L, "i", Mix_FadeOutChannel(channel, ms));
}

/*
 * mixer.playing(channel)
 *
 * Arguments:
 *	channel (optional) the channel, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_playing(lua_State *L)
{
	return commonPush(L, "i", Mix_Playing(luaL_optinteger(L, 1, -1)));
}

/*
 * mixer.paused(channel)
 *
 * Arguments:
 *	channel (optional) the channel, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_paused(lua_State *L)
{
	return commonPush(L, "i", Mix_Paused(luaL_optinteger(L, 1, -1)));
}

/*
 * mixer.fadingChannel(channel)
 *
 * Arguments:
 *	channel the channel
 *
 * Returns:
 *	The status
 */
static int
l_mixer_fadingChannel(lua_State *L)
{
	int channel = luaL_checkinteger(L, 1);

	if (channel < 0)
		return luaL_error(L, "invalid channel value: %d", channel);

	return commonPush(L, "i", Mix_FadingChannel(channel));
}

/*
 * mixer.reserveChannels(num)
 *
 * Arguments:
 *	num the number of channels to allocate
 *
 * Returns:
 *	The number allocated
 */
static int
l_mixer_reserveChannels(lua_State *L)
{
	return commonPush(L, "i", Mix_ReserveChannels(luaL_checkinteger(L, 1)));
}

/*
 * mixer.groupChannel(channel, tag)
 *
 * Arguments:
 *	channel the channel number
 *	tag (optional) the tag number, default: -1
 *
 * Returns:
 *	True on success
 */
static int
l_mixer_groupChannel(lua_State *L)
{
	int which	= luaL_checkinteger(L, 1);
	int tag		= luaL_optinteger(L, 2, -1);

	return commonPush(L, "b", Mix_GroupChannel(which, tag));
}

/*
 * mixer.groupChannels(from, to, tag)
 *
 * Arguments:
 *	from the minimum
 *	to the maximum
 *	tag the tag number
 *
 * Returns:
 *	The number affected
 */
static int
l_mixer_groupChannels(lua_State *L)
{
	int from	= luaL_checkinteger(L, 1);
	int to		= luaL_checkinteger(L, 2);
	int tag		= luaL_optinteger(L, 3, -1);

	return commonPush(L, "i", Mix_GroupChannels(from, to, tag));
}

/*
 * mixer.groupCount(tag)
 *
 * Arguments:
 *	tag (optional) the tag, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_groupCount(lua_State *L)
{
	return groupFunction(L, Mix_GroupCount);
}

/*
 * mixer.groupAvailable(tag)
 *
 * Arguments:
 *	tag (optional) the tag, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_groupAvailable(lua_State *L)
{
	return groupFunction(L, Mix_GroupAvailable);
}

/*
 * mixer.groupOldest(tag)
 *
 * Arguments:
 *	tag (optional) the tag, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_groupOldest(lua_State *L)
{
	return groupFunction(L, Mix_GroupOldest);
}

/*
 * mixer.groupNewer(tag)
 *
 * Arguments:
 *	tag (optional) the tag, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_groupNewer(lua_State *L)
{
	return groupFunction(L, Mix_GroupNewer);
}

/*
 * mixer.fadeOutGroup(tag, ms)
 *
 * Arguments:
 *	tag the group number
 *	ms the number of milliseconds
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_fadeOutGroup(lua_State *L)
{
	int tag	= luaL_checkinteger(L, 1);
	int ms	= luaL_checkinteger(L, 2);

	return commonPush(L, "i", Mix_FadeOutGroup(tag, ms));
}

/*
 * mixer.haltGroup(tag)
 *
 * Arguments:
 *	tag (optional) the tag, default: -1
 *
 * Returns:
 *	The number of channels affected
 */
static int
l_mixer_haltGroup(lua_State *L)
{
	return groupFunction(L, Mix_HaltGroup);
}

/*
 * mixer.getNumMusicDecoders()
 *
 * Returns:
 *	The number of music decoders
 */
static int
l_mixer_getNumMusicDecoders(lua_State *L)
{
	return commonPush(L, "i", Mix_GetNumMusicDecoders());
}

/*
 * mixer.getMusicDecoder(index)
 *
 * Arguments:
 *	index the decoder index
 *
 * Returns:
 *	The music decoder name
 */
static int
l_mixer_getMusicDecoder(lua_State *L)
{
	return commonPush(L, "s", Mix_GetMusicDecoder(luaL_checkinteger(L, 1)));
}

/*
 * mixer.loadMUS(path)
 *
 * Arguments:
 *	path the path to the sound
 *
 * Returns:
 *	The music object or nil on failure
 *	The error message
 */
static int
l_mixer_loadMUS(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	Mix_Music *music;

	if ((music = Mix_LoadMUS(path)) == NULL)
		return commonPushSDLError(L, 1);

	(void)L;

	return commonPush(L, "p", MixMusicName, music);
}

/*
 * mixer.closeAudio()
 */
static int
l_mixer_closeAudio(lua_State *L)
{
	Mix_CloseAudio();

	(void)L;

	return 0;
}

/*
 * mixer.quit()
 */
static int
l_mixer_quit(lua_State *L)
{
	Mix_Quit();

	(void)L;

	return 0;
}

static const luaL_Reg MixerFunctions[] = {
	{ "init",			l_mixer_init			},
	{ "openAudio",			l_mixer_openAudio		},
	{ "getNumChunkDecoders",	l_mixer_getNumChunkDecoders	},
	{ "getChunkDecoder",		l_mixer_getChunkDecoder		},
	{ "loadWAV",			l_mixer_loadWAV			},
	{ "loadWAV_RW",			l_mixer_loadWAV_RW		},
	{ "allocateChannels",		l_mixer_allocateChannels	},
	{ "volume",			l_mixer_volume			},
	{ "pause",			l_mixer_pause			},
	{ "resume",			l_mixer_resume			},
	{ "haltChannel",		l_mixer_haltChannel		},
	{ "expireChannel",		l_mixer_expireChannel		},
	{ "fadeOutChannel",		l_mixer_fadeOutChannel		},
	{ "playing",			l_mixer_playing			},
	{ "paused",			l_mixer_paused			},
	{ "fadingChannel",		l_mixer_fadingChannel		},
	{ "reserveChannels",		l_mixer_reserveChannels		},
	{ "groupChannel",		l_mixer_groupChannel		},
	{ "groupChannels",		l_mixer_groupChannels		},
	{ "groupCount",			l_mixer_groupCount		},
	{ "groupAvailable",		l_mixer_groupAvailable		},
	{ "groupOldest",		l_mixer_groupOldest		},
	{ "groupNewer",			l_mixer_groupNewer		},
	{ "fadeOutGroup",		l_mixer_fadeOutGroup		},
	{ "haltGroup",			l_mixer_haltGroup		},
	{ "getNumMusicDecoders",	l_mixer_getNumMusicDecoders	},
	{ "getMusicDecoder",		l_mixer_getMusicDecoder		},
	{ "loadMUS",			l_mixer_loadMUS			},
	{ "closeAudio",			l_mixer_closeAudio		},
	{ "quit",			l_mixer_quit			},
	{ NULL,				NULL				}
};

int EXPORT
luaopen_SDL_mixer(lua_State *L)
{
	commonNewLibrary(L, MixerFunctions);

	commonBindEnum(L, -1, "flags", MixerFlags);
	commonBindEnum(L, -1, "fading", MixerFading);
	commonBindEnum(L, -1, "type", MusicType);

	/* Mix_Chunk object */
	commonBindObject(L, &MixChunkObject);

	/* Mix_Music object */
	commonBindObject(L, &Music);

	return 1;
}
