#pragma once

#include <lauxlib.h>
#include <lua.h>

#define LUA_MOD_EXPORT extern

#if LUA_VERSION_NUM <= 501
#define lua_rawlen lua_objlen

#define luaL_newlib(L, l)      (luaL_newlibtable(L, l), luaL_setfuncs(L, l, 0))
#define luaL_newlibtable(L, l) (lua_createtable(L, 0, sizeof(l) / sizeof(l[0])))
#endif

#if LUA_VERSION_NUM > 501
#define lua_equal(L, i1, i2) lua_compare(L, i1, i2, LUA_OPEQ)
#endif

typedef struct luaU_enumfield {
    const char* name;
    const char* value;
} luaU_enumfield;
