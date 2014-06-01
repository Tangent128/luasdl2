/*
 * net.c -- main SDL_net (2.0) module
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

#include <SDL_net.h>

#include <common/common.h>
#include <common/table.h>

/* ---------------------------------------------------------
 * Common helpers
 * --------------------------------------------------------- */

/*
 * This define is used to store set in registry when they are attached to
 * a socket set to they are not deleted accidentally.
 */
#define REGISTRY	"__SDL_net_set_registry"

enum SocketType {
	Tcp,
	Udp
};

static int
pushAddress(lua_State *L, const IPaddress *addr)
{
	lua_createtable(L, 0, 2);

	tableSetInt(L, -1, "host", addr->host);
	tableSetInt(L, -1, "port", addr->port);

	return 1;
}

static void
checkAddress(lua_State *L, int index, IPaddress *addr)
{
	luaL_checktype(L, index, LUA_TTABLE);

	addr->host = tableGetInt(L, index, "host");
	addr->port = tableGetInt(L, index, "port");
}

static void
assertNotClosed(lua_State *L, int index)
{
	int type;

#if LUA_VERSION_NUM >= 502
	lua_getuservalue(L, index);
	type = lua_type(L, -1);
	lua_pop(L, 1);
#else
	lua_getfenv(L, index);
	lua_getfield(L, -1, "closed");
	type = lua_type(L, -1);
	lua_pop(L, 2);
#endif

	if (type != LUA_TNIL)
		luaL_error(L, "attempt operation on closed socket");
}

static int
closeSocket(lua_State *L, enum SocketType type, const char *name)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, name);

	if (type == Tcp)
		SDLNet_TCP_Close(udata->data);
	else if (type == Udp)
		SDLNet_UDP_Close(udata->data);

	udata->mustdelete = 0;

	// Waiting for Lua 5.3, just set a table as user value
	lua_createtable(L, 0, 0);

#if LUA_VERSION_NUM >= 502
	lua_setuservalue(L, 1);
#else
	lua_getfenv(L, 1);
	lua_pushboolean(L, 1);
	lua_setfield(L, -2, "closed");
	lua_setfenv(L, 1);
#endif

	return 0;
}

static int
collectSocket(lua_State *L, enum SocketType type, const char *name)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, name);

	if (udata->mustdelete) {
		if (type == Tcp)
			SDLNet_TCP_Close(udata->data);
		else if (type == Udp)
			SDLNet_UDP_Close(udata->data);
	}

	return 0;
}

static int
ready(lua_State *L, int index, enum SocketType type, const char *name)
{
	SDLNet_GenericSocket s;

	if (type == Tcp)
		s = (SDLNet_GenericSocket)commonGetAs(L, index, name, TCPsocket);
	else if (type == Udp)
		s = (SDLNet_GenericSocket)commonGetAs(L, index, name, UDPsocket);
	else
		return luaL_error(L, "unknown socket");

	return commonPush(L, "b", SDLNet_SocketReady(s));
}

/* ---------------------------------------------------------
 * TCPsocket object
 * --------------------------------------------------------- */

#define TcpName	TcpSocket.name

static const CommonObject TcpSocket;

static int
l_tcp_close(lua_State *L)
{
	return closeSocket(L, Tcp, TcpName);
}

