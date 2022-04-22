/** Bindings for libpulse's `pa_cvolume`.
 *
 * Contrary to libpulse, methods that change the a @{Volume} object generally don't return anything, but
 * instead change the instance itself.
 *
 * @module pulseaudio.volume
 */
#ifndef volume_h_INCLUDED
#define volume_h_INCLUDED

#include <lauxlib.h>
#include <lua.h>
#include <pulse/volume.h>

#define LUA_PA_VOLUME "pulseaudio.volume"


typedef struct volume_t {
    pa_cvolume inner;
} volume_t;


/* Creates a Lua userdatum from/for a PulseAudio volume.
 *
 * Control over the `pa_plist`'s memory has to be taken over by this function,
 * to enable integration into Lua's garbage collection.
 *
 * @function volume_to_lua
 * @treturn Volume
 */
int volume_to_lua(lua_State*, const pa_cvolume*);

/* Creates a pa_cvolume from Lua userdata
 */
pa_cvolume* volume_from_lua(lua_State*, int);

/* Implements the `#` length operator.
 * This simply proxies to the `.channels` value.
 */
int volume__len(lua_State*);
int volume__eq(lua_State*);
int volume__tostring(lua_State*);
int volume__index(lua_State*);
int volume__newindex(lua_State*);


/// Static Functions
/// @section static


/** Checks whether a value is a valid @{Volume}.
 *
 * @function is_valid
 * @tparam any value The value to check
 * @treturn boolean
 */
int volume_is_valid(lua_State*);

/// Volume
/// @type Volume

/** Returns the average volume over all channels.
 *
 * @function Volume:avg
 * @treturn number
 */
int volume_avg(lua_State*);

/** Returns `true` if the volume of all channels is equal to the specified value.
 *
 * @function Volume:channels_equal_to
 * @tparam number value Volume to compare to
 * @treturn boolean
 */
int volume_channels_equal_to(lua_State*);

/** Decreases the volume by the given amount.
 *
 * The proportions between the channels are kept.
 *
 * @function Volume:dec
 * @tparam number value The value to decrease by.
 */
int volume_dec(lua_State*);

/** Divides the volume by the given value.
 *
 * The value to divide by may either be a scalar, that's applied to all channels,
 * or another instance of @{Volume}, which would be applied channel by channel.
 *
 * It is possible to divide a @{Volume} by itself.
 *
 * This is only valid for software volumes.
 *
 * @function Volume:divide
 * @tparam number|Volume value The volume to divide by.
 */
int volume_divide(lua_State*);

/** Returns the volume of a single channel.
 *
 * @function Volume:get
 * @tparam number index The channel index
 * @treturn number The channel's volume
 */
int volume_get(lua_State*);

/** Increases the volume by the given amount.
 *
 * The proportions between the channels are kept.
 *
 * @function Volume:inc
 * @tparam number value the value to increase by.
 */
int volume_inc(lua_State*);

/** Returns `true` when all channels are muted.
 *
 * @function Volume:is_muted
 * @treturn boolean
 */
int volume_is_muted(lua_State*);

/** Returns `true` when all channels are on normal level.
 *
 * @function Volume:is_norm
 * @treturn boolean
 */
int volume_is_norm(lua_State*);

/** Returns the maximum volume out of all channels.
 *
 * @function Volume:max
 * @treturn number
 */
int volume_max(lua_State*);

/** Returns the minimum volume out of all channels.
 *
 * @function Volume:min
 * @treturn number
 */
int volume_min(lua_State*);

/** Multiplies the volume by the given amount.
 *
 * The value to multiply with may either be a scalar, that's applied to all channels,
 * or another instance of @{Volume}, which would be applied channel by channel.
 *
 * It is possible to multiply a @{Volume} by itself.
 *
 * This is only valid for software volumes.
 *
 * @function Volume:multiply
 * @tparam number|Volume value
 */
int volume_multiply(lua_State*);

/** Mutes all channels.
 *
 * @function Volume:mute
 */
int volume_mute(lua_State*);

/** Resets all channels to normal volume.
 *
 * @function Volume:reset
 */
int volume_reset(lua_State*);

/** Scales all channels to the passed amount.
 *
 * This adjust all channel volumes so that the maximum between them equals the given value, while
 * keeping proportions between channels the same.
 *
 * @function Volume:scale
 * @tparam number value The value to scale to.
 */
int volume_scale(lua_State*);

/** Sets a channel to the given value.
 *
 * @function Volume:set
 * @tparam number index The channel index.
 * @tparam number value The volume to set to.
 */
int volume_set(lua_State*);

/** Sets a number of channels to the given volume value.
 *
 * @function Volume:set_channels
 * @tparam number channels The number of channels to set
 * @tparam number value The volume to set to.
 */
int volume_set_channels(lua_State*);


static const struct luaL_Reg volume_f[] = {
    {"avg",                volume_avg              },
    { "channels_equal_to", volume_channels_equal_to},
    { "dec",               volume_dec              },
    { "divide",            volume_divide           },
    { "get",               volume_get              },
    { "inc",               volume_inc              },
    { "is_muted",          volume_is_muted         },
    { "is_norm",           volume_is_norm          },
    { "is_valid",          volume_is_valid         },
    { "max",               volume_max              },
    { "min",               volume_min              },
    { "multiply",          volume_multiply         },
    { "mute",              volume_mute             },
    { "reset",             volume_reset            },
    { "scale",             volume_scale            },
    { "set",               volume_set              },
    { NULL,                NULL                    }
};


// We don't add `__index` here, as it needs to be handled specially,
// to allow indexing both by channel index as well as accessing the methods above.
static const struct luaL_Reg volume_mt[] = {
    {"__len",       volume__len     },
    { "__tostring", volume__tostring},
    { "__eq",       volume__eq      },
    { "__newindex", volume__newindex},
    { NULL,         NULL            }
};


static const struct luaL_Reg volume_lib[] = {
    {"is_valid", volume_is_valid},
    { NULL,      NULL           }
};


#endif // volume_h_INCLUDED

