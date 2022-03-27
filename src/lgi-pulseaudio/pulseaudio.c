/// libpulse bindings.
//
// @module pulseaudio
#include "lib/lgi/lgi/lgi.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "lgi-pulseaudio"

#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib-2.0/glib.h>
#include <glib-object.h>
#include <pulse/glib-mainloop.h>

#ifdef _WIN32
#define LUA_MOD_EXPORT __declspec(dllexport)
#else
#define LUA_MOD_EXPORT extern
#endif

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


static int
pulseaudio_new(lua_State* L)
{
    printf("get new stuff: %d\n", lua_gettop(L));
    GMainContext* ctx = NULL;

    if (lua_gettop(L) > 0) {
        luaL_argcheck(L, lua_isuserdata(L, 1), 1, NULL);
        printf("it's userdata\n");

        lua_getfield(L, -1, "type");
        GITypeInfo* type_info = *(GITypeInfo **) luaL_checkudata(L, -1, LGI_GI_INFO);
        luaL_argcheck(L, type_info != NULL && GI_IS_TYPE_INFO(type_info), 1, "paramter has no type info");
        // TODO: Check if it's really a GMainContext
        printf("type: %s\n", g_base_info_get_name(type_info));
        // Pop the `type` field
        lua_pop(L, 1);

        lua_pop(L, lgi_marshal_2c(L, type_info, NULL, GI_TRANSFER_NOTHING, (gpointer) ctx, 1, 0, NULL, NULL));
        ctx = (GMainContext*) lua_touserdata(L, 1);
    } else {
        ctx = g_main_context_default();

        if (ctx == NULL) {
            lua_pushfstring(L, "Failed to accquire default GLib Main Context. Are we running in a Main Loop?");
            lua_error(L);
            return 0;
        }
    }
    printf("got some context\n");

    pulseaudio* pa = lua_newuserdata (L, sizeof(pulseaudio));
    if (!pa) {
        return luaL_error(L, "failed to create pulseaudio userdata");
    }
    pa->mainloop = pa_glib_mainloop_new(ctx);
    luaL_getmetatable(L, LUA_PULSEAUDIO);
    lua_setmetatable(L, -2);
    return 1;
}


// TODO: Implement `__gc` meta method to free the inner `pa_glib_mainloop`
static const struct luaL_Reg pulseaudio_mt [] = {
    {NULL, NULL}
};

static const struct luaL_Reg pulseaudio_lib [] = {
    {"new", pulseaudio_new},
    {NULL, NULL}
};


// Shamelessly stolen from Lua 5.3 source code
// Uses Lua's `require`
static int dolibrary(lua_State *L, const char *name) {
    int status;
    int base;

    base = lua_gettop(L);

    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    status = lua_pcall(L, 1, 1, 0);

    // Data for the `require` call is no longer needed
    lua_insert(L, base);
    lua_pop(L, 2);

    if (status == 0) {
        lua_setglobal(L, name);
        lua_pop(L, 1);
    }

    return status;
}

LUA_MOD_EXPORT int luaopen_lgi_pulseaudio(lua_State* L)
{
    if (dolibrary(L, "lgi") != 0) {
        lua_pushfstring(L, "failed to load library 'lgi'");
        lua_error(L);
        return 0;
    }

    luaL_newmetatable(L, LUA_PULSEAUDIO);
    luaL_setfuncs(L, pulseaudio_mt, 0);

    luaL_newlib(L, pulseaudio_lib);
    return 1;
}
