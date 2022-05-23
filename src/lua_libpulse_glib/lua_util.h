#ifndef lua_util_h_INCLUDED
#define lua_util_h_INCLUDED

#include <lauxlib.h>
#include <lua.h>

#define LUA_MOD_EXPORT extern


typedef struct luaU_enumfield {
    const char* name;
    const char* value;
} luaU_enumfield;


#endif // lua_util_h_INCLUDED
