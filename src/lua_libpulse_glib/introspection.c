#include <pulse/introspect.h>
#include <pulse/error.h>
#include "introspection.h"
#include "context.h"
#include "pulseaudio.h"


// TODO: Figure out error handling for callbacks


typedef struct server_info_callback_data {
    lua_State* L;
    int thread_ref;
} server_info_callback_data;


void
server_info_to_lua(lua_State* L, const pa_server_info* info)
{
    lua_createtable(L, 0, 9);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "user_name");
    lua_pushstring(L, info->user_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "host_name");
    lua_pushstring(L, info->host_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "server_version");
    lua_pushstring(L, info->server_version);
    lua_settable(L, table_index);

    lua_pushstring(L, "server_name");
    lua_pushstring(L, info->server_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "default_sink_name");
    lua_pushstring(L, info->default_sink_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "cookie");
    lua_pushinteger(L, info->cookie);
    lua_settable(L, table_index);

    // TODO: Handle `sample_spec` and `channel_map` tables
}


void
server_info_callback(pa_context* c, const pa_server_info* info, void* userdata)
{

    server_info_callback_data* data = (server_info_callback_data*) userdata;
    lua_State* L = data->L;

    lua_pushnil(L);
    server_info_to_lua(L, info);
    lua_call(L, 2, 0);

    // Remove thread reference
    lua_pushstring(L, LUA_PULSEAUDIO);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, LUA_PA_REGISTRY);
    lua_gettable(L, -2);
    luaL_unref(L, -1, data->thread_ref);

    free(data);
}


int
context_get_server_info(lua_State* L)
{
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

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

    server_info_callback_data* data = calloc(1, sizeof(struct server_info_callback_data));
    data->L = thread;
    data->thread_ref = thread_ref;

    pa_operation* op = pa_context_get_server_info(ctx->context, server_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get server info: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


typedef struct sink_info_list_callback_data {
    lua_State* L;
    int thread_ref;
} sink_info_list_callback_data;


void
sink_info_to_lua(lua_State* L, const pa_sink_info* info)
{
    lua_createtable(L, 0, 24);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    // TODO: Handle rest of the table
}


void
sink_info_list_callback(pa_context* c, const pa_sink_info* info, int eol, void* userdata)
{
    sink_info_list_callback_data* data = (sink_info_list_callback_data*) userdata;
    lua_State* L = data->L;

    if (eol == 0) {
        int i = lua_objlen(L, 2);
        lua_pushinteger(L, i+1);
        sink_info_to_lua(L, info);
        lua_settable(L, 2);
    } else {
        // Insert the error argument
        lua_pushnil(L);
        lua_insert(L, -2);

        lua_call(L, 2, 0);

        lua_pushstring(L, LUA_PULSEAUDIO);
        lua_rawget(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, LUA_PA_REGISTRY);
        lua_gettable(L, -2);
        luaL_unref(L, -1, data->thread_ref);
        free(data);
    }
}


int
context_get_sink_info_list(lua_State* L)
{
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

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

    // List to store the values in
    lua_newtable(thread);

    sink_info_list_callback_data* data = calloc(1, sizeof(struct sink_info_list_callback_data));
    data->L = thread;
    data->thread_ref = thread_ref;

    pa_context_get_sink_info_list(ctx->context, sink_info_list_callback, data);
    return 0;
}


typedef struct source_info_list_callback_data {
    lua_State* L;
    int thread_ref;
} source_info_list_callback_data;


void
source_info_to_lua(lua_State* L, const pa_source_info* info)
{
    lua_createtable(L, 0, 24);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    // TODO: Handle rest of the table
}


void
source_info_list_callback(pa_context* c, const pa_source_info* info, int eol, void* userdata)
{
    source_info_list_callback_data* data = (source_info_list_callback_data*) userdata;
    lua_State* L = data->L;

    if (eol == 0) {
        int i = lua_objlen(L, 2);
        lua_pushinteger(L, i+1);
        source_info_to_lua(L, info);
        lua_settable(L, 2);
    } else {
        // Insert the error argument
        lua_pushnil(L);
        lua_insert(L, -2);

        lua_call(L, 2, 0);

        lua_pushstring(L, LUA_PULSEAUDIO);
        lua_rawget(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, LUA_PA_REGISTRY);
        lua_gettable(L, -2);
        luaL_unref(L, -1, data->thread_ref);
        free(data);
    }
}


int
context_get_source_info_list(lua_State* L)
{
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

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

    // List to store the values in
    lua_newtable(thread);

    source_info_list_callback_data* data = calloc(1, sizeof(struct source_info_list_callback_data));
    data->L = thread;
    data->thread_ref = thread_ref;

    pa_context_get_source_info_list(ctx->context, source_info_list_callback, data);
    return 0;
}
