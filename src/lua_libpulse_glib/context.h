#pragma once

#include "stdbool.h"
#include "lua.h"
#include "lauxlib.h"
#include "pulse/context.h"
#include "pulse/mainloop-api.h"

#define LUA_PA_CONTEXT "pulseaudio.context"


typedef struct context_state_callback_data {
    lua_State* L;
    int thread_ref;
} context_state_callback_data;


typedef struct lua_pa_context {
    pa_context* context;
    bool connected;
    context_state_callback_data* state_callback_data;
} lua_pa_context;


int
context_new(lua_State*, pa_mainloop_api*);
int
context__index(lua_State*);
int
context__gc(lua_State*);
int
context_connect(lua_State*);


static const struct luaL_Reg context_mt [] = {
    {"__index", context__index},
    {"__gc", context__gc},
    {"connect", context_connect},
    {NULL, NULL}
};
