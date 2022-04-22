#include "proplist.h"

#include <pulse/xmalloc.h>

// Creates a Lua userdatum from/for a PulseAudio proplist.
//
// The provided proplist will be `pa_proplist_copy()`ed to pass memory management over
// to the garbage collector.
//
// @return[type=PropList]
int proplist_to_lua(lua_State* L, pa_proplist* pa_plist) {
    proplist* plist = lua_newuserdata(L, sizeof(proplist));
    if (pa_plist == NULL) {
        lua_pushfstring(L, "Failed to allocate proplist userdata");
        return lua_error(L);
    }
    luaL_getmetatable(L, LUA_PA_PROPLIST);
    lua_setmetatable(L, -2);

    plist->plist = pa_proplist_copy(pa_plist);

    return 1;
}


int proplist_new(lua_State* L) {
    pa_proplist* pa_plist = pa_proplist_new();
    if (pa_plist == NULL) {
        lua_pushfstring(L, "Failed to allocate PulseAudio proplist");
        return lua_error(L);
    }

    return proplist_to_lua(L, pa_plist);
}


int proplist_from_string(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    pa_proplist* pa_plist = pa_proplist_from_string(str);

    return proplist_to_lua(L, pa_plist);
}


int proplist_key_valid(lua_State* L) {
    const char* key = luaL_checkstring(L, 1);
    int valid = pa_proplist_key_valid(key);
    lua_pushboolean(L, valid);
    return 1;
}


int proplist_isempty(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    int empty = pa_proplist_isempty(plist->plist);
    lua_pushboolean(L, empty);
    return 1;
}


int proplist_tostring_sep(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    const char* sep = luaL_checkstring(L, 2);
    const char* str = pa_proplist_to_string_sep(plist->plist, sep);
    lua_pushstring(L, str);
    pa_xfree((void*) str);
    return 1;
}


int proplist_clear(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    pa_proplist_clear(plist->plist);
    return 0;
}


int proplist_contains(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    const char* key = luaL_checkstring(L, 2);
    if (!pa_proplist_key_valid(key)) {
        return luaL_error(L, "invalid key for proplist");
    }

    int contains = pa_proplist_contains(plist->plist, key);
    if (contains < 0) {
        return luaL_error(L, "failed to check if proplist contains the key");
    } else {
        lua_pushboolean(L, contains);
    }

    return 1;
}


int proplist_copy(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    pa_proplist* other = pa_proplist_copy(plist->plist);

    return proplist_to_lua(L, other);
}


// Metatable functions


// Accesses the value at the given string key.
//
// This functions very similar to a regular hash table.
//
// Numeric indices/keys are not supported.
//
// Will return `nil` if there is no value or if it is not valid UTF-8.
// Arbitrary data from `pa_proplist_get()` is currently not supported.
//
// @param[type=string] key The index to access.
// @return[type=string|nil] The data at the given index.
int proplist__index(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    const char* key = luaL_checkstring(L, 2);
    if (!pa_proplist_key_valid(key)) {
        return luaL_error(L, "invalid key for proplist");
    }

    if (!pa_proplist_contains(plist->plist, key)) {
        lua_pushnil(L);
        return 1;
    }

    const char* value = pa_proplist_gets(plist->plist, key);
    if (value == NULL) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, value);
    }

    return 1;
}


// Sets or overwrites the value at the given key.
//
// Passing `nil` as value will unset the key.
//
// For both keys and values only strings are supported.
// Numeric indices or binary data values (as in `pa_proplist_set()`) are not available.
//
// @param[type=string] key The index to write to.
// @param[type=string|nil] value The value to write.
int proplist__newindex(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    const char* key = luaL_checkstring(L, 2);

    if (lua_isnil(L, 3)) {
        if (pa_proplist_unset(plist->plist, key) < 0) {
            // TODO: Get last error. Need to get access to the current context for
            // this.
            return luaL_error(L, "failed to unset key %s", key);
        }
    } else {
        const char* value = luaL_checkstring(L, 3);
        if (pa_proplist_sets(plist->plist, key, value) < 0) {
            return luaL_error(L, "failed to set value for key %s", key);
        }
    }

    return 0;
}


// Frees the internal proplist.
int proplist__gc(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    pa_proplist_free(plist->plist);
    return 0;
}


// Gets the size of the proplist.
int proplist__len(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    unsigned len = pa_proplist_size(plist->plist);
    lua_pushinteger(L, len);
    return 1;
}


// Compares two proplists for equality.
//
// Proplists are equal if they contain the same keys with the same values.
//
// @param[type=PropList] other The proplist to compare against.
// @return[type=boolean]
int proplist__eq(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    proplist* other = luaL_checkudata(L, 2, LUA_PA_PROPLIST);
    int equal = pa_proplist_equal(plist->plist, other->plist);
    lua_pushboolean(L, equal);
    return 1;
}


// Creates a string representation.
//
// This adds a newline character as separator and as final character.
//
// @return[type=string]
int proplist__tostring(lua_State* L) {
    proplist* plist = luaL_checkudata(L, 1, LUA_PA_PROPLIST);
    char* str = pa_proplist_to_string(plist->plist);
    lua_pushstring(L, str);
    pa_xfree((void*) str);
    return 1;
}
