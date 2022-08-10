#ifndef lua_util_h_INCLUDED
#define lua_util_h_INCLUDED

#include <lauxlib.h>
#include <lua.h>

#define LUA_MOD_EXPORT extern

#if LUA_VERSION_NUM <= 501
#define lua_rawlen lua_objlen
#endif

#if LUA_VERSION_NUM > 501
#define lua_equal(L, i1, i2) lua_compare(L, i1, i2, LUA_OPEQ)
#endif

typedef struct luaU_enumfield {
    const char* name;
    const char* value;
} luaU_enumfield;


#endif // lua_util_h_INCLUDED