static int
l_tcp_accept(lua_State *L)
{
	TCPsocket s = commonGetAs(L, 1, TcpName, TCPsocket);
	TCPsocket client;

	assertNotClosed(L, 1);

	client = SDLNet_TCP_Accept(s);

	if (client == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", TcpName, client);
}

static int
l_tcp_getPeerAddress(lua_State *L)
{
	TCPsocket s = commonGetAs(L, 1, TcpName, TCPsocket);
	IPaddress *addr;

	assertNotClosed(L, 1);

	if ((addr = SDLNet_TCP_GetPeerAddress(s)) == NULL)
		return commonPushSDLError(L, 1);

	return pushAddress(L, addr);
};

static int
l_tcp_send(lua_State *L)
{
	TCPsocket s = commonGetAs(L, 1, TcpName, TCPsocket);
	size_t length;
	int sent, ret = 1;
	const char *data = luaL_checklstring(L, 2, &length);

	assertNotClosed(L, 1);

	sent = SDLNet_TCP_Send(s, data, length);

	lua_pushinteger(L, sent);
	if (sent < (int)length) {
		lua_pushstring(L, SDLNet_GetError());
		++ ret;
	}

	return ret;
}

static int
l_tcp_recv(lua_State *L)
{
	TCPsocket s = commonGetAs(L, 1, TcpName, TCPsocket);
	int count = luaL_checkinteger(L, 2);
	char *data;
	int nread, ret;

	assertNotClosed(L, 1);

	if ((data = malloc(count + 1)) == NULL)
		return commonPushErrno(L, 2);

	nread = SDLNet_TCP_Recv(s, data, count);
	if (nread <= 0) {
		lua_pushnil(L);		// the string
		lua_pushnil(L);		// the number read
		lua_pushstring(L, SDLNet_GetError());
		ret = 3;
	} else {
		lua_pushlstring(L, data, nread);
		lua_pushinteger(L, nread);
		ret = 2;
	}

	free(data);

	return ret;
}

static int
l_tcp_ready(lua_State *L)
{
	assertNotClosed(L, 1);

	return ready(L, 1, Tcp, TcpName);
}

static int
l_tcp_gc(lua_State *L)
{
	return collectSocket(L, Tcp, TcpName);
}

static const luaL_Reg TcpMethods[] = {
	{ "close",			l_tcp_close			},
	{ "accept",			l_tcp_accept			},
	{ "getPeerAddress",		l_tcp_getPeerAddress		},
	{ "send",			l_tcp_send			},
	{ "recv",			l_tcp_recv			},
	{ "ready",			l_tcp_ready			},
	{ NULL,				NULL				}
};

static const luaL_Reg TcpMetamethods[] = {
	{ "__gc",			l_tcp_gc			},
	{ NULL,				NULL				}
};

static const CommonObject TcpSocket = {
	"TcpSocket",
	TcpMethods,
	TcpMetamethods
};

/* ---------------------------------------------------------
 * UDPsocket object
 * --------------------------------------------------------- */

#define UdpName	UdpSocket.name

static const CommonObject UdpSocket;

static int
l_udp_close(lua_State *L)
{
	return closeSocket(L, Udp, UdpName);
}

static int
l_udp_bind(lua_State *L)
{
	UDPsocket s	= commonGetAs(L, 1, UdpName, UDPsocket);
	int channel	= luaL_checkinteger(L, 2); 
	IPaddress addr;
	int ret;

	assertNotClosed(L, 1);
	checkAddress(L, 3, &addr);

	if ((ret = SDLNet_UDP_Bind(s, channel, &addr)) < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

static int
l_udp_unbind(lua_State *L)
{
	UDPsocket s = commonGetAs(L, 1, UdpName, UDPsocket);
	int channel = luaL_checkinteger(L, 2);

	assertNotClosed(L, 1);

	SDLNet_UDP_Unbind(s, channel);

	return 0;
}

static int
l_udp_getPeerAddress(lua_State *L)
{
	UDPsocket s	= commonGetAs(L, 1, UdpName, UDPsocket);
	int channel	= luaL_checkinteger(L, 2);
	IPaddress *addr;

	assertNotClosed(L, 1);

	if ((addr = SDLNet_UDP_GetPeerAddress(s, channel)) == NULL)
		return commonPushSDLError(L, 1);

	return pushAddress(L, addr);
}

static int
l_udp_send(lua_State *L)
{
	UDPsocket s = commonGetAs(L, 1, UdpName, UDPsocket);
	UDPpacket p;
	int ret;

	assertNotClosed(L, 1);
	p.data = (Uint8 *)luaL_checklstring(L, 2, (size_t *)&p.len);

	/*
	 * If next argument is a table, we parse the address fields, otherwise
	 * we use a channel.
	 */
	if (lua_type(L, 3) == LUA_TTABLE) {
		checkAddress(L, 3, &p.address);
		p.channel = -1;
	} else if (lua_type(L, 3) == LUA_TNUMBER) {
		p.channel = luaL_checkinteger(L, 3);
	} else
		return luaL_error(L, "expected address or channel");

	if ((ret = SDLNet_UDP_Send(s, p.channel, &p)) == 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "i", ret);
}

static int
l_udp_recv(lua_State *L)
{
	UDPsocket s = commonGetAs(L, 1, UdpName, UDPsocket);
	int count = luaL_checkinteger(L, 2);
	int ret;
	UDPpacket *p;

	assertNotClosed(L, 1);

	if ((p = SDLNet_AllocPacket(count)) == NULL)
		return commonPushSDLError(L, 2);

	ret = SDLNet_UDP_Recv(s, p);
	if (ret <= 0) {
		lua_pushnil(L);
		lua_pushnil(L);
	} else {
		lua_pushlstring(L, (const char *)p->data, p->len);
		lua_pushinteger(L, ret);
	}

	SDLNet_FreePacket(p);

	return 2;
}

static int
l_udp_ready(lua_State *L)
{
	assertNotClosed(L, 1);

	return ready(L, 1, Udp, UdpName);
}

static int
l_udp_gc(lua_State *L)
{
	return collectSocket(L, Udp, UdpName);
}

static const luaL_Reg UdpMethods[] = {
	{ "close",			l_udp_close			},
	{ "bind",			l_udp_bind			},
	{ "unbind",			l_udp_unbind			},
	{ "getPeerAddress",		l_udp_getPeerAddress		},
	{ "send",			l_udp_send			},
	{ "recv",			l_udp_recv			},
	{ "ready",			l_udp_ready			},
	{ NULL,				NULL				}
};

static const luaL_Reg UdpMetamethods[] = {
	{ "__gc",			l_udp_gc			},
	{ NULL,				NULL				}
};

static const CommonObject UdpSocket = {
	"UdpSocket",
	UdpMethods,
	UdpMetamethods
};

/* ---------------------------------------------------------
 * SDLNet_SocketSet object
 * --------------------------------------------------------- */

#define SetName	SocketSet.name

static const CommonObject SocketSet;

static int
l_set_add(lua_State *L)
{
	SDLNet_SocketSet set = commonGetAs(L, 1, SetName, SDLNet_SocketSet);
	SDLNet_GenericSocket s;
	int length;

	if (!luaL_testudata(L, 2, TcpName) && !luaL_testudata(L, 2, UdpName))
		return luaL_error(L, "TcpSocket or UdpSocket expected");

	s = ((CommonUserdata *)lua_touserdata(L, 2))->data;

	if ((length = SDLNet_AddSocket(set, s)) < 0)
		return commonPushSDLError(L, 1);

	/* Store a copy to the registry so the set owns a copy */
	lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY);
	lua_pushvalue(L, 2);
	lua_rawsetp(L, -2, s);
	lua_pop(L, 1);

	return commonPush(L, "i", length);
}

static int
l_set_del(lua_State *L)
{
	SDLNet_SocketSet set = commonGetAs(L, 1, SetName, SDLNet_SocketSet);
	SDLNet_GenericSocket s;
	int length;

	if (!luaL_testudata(L, 2, TcpName) && !luaL_testudata(L, 2, UdpName))
		return luaL_error(L, "TcpSocket or UdpSocket expected");

	s = ((CommonUserdata *)lua_touserdata(L, 2))->data;

	if ((length = SDLNet_DelSocket(set, s)) < 0)
		return commonPushSDLError(L, 1);

	/* Remove from the registry so the set does not own anymore */
	lua_getfield(L, LUA_REGISTRYINDEX, REGISTRY);
	lua_pushnil(L);
	lua_rawsetp(L, -2, s);
	lua_pop(L, 1);

	return commonPush(L, "i", length);
}

static int
l_set_checkSockets(lua_State *L)
{
	SDLNet_SocketSet set = commonGetAs(L, 1, SetName, SDLNet_SocketSet);
	int timeout = luaL_checkinteger(L, 2);
	int ret;

	ret = SDLNet_CheckSockets(set, timeout);

	/*
	 * SDL_net documentation says that error is not meaningful so just
	 * returns 0 to tell no sockets are ready.
	 */
	if (ret < 0)
		ret = 0;

	return commonPush(L, "i", ret);
}

static int
l_set_gc(lua_State *L)
{
	CommonUserdata *udata = commonGetUserdata(L, 1, SetName);

	if (udata->mustdelete) {
		/* Remove all references to the sockets from the registry */
		lua_createtable(L, 0, 0);
		lua_setfield(L, LUA_REGISTRYINDEX, REGISTRY);

		/* Don't forget to free the set */
		SDLNet_FreeSocketSet(udata->data);
	}

	return 0;
}

static const luaL_Reg SetMethods[] = {
	{ "add",			l_set_add			},
	{ "del",			l_set_del			},
	{ "checkSockets",		l_set_checkSockets		},
	{ NULL,				NULL				}
};

static const luaL_Reg SetMetamethods[] = {
	{ "__gc",			l_set_gc			},
	{ NULL,				NULL				}
};

static const CommonObject SocketSet = {
	"SocketSet",
	SetMethods,
	SetMetamethods
};

/* ---------------------------------------------------------
 * SDL_net functions
 * --------------------------------------------------------- */

static int
l_init(lua_State *L)
{
	if (SDLNet_Init() < 0)
		return commonPushSDLError(L, 1);

	return commonPush(L, "b", 1);
}

static int
l_resolveHost(lua_State *L)
{
	const char *host	= NULL;
	int port		= luaL_checkinteger(L, 2);
	IPaddress addr;

	if (lua_type(L, 1) == LUA_TSTRING)
		host = luaL_checkstring(L, 1);

	if (SDLNet_ResolveHost(&addr, host, port) < 0)
		return commonPushSDLError(L, 1);

	return pushAddress(L, &addr);
}

static int
l_resolveIp(lua_State *L)
{
	IPaddress address;
	const char *host;

	checkAddress(L, 1, &address);
	if ((host = SDLNet_ResolveIP(&address)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "s", host);
}

static int
l_set(lua_State *L)
{
	int max = luaL_checkinteger(L, 1);
	SDLNet_SocketSet set;

	if ((set = SDLNet_AllocSocketSet(max)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", SetName, set);
}

static int
l_openTcp(lua_State *L)
{
	IPaddress address;
	TCPsocket s;

	checkAddress(L, 1, &address);

	if ((s = SDLNet_TCP_Open(&address)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", TcpName, s);
}

static int
l_openUdp(lua_State *L)
{
	int port = luaL_checkinteger(L, 1);
	UDPsocket s;

	if ((s = SDLNet_UDP_Open(port)) == NULL)
		return commonPushSDLError(L, 1);

	return commonPush(L, "p", UdpName, s);
}

static int
l_quit(lua_State *L)
{
	SDLNet_Quit();

	(void)L;

	return 0;
}

static const luaL_Reg functions[] = {
	{ "init",				l_init			},
	{ "resolveHost",			l_resolveHost		},
	{ "resolveIp",				l_resolveIp		},
	{ "set",				l_set			},
	{ "openTcp",				l_openTcp		},
	{ "openUdp",				l_openUdp		},
	{ "quit",				l_quit			},
	{ NULL,					NULL			}
};

int EXPORT
luaopen_SDL_net(lua_State *L)
{
	commonNewLibrary(L, functions);

	/* Prepare the table for socket set reference */
	lua_createtable(L, 0, 0);
	lua_setfield(L, LUA_REGISTRYINDEX, REGISTRY);

	/* TCPsocket */
	commonBindObject(L, &TcpSocket);

	/* UDPsocket */
	commonBindObject(L, &UdpSocket);

	/* SDLNet_SocketSet */
	commonBindObject(L, &SocketSet);

	return 1;
}
