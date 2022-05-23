/** Bindings for PulseAudio's libpulse, using the GLib Main Loop.
 *
 * @module lua_libpulse_glib
 */
#pragma once

#include "lauxlib.h"
#include "lua.h"

#include <pulse/glib-mainloop.h>

#define LUA_PULSEAUDIO  "lua_libpulse_glib"
#define LUA_PA_REGISTRY "lua_libpulse_glib.registry"


typedef struct pulseaudio {
    pa_glib_mainloop* mainloop;
} pulseaudio;


/** Creates a new PulseAudio object.
 *
 * @function new
 * @return[type=PulseAudio]
 */
int pulseaudio_new(lua_State*);


int pulseaudio__gc(lua_State*);


/// PulseAudio API
/// @type PulseAudio


/** Creates a new PulseAudio context
 *
 * @function context
 * @tparam string name The application name.
 * @return[type=Context]
 */
int pulseaudio_new_context(lua_State*);


static const struct luaL_Reg pulseaudio_mt[] = {
    {"__gc", pulseaudio__gc},
    { NULL,  NULL          }
};


static const struct luaL_Reg pulseaudio_f[] = {
    {"context", pulseaudio_new_context},
    { NULL,     NULL                  }
};


static const struct luaL_Reg pulseaudio_lib[] = {
    {"new", pulseaudio_new},
    { NULL, NULL          }
};
