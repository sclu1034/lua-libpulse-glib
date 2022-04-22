#include "volume.h"

#include <pulse/xmalloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


int volume_to_lua(lua_State* L, const pa_cvolume* pa_volume) {
    volume_t* volume = lua_newuserdata(L, sizeof(volume_t));
    if (pa_volume == NULL) {
        lua_pushfstring(L, "Failed to allocate volume userdata");
        return lua_error(L);
    }
    luaL_getmetatable(L, LUA_PA_VOLUME);
    lua_setmetatable(L, -2);

    volume->inner = *pa_volume;
    return 1;
}


// When the value is a plain Lua table, a new `pa_cvolume` is allocated and the caller is
// responsible of freeing it with `pa_xfree`.
// If a userdata is passed instead, memory is owned by Lua and not be freed.
// TODO: Maybe construct userdata from the table, so that memory management is always handled
// by GC. Makes this easier to use. But only do so after the current version has been committed
// once, just so it stays available.
pa_cvolume* volume_from_lua(lua_State* L, int index) {
    switch (lua_type(L, 2)) {
    case LUA_TTABLE: {
        uint8_t channels = (uint8_t) lua_objlen(L, index);
        if (channels > PA_CHANNELS_MAX) {
            channels = PA_CHANNELS_MAX;
        }

        pa_cvolume* volume = pa_xnew(pa_cvolume, 1);
        volume->channels = channels;

        for (int i = 0; i < channels; ++i) {
            lua_pushinteger(L, i + 1);
            lua_gettable(L, index);
            pa_volume_t vol = (pa_volume_t) luaL_checkinteger(L, -1);
            volume->values[i] = PA_CLAMP_VOLUME(vol);
            lua_pop(L, 1);
        }

        return volume;
    }
    case LUA_TUSERDATA: {
        volume_t* volume = luaL_checkudata(L, index, LUA_PA_VOLUME);
        return &volume->inner;
    }
    default: {
        luaL_argerror(L, index, "expected table or userdata");
        return NULL;
    }
    }
}


int volume__len(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_pushinteger(L, volume->inner.channels);
    return 1;
}


int volume__eq(lua_State* L) {
    volume_t* left = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    volume_t* right = luaL_checkudata(L, 2, LUA_PA_VOLUME);
    lua_pushboolean(L, pa_cvolume_equal(&left->inner, &right->inner));
    return 1;
}


int volume__index(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    int index = luaL_checkint(L, 2);
    luaL_argcheck(L, index >= 1 && index <= (PA_CHANNELS_MAX + 1), 2, "channel index out of bounds");
    lua_pushinteger(L, volume->inner.values[index - 1]);
    return 1;
}


int volume__newindex(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer index = luaL_checkinteger(L, 2);
    lua_Integer value = luaL_checkinteger(L, 3);
    luaL_argcheck(L, index >= 1 && index <= (PA_CHANNELS_MAX + 1), 2, "channel index out of bounds");
    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 3, "volume value is invalid");
    volume->inner.values[index - 1] = value;
    return 0;
}


int volume__tostring(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    char s[PA_CVOLUME_SNPRINT_MAX];
    pa_cvolume_snprint(s, PA_CVOLUME_SNPRINT_MAX, &volume->inner);
    lua_pushstring(L, s);
    return 1;
}

int volume_is_valid(lua_State* L) {
    // Lua's `checkudata` throws an error, and catching that is expensive.
    // So we have to manually implement the check.
    volume_t* volume = lua_touserdata(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_PA_VOLUME);
    bool is_userdata = volume != NULL && lua_getmetatable(L, 1) && lua_rawequal(L, -1, -2);
    // Remove the two metatables
    lua_pop(L, 2);
    lua_pushboolean(L, is_userdata && pa_cvolume_valid(&volume->inner));
    return 1;
}


int volume_channels_equal_to(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer value = luaL_checkinteger(L, 2);

    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 2, "volume out of bounds");

    lua_pushboolean(L, pa_cvolume_channels_equal_to(&volume->inner, (pa_volume_t) value));
    return 1;
}


int volume_is_muted(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_pushboolean(L, pa_cvolume_is_muted(&volume->inner));
    return 1;
}


int volume_is_norm(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_pushboolean(L, pa_cvolume_is_norm(&volume->inner));
    return 1;
}


int volume_set_channels(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer channels = luaL_checkinteger(L, 2);
    lua_Integer value = luaL_checkinteger(L, 3);

    luaL_argcheck(L, channels >= 0 && channels <= PA_CHANNELS_MAX, 2, "channel count out of bounds");
    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 3, "volume out of bounds");

    pa_cvolume_set(&volume->inner, (unsigned int) channels, (pa_volume_t) value);
    return 0;
}


