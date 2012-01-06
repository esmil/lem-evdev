/*
 * This file is part of lem-evdev.
 * Copyright 2011-2012 Emil Renner Berthing
 *
 * lem-evdev is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * lem-evdev is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with lem-evdev.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <lem.h>

static int
evdev_closed(lua_State *T)
{
	lua_pushnil(T);
	lua_pushliteral(T, "closed");
	return 2;
}

static int
evdev_busy(lua_State *T)
{
	lua_pushnil(T);
	lua_pushliteral(T, "busy");
	return 2;
}

static int
evdev_strerror(lua_State *T, int err)
{
	lua_pushnil(T);
	lua_pushstring(T, strerror(err));
	return 2;
}

static int
evdev_gc(lua_State *T)
{
	struct ev_io *w = lua_touserdata(T, 1);

	if (w->fd >= 0)
		(void)close(w->fd);

	return 0;
}

static int
evdev_close(lua_State *T)
{
	struct ev_io *w;

	luaL_checktype(T, 1, LUA_TUSERDATA);
	w = lua_touserdata(T, 1);
	if (w->fd < 0)
		return evdev_closed(T);
	if (w->data != NULL)
		return evdev_busy(T);

	if (close(w->fd))
		return evdev_strerror(T, errno);

	lua_pushboolean(T, 1);
	return 1;
}

static int
evdev_interrupt(lua_State *T)
{
	struct ev_io *w;
	lua_State *S;

	luaL_checktype(T, 1, LUA_TUSERDATA);
	w = lua_touserdata(T, 1);
	if (w->data == NULL) {
		lua_pushnil(T);
		lua_pushliteral(T, "not busy");
		return 2;
	}

	lem_debug("interrupting io action");
	ev_io_stop(LEM_ w);

	S = w->data;
	lua_pushnil(S);
	lua_pushliteral(S, "interrupted");
	lem_queue(S, 2);
	w->data = NULL;

	lua_pushboolean(T, 1);
	return 1;
}

static int
evdev__get(lua_State *T, struct ev_io *w, int tbl)
{
	struct input_event iev;
	ssize_t ret;
	int err;

	ret = read(w->fd, &iev, sizeof(struct input_event));
	lem_debug("read %ld bytes from %d", ret, w->fd);
	if (ret > 0) {
		lua_Number n;

		assert(ret == sizeof(struct input_event));

		n = (lua_Number)iev.time.tv_usec;
		n /= 1.0e6;
		n += (lua_Number)iev.time.tv_sec;

		lua_rawgeti(T, tbl, iev.type);
		lua_pushnumber(T, iev.code);
		lua_pushnumber(T, iev.value);
		lua_pushnumber(T, n);

		return 4;
	}
	err = errno;

	if (ret < 0 && err == EAGAIN)
		return 0;

	(void)close(w->fd);
	w->fd = -1;

	if (ret == 0 || err == ECONNRESET || err == EPIPE)
		return evdev_closed(T);

	return evdev_strerror(T, err);
}

static void
evdev_get_cb(EV_P_ struct ev_io *w, int revents)
{
	int ret;

	(void)revents;

	ret = evdev__get(w->data, w, 2);
	if (ret == 0)
		return;

	ev_io_stop(EV_A_ w);
	lem_queue(w->data, ret);
	w->data = NULL;
}

static int
evdev_get(lua_State *T)
{
	struct ev_io *w;
	int ret;

	luaL_checktype(T, 1, LUA_TUSERDATA);
	w = lua_touserdata(T, 1);
	if (w->fd < 0)
		return evdev_closed(T);
	if (w->data != NULL)
		return evdev_busy(T);

	ret = evdev__get(T, w, lua_upvalueindex(1));
	if (ret > 0)
		return ret;

	lem_debug("yielding");
	w->data = T;
	ev_io_start(LEM_ w);
	lua_settop(T, 1);
	lua_pushvalue(T, lua_upvalueindex(1));
	return lua_yield(T, 2);
}

static int
evdev_open(lua_State *T)
{
	const char *path = luaL_checkstring(T, 1);
	int fd;
	struct ev_io *w;

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
		return evdev_strerror(T, errno);

	w = lua_newuserdata(T, sizeof(struct ev_io));
	lua_pushvalue(T, lua_upvalueindex(1));
	lua_setmetatable(T, -2);

	ev_io_init(w, evdev_get_cb, fd, EV_READ);
	w->data = NULL;

	return 1;
}

#define insert_evdev_constant(L, name) \
	lua_pushliteral(L, #name); \
	lua_rawseti(L, -2, EV_##name)

int
luaopen_lem_evdev(lua_State *L)
{
	/* create module table */
	lua_newtable(L);

	/* create Device metatable mt */
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	/* mt.__gc = <evdev_gc> */
	lua_pushcfunction(L, evdev_gc);
	lua_setfield(L, -2, "__gc");
	/* mt.close = <evdev_close> */
	lua_pushcfunction(L, evdev_close);
	lua_setfield(L, -2, "close");
	/* mt.interrupt = <evdev_interrupt> */
	lua_pushcfunction(L, evdev_interrupt);
	lua_setfield(L, -2, "interrupt");

	/* create type lookup table */
	lua_newtable(L);
	insert_evdev_constant(L, SYN);
	insert_evdev_constant(L, KEY);
	insert_evdev_constant(L, REL);
	insert_evdev_constant(L, ABS);
	insert_evdev_constant(L, MSC);
	insert_evdev_constant(L, SW);
	insert_evdev_constant(L, LED);
	insert_evdev_constant(L, SND);
	insert_evdev_constant(L, REP);
	insert_evdev_constant(L, FF);
	insert_evdev_constant(L, PWR);
	insert_evdev_constant(L, FF_STATUS);
	insert_evdev_constant(L, MAX);
	insert_evdev_constant(L, CNT);

	/* mt.read = <evdev_get> */
	lua_pushcclosure(L, evdev_get, 1);
	lua_setfield(L, -2, "get");

	/* insert open function */
	lua_pushvalue(L, -1);
	lua_pushcclosure(L, evdev_open, 1);
	lua_setfield(L, -3, "open");

	lua_setfield(L, -2, "Device");

	return 1;
}
