/// libpulse bindings.
//
// @module pulseaudio

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pulse/glib-mainloop.h>

#define LUA_MOD_EXPORT extern
#define LUA_PULSEAUDIO "pulseaudio"


#if LUA_VERSION_NUM <= 501
// Shamelessly copied from Lua 5.3 source.
// TODO: Is there any official way to do this in 5.1?
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (; l->name != NULL; l++) {  /* fill the table with given functions */
        if (l->func == NULL)  /* place holder? */
            lua_pushboolean(L, 0);
        else {
            int i;
            for (i = 0; i < nup; i++)  /* copy upvalues to the top */
                lua_pushvalue(L, -nup);
            lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        }
        lua_setfield(L, -(nup + 2), l->name);
    }
    lua_pop(L, nup);  /* remove upvalues */
}


#define luaL_newlib(L,l) (luaL_register(L,LUA_PULSEAUDIO,l))
#endif


typedef struct pulseaudio {
    pa_glib_mainloop* mainloop;
} pulseaudio;


/**
 * Creates a new PulseAudio object.
 *
 * The API requires a GLib Main Context internally. Currently, only the default context
 * is supported.
 *
 * @return[type=PulseAudio]
 */
static int
pulseaudio_new(lua_State* L)
{
    GMainContext* ctx = g_main_context_default();
    if (ctx == NULL) {
        lua_pushfstring(L, "Failed to accquire default GLib Main Context. Are we running in a Main Loop?");
        lua_error(L);
        return 0;
    }

    pulseaudio* pa = lua_newuserdata (L, sizeof(pulseaudio));
    if (!pa) {
        return luaL_error(L, "failed to create pulseaudio userdata");
    }
    luaL_getmetatable(L, LUA_PULSEAUDIO);
    lua_setmetatable(L, -2);

    pa->mainloop = pa_glib_mainloop_new(ctx);

    return 1;
}


/**
 * Free the PulseAudio object
 */
static int
pulseaudio__gc(lua_State* L)
{
    pulseaudio* pa = luaL_checkudata(L, 1, LUA_PULSEAUDIO);
    pa_glib_mainloop_free(pa->mainloop);
    return 0;
}


static const struct luaL_Reg pulseaudio_mt [] = {
    {"__gc", pulseaudio__gc},
    {NULL, NULL}
};


static const struct luaL_Reg pulseaudio_lib [] = {
    {"new", pulseaudio_new},
    {NULL, NULL}
};


LUA_MOD_EXPORT int luaopen_lua_libpulse_glib(lua_State* L)
{
    luaL_newmetatable(L, LUA_PULSEAUDIO);
    luaL_setfuncs(L, pulseaudio_mt, 0);

    luaL_newlib(L, pulseaudio_lib);
    return 1;
}