int volume_set(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer index = luaL_checkinteger(L, 2) - 1;
    lua_Integer value = luaL_checkinteger(L, 3);

    luaL_argcheck(L, index >= 0 && index <= PA_CHANNELS_MAX, 2, "channel index out of bounds");
    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 3, "volume out of bounds");

    volume->inner.values[index] = (pa_volume_t) value;
    return 0;
}


int volume_get(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer index = luaL_checkinteger(L, 2) - 1;

    luaL_argcheck(L, index > 0 && index < volume->inner.channels, 2, "channel index out of bounds");

    lua_pushinteger(L, volume->inner.values[index]);
    return 1;
}


int volume_reset(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer channels = luaL_checkinteger(L, 2);
    luaL_argcheck(L, channels >= 0 && channels <= PA_CHANNELS_MAX, 2, "channel count out of bounds");
    pa_cvolume_reset(&volume->inner, channels);
    return 0;
}


int volume_mute(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer channels = luaL_checkinteger(L, 2);
    luaL_argcheck(L, channels >= 0 && channels <= PA_CHANNELS_MAX, 2, "channel count out of bounds");
    pa_cvolume_mute(&volume->inner, channels);
    return 0;
}


int volume_avg(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_pushinteger(L, pa_cvolume_avg(&volume->inner));
    return 1;
}


int volume_min(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_pushinteger(L, pa_cvolume_min(&volume->inner));
    return 1;
}


int volume_max(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_pushinteger(L, pa_cvolume_max(&volume->inner));
    return 1;
}


int volume_inc(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer value = luaL_checkinteger(L, 2);
    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 2, "volume out of bounds");
    if (!pa_cvolume_inc(&volume->inner, (pa_volume_t) value)) {
        return luaL_error(L, "failed to increase volume");
    }

    return 0;
}


int volume_dec(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer value = luaL_checkinteger(L, 2);
    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 2, "volume out of bounds");
    if (!pa_cvolume_dec(&volume->inner, (pa_volume_t) value)) {
        return luaL_error(L, "failed to decrease volume");
    }

    return 0;
}


int volume_scale(lua_State* L) {
    volume_t* volume = luaL_checkudata(L, 1, LUA_PA_VOLUME);
    lua_Integer value = luaL_checkinteger(L, 2);
    luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 2, "volume out of bounds");
    if (!pa_cvolume_scale(&volume->inner, (pa_volume_t) value)) {
        return luaL_error(L, "failed to scale volume");
    }

    return 0;
}


int volume_multiply(lua_State* L) {
    volume_t* left = luaL_checkudata(L, 1, LUA_PA_VOLUME);

    switch (lua_type(L, 2)) {
    case LUA_TNUMBER: {
        lua_Integer value = luaL_checkinteger(L, 2);
        luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 2, "volume out of bounds");
        if (!pa_sw_cvolume_multiply_scalar(&left->inner, &left->inner, (pa_volume_t) value)) {
            return luaL_error(L, "failed to multiply volume");
        }

        return 0;
    }
    case LUA_TUSERDATA: {
        volume_t* right = luaL_checkudata(L, 2, LUA_PA_VOLUME);
        if (!pa_sw_cvolume_multiply(&left->inner, &left->inner, &right->inner)) {
            return luaL_error(L, "failed to multiply volume");
        }

        return 0;
    }
    default: {
        return luaL_argerror(L, 2, "expected number or userdata");
    }
    }
}

int volume_divide(lua_State* L) {
    volume_t* left = luaL_checkudata(L, 1, LUA_PA_VOLUME);

    switch (lua_type(L, 2)) {
    case LUA_TNUMBER: {
        lua_Integer value = luaL_checkinteger(L, 2);
        luaL_argcheck(L, PA_VOLUME_IS_VALID(value), 2, "volume out of bounds");
        if (!pa_sw_cvolume_divide_scalar(&left->inner, &left->inner, (pa_volume_t) value)) {
            return luaL_error(L, "failed to divide volume");
        }

        return 0;
    }
    case LUA_TUSERDATA: {
        volume_t* right = luaL_checkudata(L, 2, LUA_PA_VOLUME);
        if (!pa_sw_cvolume_divide(&left->inner, &left->inner, &right->inner)) {
            return luaL_error(L, "failed to divide volume");
        }

        return 0;
    }
    default: {
        return luaL_argerror(L, 2, "expected number or userdata");
    }
    }
}
