/*
 * audio.c -- audio playback and recording
 *
 * Copyright (c) 2013, 2014 David Demelier <markand@malikania.fr>
 * Copyright (c) 2016 Webster Sheets <webster@web-eworks.com>
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
#include <errno.h>

#include <common/rwops.h>
#include <common/table.h>

#include "audio.h"
#include "thread.h"

/*
 * Wrapper used to store the device information and its Lua state. SDL
 * uses a thread for the audio callback so we need to use the Lua callback
 * safely in a new Lua state.
 *
 * This structure is used as an object orientation for both legacy APIs
 * and new ones.
 */
typedef struct {
	SDL_bool		 isdevice;	/* true if SDL_OpenAudioDevice was used */
	SDL_AudioSpec		 desired;	/* requested */
	SDL_AudioSpec		 obtained;	/* obtained */

	/* Lua state and callbacks */
	lua_State		*L;		/* lua_State */
	int			 callback;	/* the Lua function callback */

	/* These fields are only used if isdevice is true */
	const char		 *name;		/* device name */
	SDL_AudioDeviceID	 id;		/* the device id */
	SDL_bool		 iscapture;	/* true for capture */
	SDL_bool		 allowchanges;	/* true to allow changes */
} AudioDevice;

static void
audioCallback(AudioDevice *device, Uint8 *stream, int length)
{
	lua_rawgeti(device->L, LUA_REGISTRYINDEX, device->callback);
	lua_pushinteger(device->L, length);

	if (lua_pcall(device->L, 1, 1, 0) != LUA_OK) {
		SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", lua_tostring(device->L, -1));
		lua_pop(device->L, 1);
		memset(stream, 0, length);
	} else {
		if (lua_type(device->L, -1) != LUA_TSTRING)
			memset(stream, 0, length);
		else {
			size_t strl;
			const char *str = lua_tolstring(device->L, -1, &strl);

			/* Copy to stream */
			memcpy(stream, str, (strl >= (size_t)length) ? (size_t)length : strl);
		}
	}
}

/*
 * Returns a table with the following fields:
 *	data, the raw buffer string
 *	length, the data length
 *	format, the format (SDL.audioFormat)
 *	frequency, the frequency
 *	channels, the number of channels
 *	samples, the number of samples
 */
static int
loadWAV(lua_State *L, int use_rw)
{
	SDL_AudioSpec spec;
	Uint8 *buffer;
	Uint32 length;

	if (!use_rw) {
		const char *path = luaL_checkstring(L, 1);

		if (SDL_LoadWAV(path, &spec, &buffer, &length) == NULL)
			return commonPushSDLError(L, 1);
	} else {
		SDL_RWops *ops = commonGetAs(L, 1, RWOpsName, SDL_RWops *);

		if (SDL_LoadWAV_RW(ops, 0, &spec, &buffer, &length) == NULL)
			return commonPushSDLError(L, 1);
	}

	lua_createtable(L, 0, 0);
	tableSetStringl(L, -1, "data", (const char *)buffer, length);
	tableSetInt(L, -1, "length", length);
	tableSetInt(L, -1, "format", spec.format);
	tableSetInt(L, -1, "frequency", spec.freq);
	tableSetInt(L, -1, "channels", spec.channels);
	tableSetInt(L, -1, "samples", spec.samples);

	return 1;
}

static int
mixAudio(lua_State *L, int use_format)
{
	size_t length;
	const char *src = luaL_checklstring(L, 1, &length);
	int volume = SDL_MIX_MAXVOLUME;
	int format;
	char *data;

	if (use_format) {
		format = luaL_checkinteger(L, 2);

		if (lua_gettop(L) >= 3)
			volume = luaL_checkinteger(L, 3);
	} else if (lua_gettop(L) >= 2)
		volume = luaL_checkinteger(L, 2);

	if ((data = calloc(1, length)) == NULL)
		return commonPushSDLError(L, 1);

	if (!use_format)
		SDL_MixAudio((Uint8 *)data, (const Uint8 *)src, length, volume);
	else
		SDL_MixAudioFormat((Uint8 *)data, (const Uint8 *)src, format, length, volume);

	lua_pushlstring(L, data, length);
	free(data);

	return 1;
}

