/** Bindings for libpulse's connection context.
 *
 * The connection @{Context} provides introspection calls to query state from the server and various commands to
 * change this state.
 *
 * In many cases, sinks and sources may be addressed by either their name or their numeric index.
 * Both can be queried using the `get_(sink|source)_info` or `get_(sink|source)s` calls.
 *
 * @module lua_libpulse_glib.context
 */
#pragma once

#include <lauxlib.h>
#include <lua.h>
#include <pulse/context.h>
#include <pulse/mainloop-api.h>
#include <stdbool.h>

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


int context_new(lua_State*, pa_mainloop_api*);
int context__gc(lua_State*);


/// Context
/// @type Context


/** Connects the context to the given server address.
 *
 * If the server address is `nil`, libpulse will attempt to connect to what it considers the default server.
 * In most cases, this is the local machine.
 *
 * The provided callback function will be registered as state callback function, and will be called whenever the
 * context's connection state changes.
 *
 * @function Context:connect
 * @async
 * @tparam[opt=nil] string server_address The server address.
 * @tparam function cb The connection state callback.
 * @treturn[opt] string The error message
 * @treturn string The state
 */
int context_connect(lua_State*);

/** Disconnects from the server.
 *
 * @function Context:disconnect
 */
int context_disconnect(lua_State* L);


/** Gets information about the server the context is connected to.
 *
 * See [pa_server_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__server__info.html)
 * for documentation on the return type.
 *
 * This will fail when the connection state is anything other than `READY`.
 *
 * @function Context:get_server_info
 * @async
 * @tparam function cb
 * @return[opt] string The error
 * @return table The server info.
 */
int context_get_server_info(lua_State*);


// Sinks


/** Gets information about the given sink.
 *
 * The sink may be indicated by either its name or its index.
 *
 * See [pa_sink_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__sink__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_sink_info
 * @async
 * @tparam number|string sink The index or name of the sink to query.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_sink_info(lua_State*);

/** Gets information about all sinks.
 *
 * This returns the same information as @{Context:get_sink_info} would have returned for every sink
 * that's currently registered at the server.
 *
 * See [pa_sink_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__sink__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_sinks
 * @async
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_sink_info_list(lua_State*);
int context_get_sink_info_by_name(lua_State*);
int context_get_sink_info_by_index(lua_State*);

/** Sets the sink's volume to the given value.
 *
 * The sink may be indicated by either its name or its index.
 *
 * @function Context:set_sink_volume
 * @async
 * @tparam number|string sink The sink to update.
 * @tparam Volume volume
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_sink_volume(lua_State*);
int context_set_sink_volume_by_name(lua_State*);
int context_set_sink_volume_by_index(lua_State*);

/** Sets the sink's mute state.
 *
 * The sink may be indicated by either its name or its index.
 *
 * @function Context:set_sink_mute
 * @async
 * @tparam number|string sink The sink to update.
 * @tparam boolean mute
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_sink_mute(lua_State*);
int context_set_sink_mute_by_name(lua_State*);
int context_set_sink_mute_by_index(lua_State*);

/** Sets the sink's suspended state.
 *
 * The sink may be indicated by either its name or its index.
 *
 * @function Context:set_sink_suspended
 * @async
 * @tparam number|string sink The sink to update.
 * @tparam boolean suspended
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_sink_suspended(lua_State*);
int context_set_sink_suspended_by_name(lua_State*);
int context_set_sink_suspended_by_index(lua_State*);


// Sink Inputs

/** Gets information about the given sink input.
 *
 * The sink input may be indicated by either its name or its index.
 *
 * See [pa_sink_input_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__sink_input__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_sink_input_info
 * @async
 * @tparam number|string sink input The index or name of the sink input to query.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_sink_input_info(lua_State*);

/** Gets information about all sink inputs.
 *
 * This returns the same information as @{Context:get_sink_input_info} would have returned for every sink input
 * that's currently registered at the server.
 *
 * See [pa_sink_input_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__sink_input__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_sink_inputs
 * @async
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_sink_input_info_list(lua_State*);


/** Moves the sink input to a different name.
 *
 * The target sink may be indicated by either its name or its index.
 *
 * @function Context:set_sink_suspended
 * @async
 * @tparam number The sink input to move.
 * @tparam number|string sink The sink to update.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_move_sink_input(lua_State*);
int context_move_sink_input_by_name(lua_State*);
int context_move_sink_input_by_index(lua_State*);

/** Sets the sink input's volume to the given value.
 *
 * @function Context:set_sink_input_volume
 * @async
 * @tparam number sink_input The sink input to update.
 * @tparam Volume volume
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_sink_input_volume(lua_State*);

/** Sets the sink input's mute state.
 *
 * @function Context:set_sink_input_mute
 * @async
 * @tparam number sink_input The sink input to update.
 * @tparam boolean mute
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_sink_input_mute(lua_State*);

/** Kills the sink input.
 *
 * @function Context:kill_sink_input
 * @async
 * @tparam number sink_input The sink input to kill.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_kill_sink_input(lua_State*);


// Sources

/** Gets information about the given source.
 *
 * The source may be indicated by either its name or its index.
 *
 * See [pa_source_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__source__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_source_info
 * @async
 * @tparam number|string source The index or name of the source to query.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_source_info(lua_State*);

/** Gets information about all sources.
 *
 * This returns the same information as @{Context:get_source_info} would have returned for every source
 * that's currently registered at the server.
 *
 * See [pa_source_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__source__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_sources
 * @async
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_source_info_list(lua_State*);
int context_get_source_info_by_name(lua_State*);
int context_get_source_info_by_index(lua_State*);

/** Sets the source's volume to the given value.
 *
 * The source may be indicated by either its name or its index.
 *
 * @function Context:set_source_volume
 * @async
 * @tparam number|string source The source to update.
 * @tparam Volume volume
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_source_volume(lua_State*);
int context_set_source_volume_by_name(lua_State*);
int context_set_source_volume_by_index(lua_State*);

/** Sets the source's mute state.
 *
 * The source may be indicated by either its name or its index.
 *
 * @function Context:set_source_mute
 * @async
 * @tparam number|string source The source to update.
 * @tparam boolean mute
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_source_mute(lua_State*);
int context_set_source_mute_by_name(lua_State*);
int context_set_source_mute_by_index(lua_State*);

/** Sets the source's suspended state.
 *
 * The source may be indicated by either its name or its index.
 *
 * @function Context:set_source_suspended
 * @async
 * @tparam number|string source The source to update.
 * @tparam boolean suspended
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_source_suspended(lua_State*);
int context_set_source_suspended_by_name(lua_State*);
int context_set_source_suspended_by_index(lua_State*);


// Source Outputs

/** Gets information about the given source output.
 *
 * See [pa_source_output_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__source_output__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_source_output_info
 * @async
 * @tparam number source_output The index of the source output to query.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_source_output_info(lua_State*);

/** Gets information about all source outputs.
 *
 * This returns the same information as @{Context:get_source_output_info} would have returned for every source output
 * that's currently registered at the server.
 *
 * See [pa_source_output_info](https://freedesktop.org/software/pulseaudio/doxygen/structpa__source_output__info.html)
 * for documentation on the return type.
 *
 * @function Context:get_source_outputs
 * @async
 * @tparam function cb
 * @treturn[opt] string
 * @treturn table
 */
