#include "callback.h"

#include "pulseaudio.h"


simple_callback_data* prepare_lua_callback(lua_State* L, int callback_index) {
    // Prepare a new thread to run the callback with
    lua_pushstring(L, LUA_PULSEAUDIO);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, LUA_PA_REGISTRY);
    lua_gettable(L, -2);
    lua_State* thread = lua_newthread(L);
    int thread_ref = luaL_ref(L, -2);

    if (callback_index != 0) {
        // Copy the callback function to the thread's stack
        luaL_checktype(L, callback_index, LUA_TFUNCTION);
        lua_pushvalue(L, callback_index);
        lua_xmove(L, thread, 1);
    }

    simple_callback_data* data = malloc(sizeof(struct simple_callback_data));
    data->L = thread;
    data->thread_ref = thread_ref;
    data->is_list = false;

    // Clean up the intermediate data from creating the thread
    lua_pop(L, 2);

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

// When preparing a thread for this callback, the function must be at index `1`.
// Values on the stack after that will be ignored. This allows adding userdata and other values
// for the purpose of memory management, to keep them alive until the callback has been called.
void success_callback(pa_context* c, int success, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    // Copy the callback function to a position from where it can be called.
    // There may be other values on the stack for memory management.
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    lua_pushboolean(L, success);
    lua_call(L, 2, 0);

    free_lua_callback(data);
}
