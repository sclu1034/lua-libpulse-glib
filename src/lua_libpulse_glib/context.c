#include "context.h"

#include "lua_util.h"
#include "pulseaudio.h"

#include <pulse/context.h>
#include <pulse/error.h>
#include <pulse/subscribe.h>


/* Calls the user-provided callback with the updated state info.
 */
void context_state_callback(pa_context* c, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    luaL_checktype(data->L, 1, LUA_TFUNCTION);
    // `lua_call` will pop the function and arguments from the stack, but this callback will likely be called
    // multiple times.
    // To preseve the values for future calls, we need to duplicate them.
    lua_pushvalue(data->L, 1);
    // This can't really fail, but for consistency, we keep the error value.
    lua_pushnil(data->L);

    pa_context_state_t state = pa_context_get_state(c);
    lua_pushinteger(data->L, state);

    lua_call(data->L, 2, 0);
}


/* Calls the user-prodivded event callbacks.
 */
void context_event_callback(pa_context* c, pa_subscription_event_type_t event_type, uint32_t index, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    lua_State* L = data->L;

    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checkudata(L, 2, LUA_PA_CONTEXT);

    // Iterate over the list of subscription callbacks and call each one
    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        // TODO: Once we do have the "nothing here" value, we need to check for that here.

        // Copy the `self` parameter
        lua_pushvalue(L, 2);
        lua_pushinteger(L, event_type);
        // Adjust the index to Lua's 1-base
        lua_pushinteger(L, index + 1);

        lua_call(L, 3, 0);
    }
}


void context_subscribe_success_callback(pa_context* _c, int success, void* userdata) {
    simple_callback_data* data = (simple_callback_data*) userdata;
    if (!success) {
        luaL_error(data->L, "Failed to subscribe to events");
    }
}


int context_new(lua_State* L, pa_mainloop_api* pa_api) {
    const char* name = luaL_checkstring(L, -1);
    // TODO: libpulse recommends using `new_with_proplist` instead. But I need to figure out that `proplist` first.
    pa_context* ctx = pa_context_new(pa_api, name);
    if (ctx == NULL) {
        return luaL_error(L, "failed to create pulseaudio context");
    }

    lua_pa_context* lgi_ctx = lua_newuserdata(L, sizeof(lua_pa_context));
    if (lgi_ctx == NULL) {
        return luaL_error(L, "failed to create context userdata");
    }
    lgi_ctx->context = ctx;
    lgi_ctx->connected = FALSE;
    lgi_ctx->subscribed = FALSE;
    lgi_ctx->state_callback_data = prepare_lua_callback(L, 0);
    lgi_ctx->event_callback_data = prepare_lua_callback(L, 0);

    // Create the table used to store the subscription callbacks.
    lua_newtable(lgi_ctx->event_callback_data->L);

    luaL_getmetatable(L, LUA_PA_CONTEXT);
    lua_setmetatable(L, -2);

    // Copy the `context` value to the event callback
    lua_pushvalue(L, -1);
    lua_xmove(L, lgi_ctx->event_callback_data->L, 1);

    return 1;
}


int context__gc(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    if (ctx->connected == TRUE) {
        pa_context_disconnect(ctx->context);
        ctx->connected = 0;
    }

    if (ctx->state_callback_data != NULL) {
        free_lua_callback(ctx->state_callback_data);
    }

    if (ctx->event_callback_data != NULL) {
        free_lua_callback(ctx->event_callback_data);
    }

    pa_context_unref(ctx->context);
    return 0;
}


int context_connect(lua_State* L) {
    int nargs = lua_gettop(L);
    const char* server = NULL;

    if (lua_type(L, 2) == LUA_TSTRING)
        server = lua_tostring(L, 2);
    else if (lua_type(L, 2) != LUA_TNIL) {
        const char* typearg;
        if (luaL_getmetafield(L, 2, "__name") == LUA_TSTRING)
            typearg = lua_tostring(L, -1);
        else if (lua_type(L, 2) == LUA_TLIGHTUSERDATA)
            typearg = "light userdata";
        else
            typearg = luaL_typename(L, 2);

        return luaL_argerror(L, 2, lua_pushfstring(L, "string or nil expected, got %s", typearg));
    }

    luaL_checktype(L, 3, LUA_TFUNCTION);

    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);

    pa_context_flags_t flags = PA_CONTEXT_NOAUTOSPAWN;
    if (nargs > 3)
        flags = luaL_checkinteger(L, 4);

    // Make sure the callback function is at a known position in the thread's stack
    lua_settop(ctx->state_callback_data->L, 0);
    lua_pushvalue(L, 3);
    lua_xmove(L, ctx->state_callback_data->L, 1);

    pa_context_set_state_callback(ctx->context, context_state_callback, ctx->state_callback_data);
    pa_context_set_subscribe_callback(ctx->context, context_event_callback, ctx->event_callback_data);

    // TODO: Check if I need to create bindings for `pa_spawn_api`.
    int ret = pa_context_connect(ctx->context, server, flags, NULL);
    if (ret < 0) {
        return luaL_error(L, "failed to connect: %s", pa_strerror(ret));
    }

    return 0;
}


