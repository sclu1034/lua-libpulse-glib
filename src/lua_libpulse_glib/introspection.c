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
channel_map_to_lua(lua_State* L, const pa_channel_map* spec)
{
    lua_createtable(L, spec->channels, 0);
    int table_index = lua_gettop(L);

    for (int i = 0; i < spec->channels; ++i) {
        lua_pushinteger(L, i+1);
        lua_pushinteger(L, spec->map[i]);
        lua_settable(L, table_index);
    }
}


void
sample_spec_to_lua(lua_State* L, const pa_sample_spec* spec)
{
    lua_createtable(L, 0, 3);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "rate");
    lua_pushinteger(L, spec->rate);
    lua_settable(L, table_index);

    lua_pushstring(L, "channels");
    lua_pushinteger(L, spec->channels);
    lua_settable(L, table_index);

    lua_pushstring(L, "format");
    lua_pushinteger(L, spec->format);
    lua_settable(L, table_index);
}


void
volume_to_lua(lua_State* L, const pa_cvolume* volume)
{
    lua_createtable(L, volume->channels, 0);
    int table_index = lua_gettop(L);

    for (int i = 0; i < volume->channels; ++i) {
        lua_pushinteger(L, i+1);
        lua_pushinteger(L, volume->values[i]);
        lua_settable(L, table_index);
    }
}


void
sink_port_info_to_lua(lua_State* L, const pa_sink_port_info* info)
{
    lua_createtable(L, 0, 6);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "priority");
    lua_pushinteger(L, info->priority);
    lua_settable(L, table_index);

    lua_pushstring(L, "available");
    lua_pushinteger(L, info->available);
    lua_settable(L, table_index);

    lua_pushstring(L, "availability_group");
    lua_pushstring(L, info->availability_group);
    lua_settable(L, table_index);

    lua_pushstring(L, "type");
    lua_pushinteger(L, info->type);
    lua_settable(L, table_index);
}


void
format_info_to_lua(lua_State* L, const pa_format_info* info)
{
    lua_createtable(L, 0, 6);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "encoding");
    lua_pushinteger(L, info->encoding);
    lua_settable(L, table_index);

    // TODO:
    // lua_pushstring(L, "plist");
    // proplist_to_lua(L, &info->plist);
    // lua_settable(L, table_index);
}


void
ports_to_lua(lua_State* L, const pa_sink_port_info** list, int n_ports, const pa_sink_port_info* active)
{
    lua_createtable(L, 1, n_ports);
    int table_index = lua_gettop(L);

    for (int i = 0; i < n_ports; ++i) {
        lua_pushinteger(L, i+1);
        sink_port_info_to_lua(L, list[i]);

        if (list[i] == active) {
            lua_pushstring(L, "active");
            lua_pushvalue(L, -2);
            lua_settable(L, table_index);
        }

        lua_settable(L, table_index);
    }
}


void
formats_to_lua(lua_State* L, const pa_format_info** list, int n_formats)
{
    lua_createtable(L, 0, n_formats);
    int table_index = lua_gettop(L);

    for (int i = 0; i < n_formats; ++i) {
        lua_pushinteger(L, i+1);
        format_info_to_lua(L, list[i]);
        lua_settable(L, table_index);
    }
}


void
sink_info_to_lua(lua_State* L, const pa_sink_info* info)
{
    lua_createtable(L, 0, 24);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "index");
    lua_pushinteger(L, info->index);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);

    lua_pushstring(L, "owner_module");
    lua_pushinteger(L, info->owner_module);
    lua_settable(L, table_index);

    lua_pushstring(L, "volume");
    volume_to_lua(L, &info->volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "mute");
    lua_pushinteger(L, info->mute);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_source");
    lua_pushinteger(L, info->monitor_source);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_source_name");
    lua_pushstring(L, info->monitor_source_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "latency");
    lua_pushinteger(L, info->latency);
    lua_settable(L, table_index);

    lua_pushstring(L, "driver");
    lua_pushstring(L, info->driver);
    lua_settable(L, table_index);

    lua_pushstring(L, "flags");
    lua_pushinteger(L, info->flags);
    lua_settable(L, table_index);

    // TODO: The proplist needs more advanced handling
    // lua_pushstring(L, "proplist");
    // proplist_to_lua(L, &info->proplist);
    // lua_settable(L, table_index);

    lua_pushstring(L, "base_volume");
    lua_pushinteger(L, info->base_volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "state");
    lua_pushinteger(L, info->state);
    lua_settable(L, table_index);

    lua_pushstring(L, "n_volume_steps");
    lua_pushinteger(L, info->n_volume_steps);
    lua_settable(L, table_index);

    lua_pushstring(L, "card");
    lua_pushinteger(L, info->card);
    lua_settable(L, table_index);

    lua_pushstring(L, "ports");
    ports_to_lua(L, info->ports, info->n_ports, info->active_port);
    lua_settable(L, table_index);

    lua_pushstring(L, "formats");
    formats_to_lua(L, info->formats, info->n_formats);
    lua_settable(L, table_index);
}


void
source_info_to_lua(lua_State* L, const pa_source_info* info)
{
    lua_createtable(L, 0, 24);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "index");
    lua_pushinteger(L, info->index);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);

    lua_pushstring(L, "owner_module");
    lua_pushinteger(L, info->owner_module);
    lua_settable(L, table_index);

    lua_pushstring(L, "volume");
    volume_to_lua(L, &info->volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "mute");
    lua_pushinteger(L, info->mute);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_of_sink");
    lua_pushinteger(L, info->monitor_of_sink);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_of_sink_name");
    lua_pushstring(L, info->monitor_of_sink_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "latency");
    lua_pushinteger(L, info->latency);
    lua_settable(L, table_index);

    lua_pushstring(L, "driver");
    lua_pushstring(L, info->driver);
    lua_settable(L, table_index);

    lua_pushstring(L, "flags");
    lua_pushinteger(L, info->flags);
    lua_settable(L, table_index);

    // TODO: The proplist needs more advanced handling
    // lua_pushstring(L, "proplist");
    // proplist_to_lua(L, &info->proplist);
    // lua_settable(L, table_index);

    lua_pushstring(L, "configured_latency");
    lua_pushinteger(L, info->configured_latency);
    lua_settable(L, table_index);

    lua_pushstring(L, "base_volume");
    lua_pushinteger(L, info->base_volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "state");
    lua_pushinteger(L, info->state);
    lua_settable(L, table_index);

    lua_pushstring(L, "n_volume_steps");
    lua_pushinteger(L, info->n_volume_steps);
    lua_settable(L, table_index);

    lua_pushstring(L, "card");
    lua_pushinteger(L, info->card);
    lua_settable(L, table_index);

    lua_pushstring(L, "ports");
    ports_to_lua(L, info->ports, info->n_ports, info->active_port);
    lua_settable(L, table_index);

    lua_pushstring(L, "formats");
    formats_to_lua(L, info->formats, info->n_formats);
    lua_settable(L, table_index);
}


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

    lua_pushstring(L, "default_source_name");
    lua_pushstring(L, info->default_source_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "cookie");
    lua_pushinteger(L, info->cookie);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);
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