/*
 * The table params may or must have the following fields:
 *	callback the function or file to call
 *	iscapture (optional) request for capture
 *	allowchanges (optional) set to true to allow  format changes
 *	device (optional) the device name
 *	frequency (optional) the frequency
 *	format (optional) the format (SDL.audioFormat)
 *	channels (optional) number of channels
 *	samples (optional) number of samples
 *
 * The callback function must have the following signature:
 *	func(length) -> return the stream
 *
 * NOTE: The callback function is running in a separate thread, you must use
 *	 channels to share data.
 */
static int
openAudio(lua_State *L, int isdevice)
{
	AudioDevice *device;

	/* Must be table */
	luaL_checktype(L, 1, LUA_TTABLE);

	if ((device = calloc(1, sizeof (AudioDevice))) == NULL)
		return commonPushSDLError(L, 1);

	/* Prepare Lua */
	device->L = luaL_newstate();
	luaL_openlibs(device->L);

	device->isdevice		= isdevice;
	device->desired.userdata	= device;
	device->desired.freq		= tableGetInt(L, 1, "frequency");
	device->desired.format		= tableGetInt(L, 1, "format");
	device->desired.channels	= tableGetInt(L, 1, "channels");
	device->desired.samples		= tableGetInt(L, 1, "samples");
	device->desired.callback	= (SDL_AudioCallback)audioCallback;

	if (isdevice) {
		/* Get standard parameters */
		device->iscapture	= tableGetBool(L, 1, "iscapture");
		device->allowchanges	= tableGetBool(L, 1, "allowchanges");

		/* Other parameters */
		if (tableIsType(L, 1, "device", LUA_TSTRING))
			device->name = luaL_checkstring(L, 1);
	}

	/*
	 * The threadDump function dump a Lua function and pushes it to a new
	 * Lua state. We take this function and set it to the registry so we
	 * can call it as many times as necessary.
	 *
	 * If the function fails, it already pushed nil and the error on L.
	 */
	if (tableIsType(L, 1, "callback", LUA_TSTRING)) {
		if (luaL_dofile(device->L, tableGetString(L, 1, "callback")) != LUA_OK) {
			commonPush(L, "ns", lua_tostring(device->L, -1));
			goto fail;
		}

		if (lua_type(device->L, -1) != LUA_TFUNCTION) {
			commonPush(L, "ns", "must return a function");
			goto fail;
		}

		device->callback = luaL_ref(device->L, LUA_REGISTRYINDEX);
	} else {
		commonPush(L, "ns", "callback must be a path to a file");
		goto fail;
	}

	/* Finally open the device */
	if (device->isdevice) {
		device->id = SDL_OpenAudioDevice(device->name, device->iscapture,
		    &device->desired, &device->obtained, device->allowchanges);

		if (device->id == 0) {
			commonPushSDLError(L, 1);
			goto fail;
		}
	} else {
		if (SDL_OpenAudio(&device->desired, &device->obtained) < 0) {
			commonPushSDLError(L, 1);
			goto fail;
		}
	}

	return commonPush(L, "p", AudioDeviceName, device);

fail:
	if (device->callback != LUA_REFNIL)
		luaL_unref(L, LUA_REGISTRYINDEX, device->callback);
	if (device->L != NULL)
		lua_close(device->L);

	free(device);

	return 2;
}

void
audioPushCVT(lua_State *L, const SDL_AudioCVT *cvt)
{
	lua_createtable(L, 9, 9);

	tableSetBool(L, -1, "needed", 1);
	tableSetInt(L, -1, "sourceFormat", cvt->src_format);
	tableSetInt(L, -1, "destFormat", cvt->dst_format);
	tableSetDouble(L, -1, "rateIncrement", cvt->rate_incr);
	tableSetInt(L, -1, "lengthBuffer", cvt->len);
	tableSetInt(L, -1, "lengthConverted", cvt->len_cvt);
	tableSetInt(L, -1, "lengthMult", cvt->len_mult);
	tableSetDouble(L, -1, "lengthRatio", cvt->len_ratio);

	/* The buffer is set as a string */
	lua_pushlstring(L, (const char *)cvt->buf, cvt->len);
	lua_setfield(L, -2, "data");
}