int context_disconnect(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    pa_context_disconnect(ctx->context);
    return 0;
}


int context_get_state(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    pa_context_state_t state = pa_context_get_state(ctx->context);
    lua_pushinteger(L, state);
    return 1;
}


int context_set_default_sink(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op = pa_context_set_default_sink(ctx->context, name, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set default sink: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_set_default_source(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    const char* name = luaL_checkstring(L, 2);

    simple_callback_data* data = prepare_lua_callback(L, 3);

    pa_operation* op = pa_context_set_default_source(ctx->context, name, success_callback, data);
    if (op == NULL) {
        int error = pa_context_errno(ctx->context);
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "failed to set default source: %s", pa_strerror(error));
        lua_call(L, 1, 0);
        return 0;
    }

    return 0;
}


int context_subscribe(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    // This call is only effective when the connection state is "ready".
    // So we have to do it here, rather than during `context_connect`,
    // but we also need to make sure it's only called once.
    if (!ctx->subscribed) {
        pa_context_subscribe(ctx->context,
                             PA_SUBSCRIPTION_MASK_ALL,
                             context_subscribe_success_callback,
                             ctx->event_callback_data);
    }

    size_t pos = lua_rawlen(ctx->event_callback_data->L, 1) + 1;
    // Duplicate the callback function, so we can move it over to the other thread
    // TODO: Do we actually need to duplicate?
    lua_pushvalue(L, 2);
    lua_xmove(L, ctx->event_callback_data->L, 1);
    lua_rawseti(ctx->event_callback_data->L, 1, pos);

    lua_pushinteger(L, pos);
    return 1;
}


int context_unsubscribe(lua_State* L) {
    lua_pa_context* ctx = luaL_checkudata(L, 1, LUA_PA_CONTEXT);
    lua_State* thread_L = ctx->event_callback_data->L;
    size_t pos = 0;
    size_t len = lua_rawlen(thread_L, 1);

    if (len == 0) {
        return 0;
    }

    // TODO: Handle calling this twice on the same index.
    // Given that we use an array to track things, and Lua's way of counting in arrays
    // doesn't like `nil`s, we probably need a special "nothing here" value that's not `nil`,
    // and signifies an index that has already been unsubscribed.

    // TODO: Simplify things. Supporting just the index is enough.
    // Comparing by function is convenient, but also confusing to inexperienced devs.

    switch (lua_type(L, 2)) {
    case LUA_TNUMBER: {
        pos = lua_tointeger(L, 2);
        break;
    }
    case LUA_TFUNCTION: {
        bool found = false;
        size_t i = 0;

        // Duplicate the function value, so we can move it other to the other thread for comparing
        lua_pushvalue(L, -1);
        lua_xmove(L, thread_L, 1);
        int fn_index = lua_gettop(thread_L);

        lua_pushnil(L);
        while (lua_next(thread_L, 1) != 0) {
            ++i;

            if (lua_equal(thread_L, -1, fn_index) == 1) {
                pos = i;
                lua_pop(thread_L, 2);
                break;
            }

            // Remove the value, but keep the key to continue iterating
            lua_pop(thread_L, 1);
        }

        if (!found) {
            return luaL_error(L, "couldn't find this function in the list of subscriptions");
        }

        break;
    }
    default: {
        return luaL_argerror(L, 2, "expected number or function");
    }
    }

    // TODO: As explained above, we need to handle calling unsubscribe twice better.
    // Indices should not be re-used but replaced by a special "nothing here" value.
    for (; pos < len; ++pos) {
        lua_rawgeti(thread_L, 1, pos + 1);
        lua_rawseti(thread_L, 1, pos);
    }
    lua_pushnil(thread_L);
    lua_rawseti(thread_L, 1, len);

    return 0;
}
