#pragma once

#include "lua.h"
#include "lauxlib.h"
#include <pulse/glib-mainloop.h>

#ifdef _WIN32
#define LUA_MOD_EXPORT __declspec(dllexport)
#else
#define LUA_MOD_EXPORT extern
#endif


#define LUA_PULSEAUDIO "pulseaudio"
#define LUA_PA_REGISTRY "pulseaudio.registry"


typedef struct pulseaudio {
    pa_glib_mainloop* mainloop;
} pulseaudio;


int
pulseaudio_new(lua_State*);
int
pulseaudio__gc(lua_State*);
int
pulseaudio__index(lua_State*);
int
pulseaudio_new_context(lua_State*);


static const struct luaL_Reg pulseaudio_mt [] = {
    {"__index", pulseaudio__index},
    {"__gc", pulseaudio__gc},
    {"context", pulseaudio_new_context},
    {NULL, NULL}
};


static const struct luaL_Reg pulseaudio_lib [] = {
    {"new", pulseaudio_new},
    {NULL, NULL}
};
