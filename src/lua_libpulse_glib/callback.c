#include "callback.h"

#include "pulseaudio.h"


simple_callback_data* prepare_lua_callback(lua_State* L) {
    // Prepare a new thread to run the callback with
    lua_pushstring(L, LUA_PULSEAUDIO);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, LUA_PA_REGISTRY);
    lua_gettable(L, -2);
    lua_State* thread = lua_newthread(L);
    int thread_ref = luaL_ref(L, -2);

    // Copy the callback function to the thread's stack
    lua_pushvalue(L, 2);
    lua_xmove(L, thread, 1);

    simple_callback_data* data = malloc(sizeof(struct simple_callback_data));
    data->L = thread;
    data->thread_ref = thread_ref;
    data->is_list = false;

    return data;
}


void free_lua_callback(simple_callback_data* data) {
    lua_State* L = data->L;

    // Remove thread reference
    lua_pushstring(L, LUA_PULSEAUDIO);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, LUA_PA_REGISTRY);
    lua_gettable(L, -2);
    luaL_unref(L, -1, data->thread_ref);

    free(data);
}


void success_callback(pa_context* c, int success, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    lua_pushnil(L);
    lua_pushboolean(L, success);
    lua_call(L, 2, 0);

    free_lua_callback(data);
}
