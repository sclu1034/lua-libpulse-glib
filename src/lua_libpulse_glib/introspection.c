#include "callback.h"
#include "context.h"
#include "convert.h"
#include "proplist.h"
#include "pulseaudio.h"
#include "volume.h"

#include <pulse/error.h>
#include <pulse/introspect.h>
#include <pulse/xmalloc.h>
#include <stdbool.h>
#include <stdlib.h>


void server_info_callback(pa_context* c, const pa_server_info* info, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    lua_pushnil(L);
    server_info_to_lua(L, info);
    lua_call(L, 2, 0);

    free_lua_callback(data);
}


int context_get_server_info(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 2);

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


void sink_info_callback(pa_context* c, const pa_sink_info* info, int eol, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    if (data->is_list) {
        if (!eol) {
            int i = lua_objlen(L, 2);
            lua_pushinteger(L, i + 1);
            sink_info_to_lua(L, info);
            lua_settable(L, 2);
        } else {
            // Insert the error argument
            lua_pushnil(L);
            lua_insert(L, -2);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    } else {
        if (!eol) {
            lua_pushfstring(L, "only one sink info expected, but got multiple");
            lua_call(L, 1, 0);
        } else {
            lua_pushnil(L);
            sink_info_to_lua(L, info);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    }
}


int context_get_sink_info_list(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 2);
    data->is_list = true;
    // Create the list to store infos in
    lua_newtable(data->L);

    pa_operation* op = pa_context_get_sink_info_list(ctx->context, sink_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get sink info list: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_sink_info_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 2);

    pa_operation* op = pa_context_get_sink_info_by_name(ctx->context, name, sink_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get sink info by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_sink_info_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink index out of bounds. Got %d", index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op = pa_context_get_sink_info_by_index(ctx->context, (uint32_t) index - 1, sink_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get sink info by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_sink_info(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_get_sink_info_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_get_sink_info_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_set_sink_volume_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);
    const char* name = luaL_checkstring(L, 2);
    pa_cvolume* volume = volume_from_lua(L, 3);

    pa_operation* op = pa_context_set_sink_volume_by_name(ctx->context, name, volume, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set sink volume by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
    }

    // TODO: Is this too early?
    // It's probably fine when the operation failed, but in the other case, this might have to be moved to the callback.
    // But once this is userdata, I can simply put it onto the callback's thread stack and have Lua garbage collect it.
    pa_xfree((void*) volume);

    return 0;
}


int context_set_sink_volume_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink index out of bounds. Got %d", index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);
    pa_cvolume* volume = volume_from_lua(L, 3);

    pa_operation* op =
        pa_context_set_sink_volume_by_index(ctx->context, (uint32_t) index - 1, volume, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set sink volume by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
    }

    // TODO: Is this too early?
    // It's probably fine when the operation failed, but in the other case, this might have to be moved to the callback.
    // But once this is userdata, I can simply put it onto the callback's thread stack and have Lua garbage collect it.
    pa_xfree((void*) volume);

    return 0;
}


int context_set_sink_volume(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_set_sink_volume_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_set_sink_volume_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_set_sink_mute_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);
    int mute = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_set_sink_mute_by_name(ctx->context, name, mute, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set sink mute by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_sink_mute_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink index out of bounds. Got %d", index);
    }
    int mute = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_set_sink_mute_by_index(ctx->context, (uint32_t) index - 1, mute, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set sink mute by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_sink_mute(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_set_sink_mute_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_set_sink_mute_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_set_sink_suspended_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);
    int suspended = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_suspend_sink_by_name(ctx->context, name, suspended, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set sink suspended by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_sink_suspended_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink index out of bounds. Got %d", index);
    }
    int suspended = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_suspend_sink_by_index(ctx->context, (uint32_t) index - 1, suspended, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set sink suspended by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_sink_suspended(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_set_sink_suspended_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_set_sink_suspended_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


// Can be used both functions that produce a single value or the `_list` versions.
// The `is_list` value on the userdata needs to be set accordingly.
// For lists, the callback will be called multiple times, once for each entry.
// The function that sets up the callback must make sure to push a table on the stack, where the
// values can be collected.
//
// @tparam pa_source_info* info Pointer to the data.
// @tparam boolean eol Indicates whether the last item of the list was reached.
// @tparam simple_callback_data* userdata Expected to be an instance of `simple_callback_data`.
void source_info_callback(pa_context* c, const pa_source_info* info, int eol, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    if (data->is_list) {
        if (!eol) {
            int i = lua_objlen(L, 2);
            lua_pushinteger(L, i + 1);
            source_info_to_lua(L, info);
            lua_settable(L, 2);
        } else {
            // Insert the error argument
            lua_pushnil(L);
            lua_insert(L, -2);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    } else {
        if (!eol) {
            lua_pushfstring(L, "only one source info expected, but got multiple");
            lua_call(L, 1, 0);
        } else {
            lua_pushnil(L);
            source_info_to_lua(L, info);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    }
}


int context_get_source_info_list(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 2);
    data->is_list = true;
    // Create the list to store infos in
    lua_newtable(data->L);

    pa_operation* op = pa_context_get_source_info_list(ctx->context, source_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get source info list: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_source_info_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op = pa_context_get_source_info_by_name(ctx->context, name, source_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get source info by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_source_info_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source index out of bounds. Got %d", index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op =
        pa_context_get_source_info_by_index(ctx->context, (uint32_t) index - 1, source_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get source info by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_source_info(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_get_source_info_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_get_source_info_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_set_source_volume_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);
    const char* name = luaL_checkstring(L, 2);
    pa_cvolume* volume = volume_from_lua(L, 3);

    pa_operation* op = pa_context_set_source_volume_by_name(ctx->context, name, volume, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set source volume by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
    }

    // TODO: Is this too early?
    // It's probably fine when the operation failed, but in the other case, this might have to be moved to the callback.
    // But once this is userdata, I can simply put it onto the callback's thread stack and have Lua garbage collect it.
    pa_xfree((void*) volume);

    return 0;
}


int context_set_source_volume_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source index out of bounds. Got %d", index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);
    pa_cvolume* volume = volume_from_lua(L, 3);

    pa_operation* op =
        pa_context_set_source_volume_by_index(ctx->context, (uint32_t) index - 1, volume, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set source volume by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
    }

    // TODO: Is this too early?
    // It's probably fine when the operation failed, but in the other case, this might have to be moved to the callback.
    // But once this is userdata, I can simply put it onto the callback's thread stack and have Lua garbage collect it.
    pa_xfree((void*) volume);

    return 0;
}


int context_set_source_volume(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_set_source_volume_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_set_source_volume_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_set_source_mute_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);
    int mute = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_set_source_mute_by_name(ctx->context, name, mute, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set source mute by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_source_mute_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source index out of bounds. Got %d", index);
    }
    int mute = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_set_source_mute_by_index(ctx->context, (uint32_t) index - 1, mute, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set source mute by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_source_mute(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_set_source_mute_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_set_source_mute_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_set_source_suspended_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);
    int suspended = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_suspend_source_by_name(ctx->context, name, suspended, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set source suspended by name: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_source_suspended_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source index out of bounds. Got %d", index);
    }
    int suspended = lua_toboolean(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_suspend_source_by_index(ctx->context, (uint32_t) index - 1, suspended, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set source suspended by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_source_suspended(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_set_source_suspended_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_set_source_suspended_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


void sink_input_info_callback(pa_context* c, const pa_sink_input_info* info, int eol, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    if (data->is_list) {
        if (!eol) {
            int i = lua_objlen(L, 2);
            lua_pushinteger(L, i + 1);
            sink_input_info_to_lua(L, info);
            lua_settable(L, 2);
        } else {
            // Insert the error argument
            lua_pushnil(L);
            lua_insert(L, -2);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    } else {
        if (!eol) {
            lua_pushfstring(L, "only one sink input info expected, but got multiple");
            lua_call(L, 1, 0);
        } else {
            lua_pushnil(L);
            sink_input_info_to_lua(L, info);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    }
}


int context_get_sink_input_info_list(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 2);
    data->is_list = true;
    // Create the list to store infos in
    lua_newtable(data->L);

    pa_operation* op = pa_context_get_sink_input_info_list(ctx->context, sink_input_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get source info list: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_sink_input_info(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink input index out of bounds. Got %d", index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op =
        pa_context_get_sink_input_info(ctx->context, (uint32_t) index - 1, sink_input_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get sink input info by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_move_sink_input(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_move_sink_input_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_move_sink_input_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_move_sink_input_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer sink_input_index = luaL_checkinteger(L, 2);
    if (sink_input_index < 1) {
        return luaL_error(L, "Sink input index out of bounds. Got %d", sink_input_index);
    }

    lua_Integer sink_index = luaL_checkinteger(L, 3);
    if (sink_index < 1) {
        return luaL_error(L, "Sink index out of bounds. Got %d", sink_index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_move_sink_input_by_index(ctx->context,
                                                           (uint32_t) sink_input_index - 1,
                                                           (uint32_t) sink_index - 1,
                                                           success_callback,
                                                           data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L,
                        "failed to move sink input %d to sink %d: %s",
                        sink_input_index,
                        sink_index,
                        pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_move_sink_input_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer sink_input_index = luaL_checkinteger(L, 2);
    if (sink_input_index < 1) {
        return luaL_error(L, "Sink input index out of bounds. Got %d", sink_input_index);
    }

    const char* sink_name = luaL_checkstring(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_move_sink_input_by_name(ctx->context,
                                                          (uint32_t) sink_input_index - 1,
                                                          sink_name,
                                                          success_callback,
                                                          data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L,
                        "failed to move sink input %d to sink %s: %s",
                        sink_input_index,
                        sink_name,
                        pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_sink_input_volume(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink input index out of bounds. Got %d", index);
    }
    pa_cvolume* volume = volume_from_lua(L, 3);

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_set_sink_input_volume(ctx->context, (uint32_t) index - 1, volume, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set volume for sink input %d: %s", index, pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_sink_input_mute(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink input index out of bounds. Got %d", index);
    }
    bool mute = lua_toboolean(L, 3);

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_set_sink_input_mute(ctx->context, (uint32_t) index - 1, mute, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set mute for sink input %d: %s", index, pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_kill_sink_input(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Sink input index out of bounds. Got %d", index);
    }


    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op = pa_context_kill_sink_input(ctx->context, (uint32_t) index - 1, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to kill sink input %d: %s", index, pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


void source_output_info_callback(pa_context* c, const pa_source_output_info* info, int eol, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    if (data->is_list) {
        if (!eol) {
            int i = lua_objlen(L, 2);
            lua_pushinteger(L, i + 1);
            source_output_info_to_lua(L, info);
            lua_settable(L, 2);
        } else {
            // Insert the error argument
            lua_pushnil(L);
            lua_insert(L, -2);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    } else {
        if (!eol) {
            lua_pushfstring(L, "only one source output info expected, but got multiple");
            lua_call(L, 1, 0);
        } else {
            lua_pushnil(L);
            source_output_info_to_lua(L, info);

            lua_call(L, 2, 0);

            free_lua_callback(data);
        }
    }
}


int context_get_source_output_info_list(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 2);
    data->is_list = true;
    // Create the list to store infos in
    lua_newtable(data->L);

    pa_operation* op = pa_context_get_source_output_info_list(ctx->context, source_output_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get source info list: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_get_source_output_info(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source output index out of bounds. Got %d", index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op =
        pa_context_get_source_output_info(ctx->context, (uint32_t) index - 1, source_output_info_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to get source output info by index: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_move_source_output(lua_State* L) {
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        return context_move_source_output_by_name(L);
    }
    case LUA_TNUMBER: {
        return context_move_source_output_by_index(L);
    }
    default: {
        lua_pushfstring(L, "expected number or string, got %s", luaL_typename(L, 2));
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    }
}


int context_move_source_output_by_index(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer source_output_index = luaL_checkinteger(L, 2);
    if (source_output_index < 1) {
        return luaL_error(L, "Source output index out of bounds. Got %d", source_output_index);
    }

    lua_Integer source_index = luaL_checkinteger(L, 3);
    if (source_index < 1) {
        return luaL_error(L, "Source index out of bounds. Got %d", source_index);
    }

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_move_source_output_by_index(ctx->context,
                                                              (uint32_t) source_output_index - 1,
                                                              (uint32_t) source_index - 1,
                                                              success_callback,
                                                              data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L,
                        "failed to move source output %d to source %d: %s",
                        source_output_index,
                        source_index,
                        pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_move_source_output_by_name(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer source_output_index = luaL_checkinteger(L, 2);
    if (source_output_index < 1) {
        return luaL_error(L, "Source output index out of bounds. Got %d", source_output_index);
    }

    const char* source_name = luaL_checkstring(L, 3);

    if (pa_context_get_state(ctx->context) != PA_CONTEXT_READY) {
        lua_pushvalue(L, 2);
        lua_pushstring(L, "connection not ready");
        lua_call(L, 1, 0);
        return 0;
    }

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op = pa_context_move_source_output_by_name(ctx->context,
                                                             (uint32_t) source_output_index - 1,
                                                             source_name,
                                                             success_callback,
                                                             data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L,
                        "failed to move source output %d to source %s: %s",
                        source_output_index,
                        source_name,
                        pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_source_output_volume(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source output index out of bounds. Got %d", index);
    }
    pa_cvolume* volume = volume_from_lua(L, 3);

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_set_source_output_volume(ctx->context, (uint32_t) index - 1, volume, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set volume for source output %d: %s", index, pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_source_output_mute(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source output index out of bounds. Got %d", index);
    }
    bool mute = lua_toboolean(L, 3);

    simple_callback_data* data = prepare_lua_callback(L, 4);

    pa_operation* op =
        pa_context_set_source_output_mute(ctx->context, (uint32_t) index - 1, mute, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set mute for source output %d: %s", index, pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_kill_source_output(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index < 1) {
        return luaL_error(L, "Source output index out of bounds. Got %d", index);
    }


    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op = pa_context_kill_source_output(ctx->context, (uint32_t) index - 1, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to kill source output %d: %s", index, pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}