int
audioGetCVT(lua_State *L, int index, SDL_AudioCVT *cvt)
{
	int srcformat, srcchannels, srcrate;
	int dstformat, dstchannels, dstrate;
	int ret;
	size_t length;
	const char *str;

	luaL_checktype(L, index, LUA_TTABLE);

	srcformat	= tableGetInt(L, index, "sourceFormat");
	srcchannels	= tableGetInt(L, index, "sourceChannels");
	srcrate		= tableGetInt(L, index, "sourceRate");
	dstformat	= tableGetInt(L, index, "destFormat");
	dstchannels	= tableGetInt(L, index, "destChannels");
	dstrate		= tableGetInt(L, index, "destRate");

	ret = SDL_BuildAudioCVT(cvt, srcformat, srcchannels, srcrate,
	    dstformat, dstchannels, dstrate);

	if (ret < 0)
		return commonPushSDLError(L, 1);

	if (!tableIsType(L, index, "data", LUA_TSTRING))
		return luaL_error(L, "field data must be a string");

	str = tableGetStringl(L, index, "data", &length);

	/* Copy it, will be freed in SDL_ConvertAudio */
	cvt->buf	= malloc(length);
	cvt->len	= length;

	if (!cvt->buf)
		return commonPushSDLError(L, 1);

	memcpy(cvt->buf, str, length);

	return 0;
}

/* --------------------------------------------------------
 * Audio functions
 * -------------------------------------------------------- */

/*
 * SDL.audioInit(name)
 *
 * Arguments:
 *	name the driver name
 *
 * Returns:
 *	True on success or false
 *	The error message
 */
