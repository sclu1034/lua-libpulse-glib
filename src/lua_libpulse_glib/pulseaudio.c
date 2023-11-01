#include "pulseaudio.h"

#include "context.h"
#include "lua_util.h"
#include "proplist.h"
#include "volume.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <pulse/glib-mainloop.h>

#if LUA_VERSION_NUM <= 501
// Shamelessly copied from Lua 5.3 source.
void luaL_setfuncs(lua_State* L, const luaL_Reg* l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (; l->name != NULL; l++) { /* fill the table with given functions */
        if (l->func == NULL)       /* place holder? */
            lua_pushboolean(L, 0);
        else {
            int i;
            for (i = 0; i < nup; i++) /* copy upvalues to the top */
                lua_pushvalue(L, -nup);
            lua_pushcclosure(L, l->func, nup); /* closure with those upvalues */
        }
        lua_setfield(L, -(nup + 2), l->name);
    }
    lua_pop(L, nup); /* remove upvalues */
}
#endif

int pulseaudio_new(lua_State* L) {
    GMainContext* ctx = g_main_context_default();
    if (ctx == NULL) {
        lua_pushfstring(L, "Failed to accquire default GLib Main Context. Are we running in a Main Loop?");
        lua_error(L);
        return 0;
    }

    pulseaudio* pa = lua_newuserdata(L, sizeof(pulseaudio));
    if (!pa) {
        return luaL_error(L, "failed to create pulseaudio userdata");
    }
    luaL_getmetatable(L, LUA_PULSEAUDIO);
    lua_setmetatable(L, -2);

    pa->mainloop = pa_glib_mainloop_new(ctx);

    return 1;
}


/*
 * Proxies table index operations to our metatable.
 */
int pulseaudio__index(lua_State* L) {
    const char* index = luaL_checkstring(L, 2);
    luaL_getmetatable(L, LUA_PULSEAUDIO);
    lua_getfield(L, -1, index);
    return 1;
}


/*
 * Free the PulseAudio object
 */
int pulseaudio__gc(lua_State* L) {
    pulseaudio* pa = luaL_checkudata(L, 1, LUA_PULSEAUDIO);
    pa_glib_mainloop_free(pa->mainloop);
    return 0;
}


int pulseaudio_new_context(lua_State* L) {
    pulseaudio* pa = luaL_checkudata(L, 1, LUA_PULSEAUDIO);
    return context_new(L, pa_glib_mainloop_get_api(pa->mainloop));
}


void createlib_volume(lua_State* L) {
    luaL_newmetatable(L, LUA_PA_VOLUME);

    luaL_newlib(L, volume_f);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, volume_mt, 0);

    luaL_newlib(L, volume_lib);

    lua_setmetatable(L, -2);
}


void createlib_proplist(lua_State* L) {
    luaL_newmetatable(L, LUA_PA_PROPLIST);

    luaL_newlib(L, proplist_f);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, proplist_mt, 0);

    luaL_newlib(L, proplist_lib);

    // Create a metatable with an `__index` table for read-only enum fields.
    lua_createtable(L, 0, 1);

    lua_createtable(L, 0, sizeof proplist_enum / sizeof proplist_enum[0]);
    for (int i = 0; proplist_enum[i].name != NULL; ++i) {
        lua_pushstring(L, proplist_enum[i].value);
        lua_setfield(L, -2, proplist_enum[i].name);
    }

    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
}


void createlib_context(lua_State* L) {
    luaL_newmetatable(L, LUA_PA_CONTEXT);

    luaL_newlib(L, context_f);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, context_mt, 0);
}


void createlib_pulseaudio(lua_State* L) {
    luaL_newmetatable(L, LUA_PULSEAUDIO);

    luaL_newlib(L, pulseaudio_f);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, pulseaudio_mt, 0);

    luaL_newlib(L, pulseaudio_lib);
}


LUA_MOD_EXPORT int luaopen_lua_libpulse_glib(lua_State* L) {
    // Create a table to store callback refs in, stored in the Lua registry
    lua_pushstring(L, LUA_PULSEAUDIO);
    lua_newtable(L);
    lua_pushstring(L, LUA_PA_REGISTRY);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_rawset(L, LUA_REGISTRYINDEX);

    createlib_context(L);
    createlib_proplist(L);
    createlib_pulseaudio(L);
    return 1;
}


LUA_MOD_EXPORT int luaopen_lua_libpulse_glib_volume(lua_State* L) {
    luaL_newmetatable(L, LUA_PA_VOLUME);

    luaL_newlib(L, volume_f);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, volume_mt, 0);

    luaL_newlib(L, volume_lib);
    return 1;
}
