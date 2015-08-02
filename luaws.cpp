#define LUA_LIB

#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "easywsclient.hpp"

using easywsclient::WebSocket;

struct wsclient {
	WebSocket::pointer ws;
} WSClient;


static wsclient *luaws_checkws(lua_State* L) {
	void* ud = luaL_checkudata(L, 1, "WS.client");
	luaL_argcheck(L, ud != NULL, 1, "{WSclient} expected");
	return (wsclient*)ud;
}

static int luaws_connect(lua_State* L) {
	const char* uri = luaL_checkstring(L, 1);
	const char* origin = lua_tolstring(L, 2, NULL);
	wsclient* ws = (wsclient*)lua_newuserdata(L, sizeof(wsclient));
	if(origin)
		ws->ws = WebSocket::from_url(uri, origin);
	else
		ws->ws = WebSocket::from_url(uri);
	if (!ws->ws) {
		lua_pushnil(L); // push nil
		return 1;
	}	

	luaL_getmetatable(L, "WS.client");
	lua_setmetatable(L, -2);
	return 1;
}

static int luaws_tostring(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	lua_pushstring(L, "{WSclient}");
	return 1;
}

static int luaws_send(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	std::string message = std::string(luaL_checkstring(L, 2));
	ws->ws->send(message);
	return 0;
}

static int luaws_sendBinary(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	size_t len;
	const char* str = lua_tolstring(L, 2, &len);
	if (!str)
		luaL_argerror(L, 2, "string expected");

	// Pushing the outgoing message because std::string was truncating strings with null characters
	std::vector<uint8_t> message;

	for (int i = 0; i < len; i++)
		message.push_back(str[i]);	

	ws->ws->sendBinary(message);

	return 0;
}

static int luaws_close(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	ws->ws->close();
	return 0;
}

static int luaws_poll(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	int timeout = lua_tonumber(L, 2);
	ws->ws->poll(timeout);
	return 0;
}

static int luaws_receive(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	std::string output;
	ws->ws->dispatch([&output](std::string message) {
		output = std::move(message);
	});
	if (output.size() != 0) {
		lua_pushlstring(L, output.c_str(), output.length());
		return 1;
	}
	return 0;
}

static int luaws_getReadyState(lua_State* L) {
	wsclient* ws = luaws_checkws(L);
	lua_pushnumber(L, ws->ws->getReadyState());

	return 1;
}

static const struct luaL_Reg lib[] = {
	{ "connect", luaws_connect },	
	{ NULL, NULL }
};

static const struct luaL_Reg methods[] = {
	{"__tostring", luaws_tostring},
	{ "send", luaws_send },
	{ "sendBinary", luaws_sendBinary },
	{ "receive", luaws_receive },
	{ "poll", luaws_poll },
	{ "close", luaws_close },
	{ "getReadyState", luaws_getReadyState },
	
	{ NULL, NULL }
};

extern "C" __declspec(dllexport) int luaopen_luaws(lua_State *L) {
	luaL_newmetatable(L, "WS.client");

	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);

	luaL_openlib(L, NULL, methods, 0);
	luaL_openlib(L, "luaws", lib, 0);

	lua_pushstring(L, "CLOSING");
	lua_pushnumber(L, 0);
	lua_settable(L, -3);

	lua_pushstring(L, "CLOSED");
	lua_pushnumber(L, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "CONNECTING");
	lua_pushnumber(L, 2);
	lua_settable(L, -3);

	lua_pushstring(L, "OPEN");
	lua_pushnumber(L, 3);
	lua_settable(L, -3);

	return 1;
}