int context_get_source_output_info_list(lua_State*);

/** Moves the source output to a different name.
 *
 * The target source may be indicated by either its name or its index.
 *
 * @function Context:set_source_suspended
 * @async
 * @tparam number The source output to move.
 * @tparam number|string source The source to update.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_move_source_output(lua_State*);
int context_move_source_output_by_name(lua_State*);
int context_move_source_output_by_index(lua_State*);


/** Sets the source output's volume to the given value.
 *
 * @function Context:set_source_output_volume
 * @async
 * @tparam number source_output The source output to update.
 * @tparam Volume volume
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_source_output_volume(lua_State*);

/** Sets the source output's mute state.
 *
 * @function Context:set_source_output_mute
 * @async
 * @tparam number source_output The source output to update.
 * @tparam boolean mute
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_set_source_output_mute(lua_State*);

/** Kills the source output.
 *
 * @function Context:kill_source_output
 * @async
 * @tparam number source_output The source output to kill.
 * @tparam function cb
 * @treturn[opt] string
 * @treturn boolean
 */
int context_kill_source_output(lua_State*);


static const struct luaL_Reg context_mt[] = {
    {"__gc", context__gc},
    { NULL,  NULL       }
};


static const struct luaL_Reg context_f[] = {
    {"connect",                   context_connect                    },
    { "disconnect",               context_disconnect                 },
    { "get_server_info",          context_get_server_info            },
    { "get_sinks",                context_get_sink_info_list         },
    { "get_sink_info",            context_get_sink_info              },
    { "set_sink_volume",          context_set_sink_volume            },
    { "set_sink_mute",            context_set_sink_mute              },
    { "set_sink_suspended",       context_set_sink_suspended         },
    { "get_sink_inputs",          context_get_sink_input_info_list   },
    { "get_sink_input_info",      context_get_sink_input_info        },
    { "set_sink_input_volume",    context_set_sink_input_volume      },
    { "set_sink_input_mute",      context_set_sink_input_mute        },
    { "move_sink_input",          context_move_sink_input            },
    { "kill_sink_input",          context_kill_sink_input            },
    { "set_source_volume",        context_set_source_volume          },
    { "set_source_mute",          context_set_source_mute            },
    { "set_source_suspended",     context_set_source_suspended       },
    { "get_sources",              context_get_source_info_list       },
    { "get_source_info",          context_get_source_info            },
    { "get_source_outputs",       context_get_source_output_info_list},
    { "get_source_output_info",   context_get_source_output_info     },
    { "set_source_output_volume", context_set_source_output_volume   },
    { "set_source_output_mute",   context_set_source_output_mute     },
    { "move_source_output",       context_move_source_output         },
    { "kill_source_output",       context_kill_source_output         },
    { NULL,                       NULL                               }
};