static int
l_audioInit(lua_State *L)
{
	const char *dvname = luaL_checkstring(L, 1);

	if (SDL_AudioInit(dvname) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

/*
 * SDL.audioQuit()
 */
static int
l_audioQuit(lua_State *L)
{
	SDL_AudioQuit();

	(void)L;

	return 0;
}

/*
 * SDL.audioConvert(input)
 *
 * Takes a table with the following fields:
 *	sourceFormat	the source format (SDL.audioFormat)
 *	sourceChannels	the number of channels in source
 *	sourceRate	the source frequency
 *	destFormat	the destination format (SDL.audioFormat)
 *	destChannels	the number of channels in destination
 *	destRate	the frequency for destination
 *	data		the string data
 *
 * Returns a table with the following fields:
 *	needed		set to true
 *	sourceFormat	the source format (SDL.audioFormat)
 *	destFormat	the destination format (SDL.audioFormat)
 *	rateIncrement	the frequency infrement
 *	lengthBuffer	the original length
 *	lengthConverted	the converted length
 *	lengthMult	the len_mult field
 *	lengthRatio	the len_ratio field
 *	data		the audio data
 *
 * Arguments:
 *	input		the input CVT
 *
 * Returns:
 *	The converted data or nil on failure
 *	The error message
 */
static int
l_convertAudio(lua_State *L)
{
	SDL_AudioCVT cvt;

	audioGetCVT(L, -1, &cvt);
	if (SDL_ConvertAudio(&cvt) < 0)
		return commonPushSDLError(L, 1);

	audioPushCVT(L, &cvt);
	free(cvt.buf);

	return 1;
}

/*
 * SDL.getAudioDeviceName(index, iscapture)
 *
 * Arguments:
 *	index the index
 *	iscapture set to true for recording
 *
 * Returns:
 *	The name or nil on failure
 *	The error message
 */
static int
l_getAudioDeviceName(lua_State *L)
{
	int index	= luaL_checkinteger(L, 1);
	int iscapture	= lua_toboolean(L, 2);
	const char *name;

	if ((name = SDL_GetAudioDeviceName(index, iscapture)))
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * SDL.getAudioDriver(index)
 *
 * Arguments:
 *	index the index
 *
 * Returns:
 *	The name or nil on failure
 *	The error message
 */
static int
l_getAudioDriver(lua_State *L)
{
	int index = luaL_checkinteger(L, 1);
	const char *name;

	if ((name = SDL_GetAudioDriver(index)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * SDL.getAudioStatus()
 *
 * Returns:
 *	The audio status (SDL.audioStatus)
 */
static int
l_getAudioStatus(lua_State *L)
{
	return commonPush(L, "i", SDL_GetAudioStatus());
}

/*
 * SDL.getCurrentAudioDriver()
 *
 * Returns:
 *	The name or nil on failure
 *	The error message
 */
static int
l_getCurrentAudioDriver(lua_State *L)
{
	const char *name = SDL_GetCurrentAudioDriver();

	if (name == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", name);
}

/*
 * SDL.getNumAudioDevices(iscapture)
 *
 * Arguments:
 *	iscapture set to true for recording devices
 *
 * Returns:
 *	The number of audio devices
 */
static int
l_getNumAudioDevices(lua_State *L)
{
	int iscapture = lua_toboolean(L, 1);

	return commonPush(L, "i", SDL_GetNumAudioDevices(iscapture));
}

/*
 * SDL.getNumAudioDrivers()
 *
 * Returns:
 *	The number of audio drivers
 */
static int
l_getNumAudioDrivers(lua_State *L)
{
	return commonPush(L, "i", SDL_GetNumAudioDrivers());
}

/*
 * SDL.loadWAV(path)
 *
 * Arguments:
 *	path the path to the file
 *
 * Returns:
 *	The table or nil on failure
 *	The error message
 */
static int
l_loadWAV(lua_State *L)
{
	return loadWAV(L, 0);
}

/*
 * SDL.loadWAV_RW(rw)
 *
 * Arguments:
 *	rw the RWops
 *
 * Returns:
 *	The table or nil on failure
 *	The error message
 */
static int
l_loadWAV_RW(lua_State *L)
{
	return loadWAV(L, 1);
}

/*
 * SDL.mixAudioFormat(src, volume)
 *
 * Arguments:
 *	src the data
 *	volume the optional volume
 *
 * Returns:
 *	The mixed buffer or nil on failure
 *	The error message
 */
static int
l_mixAudio(lua_State *L)
{
	return mixAudio(L, 0);
}

/*
 * SDL.mixAudioFormat(src, format, volume)
 *
 * Arguments:
 *	src the data
 *	format the format
 *	volume the optional volume
 *
 * Returns:
 *	The mixed buffer or nil on failure
 *	The error message
 */
static int
l_mixAudioFormat(lua_State *L)
{
	return mixAudio(L, 1);
}

/*
 * SDL.openAudioDevice(params)
 *
 * Arguments:
 *	params the parameters
 *
 * Returns:
 *	The device object or nil on failure
 *	The error message
 */
static int
l_openAudioDevice(lua_State *L)
{
	return openAudio(L, 1);
}

/*
 * SDL.openAudio(spec)
 *
 * Exactly like SDL.openAudioDevice but without the following fields:
 *	iscapture,
 *	device,
 *	allowchanges
 *
 * Arguments:
 *	params the parameters
 *
 * Returns:
 *	The device object or nil on failure
 *	The error message
 */
static int
l_openAudio(lua_State *L)
{
	return openAudio(L, 0);
}

const luaL_Reg AudioFunctions[] = {
	{ "audioInit",			l_audioInit			},
	{ "audioQuit",			l_audioQuit			},
	{ "convertAudio",		l_convertAudio			},
	{ "getAudioDeviceName",		l_getAudioDeviceName		},
	{ "getAudioDriver",		l_getAudioDriver		},
	{ "getAudioStatus",		l_getAudioStatus		},
	{ "getCurrentAudioDriver",	l_getCurrentAudioDriver		},
	{ "getNumAudioDevices",		l_getNumAudioDevices		},
	{ "getNumAudioDrivers",		l_getNumAudioDrivers		},
	{ "loadWAV",			l_loadWAV			},
	{ "loadWAV_RW",			l_loadWAV_RW			},
	{ "mixAudio",			l_mixAudio			},
	{ "mixAudioFormat",		l_mixAudioFormat		},
	{ "openAudio",			l_openAudio			},
	{ "openAudioDevice",		l_openAudioDevice		},
	{ NULL,				NULL				}
};

/*
 * SDL.audioFormat
 */
const CommonEnum AudioFormat[] = {
	/* 8 bits */
	{ "S8",				AUDIO_S8			},
	{ "U8",				AUDIO_U8			},

	/* 16 signed bits */
	{ "S16LSB",			AUDIO_S16LSB			},
	{ "S16MSB",			AUDIO_S16MSB			},
	{ "S16SYS",			AUDIO_S16SYS			},
	{ "S16",			AUDIO_S16			},

	/* 16 unsigned bits */
	{ "U16LSB",			AUDIO_U16LSB			},
	{ "U16MSB",			AUDIO_U16MSB			},
	{ "U16SYS",			AUDIO_U16SYS			},
	{ "U16",			AUDIO_U16			},

	/* 32 signed bits */
	{ "S32LSB", 			AUDIO_S32LSB			},
	{ "S32MSB", 			AUDIO_S32MSB			},
	{ "S32SYS", 			AUDIO_S32SYS			},
	{ "S32", 			AUDIO_S32			},

	/* 32 float bits */
	{ "F32LSB",			AUDIO_F32LSB			},
	{ "F32MSB",			AUDIO_F32MSB			},
	{ "F32SYS",			AUDIO_F32SYS			},
	{ "F32",			AUDIO_F32			},
	{ NULL,				-1				}
};

/*
 * SDL.audioStatus
 */
const CommonEnum AudioStatus[] = {
	{ "Stopped",			SDL_AUDIO_STOPPED		},
	{ "Playing",			SDL_AUDIO_PLAYING		},
	{ "Paused",			SDL_AUDIO_PAUSED		},
	{ NULL,				-1				}
};

/* --------------------------------------------------------
 * Audio device object methods
 * -------------------------------------------------------- */

/*
 * AudioDevice:lock()
 */
static int
l_audiodev_lock(lua_State *L)
{
	AudioDevice *dev = commonGetAs(L, 1, AudioDeviceName, AudioDevice *);

	if (dev->isdevice)
		SDL_LockAudioDevice(dev->id);
	else
		SDL_LockAudio();

	return 0;
}

/*
 * AudioDevice:status()
 *
 * Returns:
 *	The status
 */
static int
l_audiodev_status(lua_State *L)
{
	AudioDevice *dev = commonGetAs(L, 1, AudioDeviceName, AudioDevice *);
	int status;

	if (dev->isdevice)
		status = SDL_GetAudioDeviceStatus(dev->id);
	else
		status = SDL_GetAudioStatus();

	return commonPush(L, "i", status);
}

/*
 * AudioDevice:unlock()
 */
static int
l_audiodev_unlock(lua_State *L)
{
	AudioDevice *dev = commonGetAs(L, 1, AudioDeviceName, AudioDevice *);

	if (dev->isdevice)
		SDL_UnlockAudioDevice(dev->id);
	else
		SDL_UnlockAudio();

	return 0;
}

/*
 * AudioDevice:pause(mode)
 */
static int
l_audiodev_pause(lua_State *L)
{
	AudioDevice *dev	= commonGetAs(L, 1, AudioDeviceName, AudioDevice *);
	int pause		= lua_toboolean(L, 2);

	if (dev->isdevice)
		SDL_PauseAudioDevice(dev->id, pause);
	else
		SDL_PauseAudio(pause);

	return 0;
}

#if SDL_VERSION_ATLEAST(2, 0, 4)

/*
 * AudioDevice:queue(data)
 */
static int
l_audiodev_queue(lua_State *L)
{
	AudioDevice *dev	= commonGetAs(L, 1, AudioDeviceName, AudioDevice *);
	size_t len;
	const char *data	= luaL_checklstring(L, 2, &len);

	if (dev->isdevice) {
		if (SDL_QueueAudio(dev->id, (void *)data, len) < 0)
			return commonPushSDLError(L, 1);
		else
			return commonPush(L, "b", 1);
	}
	else {
		return commonPush(L, "ns", "Must be an AudioDevice (opened with SDL.openAudioDevice).");
	}
}

#if SDL_VERSION_ATLEAST(2, 0, 5)
/*
 * AudioDevice:dequeue(len)
 *
 * Pull (dequeue) audio data from capture devices.
 * NOTE: This function DOES NOT operate on playback devices. This
 * is audio INPUT, not output.
 *
 * Arguments:
 *	The number of bytes of audio data to retrieve.
 *
 * Returns:
 *	The dequeued audio data, as a string.
 *	The number of bytes actually queued.
 *
 */
static int
l_audiodev_dequeue(lua_State *L)
{
	AudioDevice *dev	= commonGetAs(L, 1, AudioDeviceName, AudioDevice *);
	size_t len		= luaL_checkinteger(L, 2);

	if (dev->isdevice && dev->iscapture) {
		void *data;
		if ((data = malloc(len)) == NULL)
			return commonPushErrno(L, 1);

		len = SDL_DequeueAudio(dev->id, data, len);

		lua_pushlstring(L, (const char *)data, len);
		lua_pushinteger(L, len);

		free(data);

		return 2;
	}
	else {
		return commonPush(L, "ns", "Must be a capture AudioDevice (opened with SDL.openAudioDevice).");
	}
}
#endif

/*
 * AudioDevice:clearQueued()
 */
static int
l_audiodev_clearQueued(lua_State *L)
{
	AudioDevice *dev	= commonGetAs(L, 1, AudioDeviceName, AudioDevice *);

	if (dev->isdevice) {
		SDL_ClearQueuedAudio(dev->id);
		return commonPush(L, "b", 1);
	}
	else{
		return commonPush(L, "ns", "No Audio Device ID present.");
	}
}

/*
 * AudioDevice:getQueuedSize()
 */
static int
l_audiodev_getQueuedSize(lua_State *L)
{
	AudioDevice *dev	= commonGetAs(L, 1, AudioDeviceName, AudioDevice *);

	if (dev->isdevice) {
		return commonPush(L, "i", SDL_GetQueuedAudioSize(dev->id));
	}
	else{
		return commonPush(L, "ns", "No Audio Device ID present.");
	}
}

#endif

/* --------------------------------------------------------
 * Audio device object metamethods
 * -------------------------------------------------------- */

/*
 * AudioDevice:__eq()
 */
static int
l_audiodev_eq(lua_State *L)
{
	AudioDevice *dev1 = commonGetAs(L, 1, AudioDeviceName, AudioDevice *);
	AudioDevice *dev2 = commonGetAs(L, 1, AudioDeviceName, AudioDevice *);

	return commonPush(L, "b", dev1->id == dev2->id);
}

/*
 * AudioDevice:__gc()
 */
static int
l_audiodev_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, AudioDeviceName);
	AudioDevice *dev = udata->data;

	if (udata->mustdelete) {
		if (dev->isdevice)
			SDL_CloseAudioDevice(dev->id);
		else
			SDL_CloseAudio();

		lua_close(dev->L);
		udata->mustdelete = 0;
		free(dev);
	}

	return 0;
}

/*
 * AudioDevice:__tostring()
 */
static int
l_audiodev_tostring(lua_State *L)
{
	AudioDevice *dev = commonGetAs(L, 1, AudioDeviceName, AudioDevice *);
	const char *status = "Unknown";
	int i, st;

	st = SDL_GetAudioDeviceStatus(dev->id);
	for (i = 0; AudioStatus[i].name != NULL; ++i)
		if (AudioStatus[i].value == st) {
			status = AudioStatus[i].name;
			break;
		}

	lua_pushfstring(L, "audio device %d: status: %s", dev->id, status);

	return 1;
}

/* --------------------------------------------------------
 * Audio device object definition
 * -------------------------------------------------------- */

static const luaL_Reg AudiodevMethods[] = {
	{ "close",			l_audiodev_gc		},
	{ "pause",			l_audiodev_pause	},
	{ "lock",			l_audiodev_lock		},
	{ "status",			l_audiodev_status	},
	{ "unlock",			l_audiodev_unlock	},
#if SDL_VERSION_ATLEAST(2, 0, 4)
	{ "queue",			l_audiodev_queue	},
#if SDL_VERSION_ATLEAST(2, 0, 5)
	{ "dequeue",			l_audiodev_dequeue	},
#endif
	{ "clearQueued",		l_audiodev_clearQueued	},
	{ "getQueuedSize",		l_audiodev_getQueuedSize},
#endif
	{ NULL,				NULL			}
};

static const luaL_Reg AudiodevMetamethods[] = {
	{ "__eq",			l_audiodev_eq		},
	{ "__gc",			l_audiodev_gc		},
	{ "__tostring",			l_audiodev_tostring	},
	{ NULL,				NULL			}
};

const CommonObject AudioObject = {
	"AudioDevice",
	AudiodevMethods,
	AudiodevMetamethods
};
