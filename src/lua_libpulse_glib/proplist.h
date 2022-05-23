/** Bindings for PulseAudio's property lists.
 *
 * Basic operations are mapped to Lua table operations:
 *
 * - get a value: `plist[key]`
 * - set a value: `plist[key] = value`
 * - get the size: `#plist`
 * - get default string presentation: `tostring(plist)`
 * - equality: `plist == other`
 *
 * Additional operations are exposed as methods, as documented below.
 *
 * @module lua_libpulse_glib.proplist
 */
#pragma once

#include "lua_util.h"

#include <lauxlib.h>
#include <lua.h>
#include <pulse/proplist.h>
#include <stdbool.h>

#define LUA_PA_PROPLIST "lua_libpulse_glib.proplist"


typedef struct proplist {
    pa_proplist* plist;
} proplist;


/** A list of well-known keys. These will likely be used by other consumers
 * of PulseAudio, so should be preferred for interoperatibility.
 *
 * See [proplist.h](https://freedesktop.org/software/pulseaudio/doxygen/proplist_8h.html) for details on
 * these values.
 *
 *     local proplist = require("pulseaudio.proplist")
 *     proplist.is_key_valid(proplist.MEDIA_NAME) -- true
 *     print(proplist.MEDIA_NAME) -- media.name
 *
 * @table pulseaudio.proplist
 * @field MEDIA_NAME media.name
 * @field MEDIA_TITLE media.title
 * @field MEDIA_ARTIST media.artist
 * @field MEDIA_COPYRIGHT media.copyright
 * @field MEDIA_SOFTWARE media.software
 * @field MEDIA_LANGUAGE media.language
 * @field MEDIA_FILENAME media.filename
 * @field MEDIA_ICON media.icon
 * @field MEDIA_ICON_NAME media.icon_name
 * @field MEDIA_ROLE media.role
 * @field FILTER_WANT filter.want
 * @field FILTER_APPLY filter.apply
 * @field FILTER_SUPPRESS filter.suppress
 * @field EVENT_ID event.id
 * @field EVENT_DESCRIPTION event.description
 * @field EVENT_MOUSE_X event.mouse.x
 * @field EVENT_MOUSE_Y event.mouse.y
 * @field EVENT_MOUSE_HPOS event.mouse.hpos
 * @field EVENT_MOUSE_VPOS event.mouse.vpos
 * @field EVENT_MOUSE_BUTTON event.mouse.button
 * @field WINDOW_NAME window.name
 * @field WINDOW_ID window.id
 * @field WINDOW_ICON window.icon
 * @field WINDOW_ICON_NAME window.icon_name
 * @field WINDOW_X window.x
 * @field WINDOW_Y window.y
 * @field WINDOW_WIDTH window.width
 * @field WINDOW_HEIGHT window.height
 * @field WINDOW_HPOS window.hpos
 * @field WINDOW_VPOS window.vpos
 * @field WINDOW_DESKTOP window.desktop
 * @field WINDOW_X11_DISPLAY window.x11.display
 * @field WINDOW_X11_SCREEN window.x11.screen
 * @field WINDOW_X11_MONITOR window.x11.monitor
 * @field WINDOW_X11_XID window.x11.xid
 * @field APPLICATION_NAME application.name
 * @field APPLICATION_ID application.id
 * @field APPLICATION_VERSION application.version
 * @field APPLICATION_ICON application.icon
 * @field APPLICATION_ICON_NAME application.icon_name
 * @field APPLICATION_LANGUAGE application.language
 * @field APPLICATION_PROCESS_ID application.process.id
 * @field APPLICATION_PROCESS_BINARY application.process.binary
 * @field APPLICATION_PROCESS_USER application.process.user
 * @field APPLICATION_PROCESS_HOST application.process.host
 * @field APPLICATION_PROCESS_MACHINE_ID application.process.machine_id
 * @field APPLICATION_PROCESS_SESSION_ID application.process.session_id
 * @field DEVICE_STRING device.string
 * @field DEVICE_API device.api
 * @field DEVICE_DESCRIPTION device.description
 * @field DEVICE_BUS_PATH device.bus_path
 * @field DEVICE_SERIAL device.serial
 * @field DEVICE_VENDOR_ID device.vendor.id
 * @field DEVICE_VENDOR_NAME device.vendor.name
 * @field DEVICE_PRODUCT_ID device.product.id
 * @field DEVICE_PRODUCT_NAME device.product.name
 * @field DEVICE_CLASS device.class
 * @field DEVICE_FORM_FACTOR device.form_factor
 * @field DEVICE_BUS device.bus
 * @field DEVICE_ICON device.icon
 * @field DEVICE_ICON_NAME device.icon_name
 * @field DEVICE_ACCESS_MODE device.access_mode
 * @field DEVICE_MASTER_DEVICE device.master_device
 * @field DEVICE_BUFFERING_BUFFER_SIZE device.buffering.buffer_size
 * @field DEVICE_BUFFERING_FRAGMENT_SIZE device.buffering.fragment_size
 * @field DEVICE_PROFILE_NAME device.profile.name
 * @field DEVICE_INTENDED_ROLES device.intended_roles
 * @field DEVICE_PROFILE_DESCRIPTION device.profile.description
 * @field MODULE_AUTHOR module.author
 * @field MODULE_DESCRIPTION module.description
 * @field MODULE_USAGE module.usage
 * @field MODULE_VERSION module.version
 * @field FORMAT_SAMPLE_FORMAT format.sample_format
 * @field FORMAT_RATE format.rate
 * @field FORMAT_CHANNELS format.channels
 * @field FORMAT_CHANNEL_MAP format.channel_map
 * @field CONTEXT_FORCE_DISABLE_SHM context.force.disable.shm
 * @field BLUETOOTH_CODEC bluetooth.codec
 */


/// Constructor functions.
/// @section constructors


/** Creates a new, empty property list.
 *
 * @function new
 * @return[type=PropList]
 */
int proplist_new(lua_State*);

/** Parses a string into a @{PropList}.
 *
 * @function from_string
 * @param[type=string] str The string to parse.
 * @return[type=PropList]
 */
int proplist_from_string(lua_State*);


/// Static functions
/// @section static


/** Checks if the given string is a valid key.
 *
 * @function is_key_valid
 * @param[type=string] key The string to check.
 * @return[type=boolean]
 */
int proplist_key_valid(lua_State*);


// Internal functions

int proplist_to_lua(lua_State*, pa_proplist*);
int proplist__index(lua_State*);
int proplist__newindex(lua_State*);
int proplist__gc(lua_State*);
int proplist__len(lua_State*);
int proplist__eq(lua_State*);
int proplist__tostring(lua_State*);


/// Methods
/// @type PropList


/** Checks if the proplist is empty.
 *
 * @function is_empty
 * @return[type=boolean]
 */
int proplist_isempty(lua_State*);

/** Creates a string representation with a custom separator.
 *
 * @function tostring_sep
 * @return[type=string]
 */
int proplist_tostring_sep(lua_State*);

/** Removes all keys from the proplist
 *
 * @function clear
 */
int proplist_clear(lua_State*);

/** Checks if the proplist contains the given key.
 *
 * @function contains
 * @param[type=string] key The key to check for.
 * @return[type=boolean]
 */
int proplist_contains(lua_State*);

/** Duplicates the proplist.
 *
 * @function copy
 * @return[type=proplist]
 */
int proplist_copy(lua_State*);


static const struct luaL_Reg proplist_f[] = {
    {"clear",         proplist_clear       },
    { "contains",     proplist_contains    },
    { "copy",         proplist_copy        },
    { "is_empty",     proplist_isempty     },
    { "tostring_sep", proplist_tostring_sep},
    { NULL,           NULL                 }
};


static const struct luaL_Reg proplist_mt[] = {
    {"__gc",        proplist__gc      },
    { "__len",      proplist__len     },
    { "__tostring", proplist__tostring},
    { "__eq",       proplist__eq      },
    { "__index",    proplist__index   },
    { "__newindex", proplist__newindex},
    { NULL,         NULL              }
};


static const struct luaL_Reg proplist_lib[] = {
    {"new",           proplist_new        },
    { "is_key_valid", proplist_key_valid  },
    { "from_string",  proplist_from_string},
    { NULL,           NULL                }
};


static const struct luaU_enumfield proplist_enum[] = {
    {"MEDIA_TITLE",                     PA_PROP_MEDIA_TITLE                   },
    { "MEDIA_ARTIST",                   PA_PROP_MEDIA_ARTIST                  },
    { "MEDIA_COPYRIGHT",                PA_PROP_MEDIA_COPYRIGHT               },
    { "MEDIA_SOFTWARE",                 PA_PROP_MEDIA_SOFTWARE                },
    { "MEDIA_LANGUAGE",                 PA_PROP_MEDIA_LANGUAGE                },
    { "MEDIA_FILENAME",                 PA_PROP_MEDIA_FILENAME                },
    { "MEDIA_ICON",                     PA_PROP_MEDIA_ICON                    },
    { "MEDIA_ICON_NAME",                PA_PROP_MEDIA_ICON_NAME               },
    { "MEDIA_ROLE",                     PA_PROP_MEDIA_ROLE                    },
    { "FILTER_WANT",                    PA_PROP_FILTER_WANT                   },
    { "FILTER_APPLY",                   PA_PROP_FILTER_APPLY                  },
    { "FILTER_SUPPRESS",                PA_PROP_FILTER_SUPPRESS               },
    { "EVENT_ID",                       PA_PROP_EVENT_ID                      },
    { "EVENT_DESCRIPTION",              PA_PROP_EVENT_DESCRIPTION             },
    { "EVENT_MOUSE_X",                  PA_PROP_EVENT_MOUSE_X                 },
    { "EVENT_MOUSE_Y",                  PA_PROP_EVENT_MOUSE_Y                 },
    { "EVENT_MOUSE_HPOS",               PA_PROP_EVENT_MOUSE_HPOS              },
    { "EVENT_MOUSE_VPOS",               PA_PROP_EVENT_MOUSE_VPOS              },
    { "EVENT_MOUSE_BUTTON",             PA_PROP_EVENT_MOUSE_BUTTON            },
    { "WINDOW_NAME",                    PA_PROP_WINDOW_NAME                   },
    { "WINDOW_ID",                      PA_PROP_WINDOW_ID                     },
    { "WINDOW_ICON",                    PA_PROP_WINDOW_ICON                   },
    { "WINDOW_ICON_NAME",               PA_PROP_WINDOW_ICON_NAME              },
    { "WINDOW_X",                       PA_PROP_WINDOW_X                      },
    { "WINDOW_Y",                       PA_PROP_WINDOW_Y                      },
    { "WINDOW_WIDTH",                   PA_PROP_WINDOW_WIDTH                  },
    { "WINDOW_HEIGHT",                  PA_PROP_WINDOW_HEIGHT                 },
    { "WINDOW_HPOS",                    PA_PROP_WINDOW_HPOS                   },
    { "WINDOW_VPOS",                    PA_PROP_WINDOW_VPOS                   },
    { "WINDOW_DESKTOP",                 PA_PROP_WINDOW_DESKTOP                },
    { "WINDOW_X11_DISPLAY",             PA_PROP_WINDOW_X11_DISPLAY            },
    { "WINDOW_X11_SCREEN",              PA_PROP_WINDOW_X11_SCREEN             },
    { "WINDOW_X11_MONITOR",             PA_PROP_WINDOW_X11_MONITOR            },
    { "WINDOW_X11_XID",                 PA_PROP_WINDOW_X11_XID                },
    { "APPLICATION_NAME",               PA_PROP_APPLICATION_NAME              },
    { "APPLICATION_ID",                 PA_PROP_APPLICATION_ID                },
    { "APPLICATION_VERSION",            PA_PROP_APPLICATION_VERSION           },
    { "APPLICATION_ICON",               PA_PROP_APPLICATION_ICON              },
    { "APPLICATION_ICON_NAME",          PA_PROP_APPLICATION_ICON_NAME         },
    { "APPLICATION_LANGUAGE",           PA_PROP_APPLICATION_LANGUAGE          },
    { "APPLICATION_PROCESS_ID",         PA_PROP_APPLICATION_PROCESS_ID        },
    { "APPLICATION_PROCESS_BINARY",     PA_PROP_APPLICATION_PROCESS_BINARY    },
    { "APPLICATION_PROCESS_USER",       PA_PROP_APPLICATION_PROCESS_USER      },
    { "APPLICATION_PROCESS_HOST",       PA_PROP_APPLICATION_PROCESS_HOST      },
    { "APPLICATION_PROCESS_MACHINE_ID", PA_PROP_APPLICATION_PROCESS_MACHINE_ID},
    { "APPLICATION_PROCESS_SESSION_ID", PA_PROP_APPLICATION_PROCESS_SESSION_ID},
    { "DEVICE_STRING",                  PA_PROP_DEVICE_STRING                 },
    { "DEVICE_API",                     PA_PROP_DEVICE_API                    },
    { "DEVICE_DESCRIPTION",             PA_PROP_DEVICE_DESCRIPTION            },
    { "DEVICE_BUS_PATH",                PA_PROP_DEVICE_BUS_PATH               },
    { "DEVICE_SERIAL",                  PA_PROP_DEVICE_SERIAL                 },
    { "DEVICE_VENDOR_ID",               PA_PROP_DEVICE_VENDOR_ID              },
    { "DEVICE_VENDOR_NAME",             PA_PROP_DEVICE_VENDOR_NAME            },
    { "DEVICE_PRODUCT_ID",              PA_PROP_DEVICE_PRODUCT_ID             },
    { "DEVICE_PRODUCT_NAME",            PA_PROP_DEVICE_PRODUCT_NAME           },
    { "DEVICE_CLASS",                   PA_PROP_DEVICE_CLASS                  },
    { "DEVICE_FORM_FACTOR",             PA_PROP_DEVICE_FORM_FACTOR            },
    { "DEVICE_BUS",                     PA_PROP_DEVICE_BUS                    },
    { "DEVICE_ICON",                    PA_PROP_DEVICE_ICON                   },
    { "DEVICE_ICON_NAME",               PA_PROP_DEVICE_ICON_NAME              },
    { "DEVICE_ACCESS_MODE",             PA_PROP_DEVICE_ACCESS_MODE            },
    { "DEVICE_MASTER_DEVICE",           PA_PROP_DEVICE_MASTER_DEVICE          },
    { "DEVICE_BUFFERING_BUFFER_SIZE",   PA_PROP_DEVICE_BUFFERING_BUFFER_SIZE  },
    { "DEVICE_BUFFERING_FRAGMENT_SIZE", PA_PROP_DEVICE_BUFFERING_FRAGMENT_SIZE},
    { "DEVICE_PROFILE_NAME",            PA_PROP_DEVICE_PROFILE_NAME           },
    { "DEVICE_INTENDED_ROLES",          PA_PROP_DEVICE_INTENDED_ROLES         },
    { "DEVICE_PROFILE_DESCRIPTION",     PA_PROP_DEVICE_PROFILE_DESCRIPTION    },
    { "MODULE_AUTHOR",                  PA_PROP_MODULE_AUTHOR                 },
    { "MODULE_DESCRIPTION",             PA_PROP_MODULE_DESCRIPTION            },
    { "MODULE_USAGE",                   PA_PROP_MODULE_USAGE                  },
    { "MODULE_VERSION",                 PA_PROP_MODULE_VERSION                },
    { "FORMAT_SAMPLE_FORMAT",           PA_PROP_FORMAT_SAMPLE_FORMAT          },
    { "FORMAT_RATE",                    PA_PROP_FORMAT_RATE                   },
    { "FORMAT_CHANNELS",                PA_PROP_FORMAT_CHANNELS               },
    { "FORMAT_CHANNEL_MAP",             PA_PROP_FORMAT_CHANNEL_MAP            },
    { "CONTEXT_FORCE_DISABLE_SHM",      PA_PROP_CONTEXT_FORCE_DISABLE_SHM     },
    { "BLUETOOTH_CODEC",                PA_PROP_BLUETOOTH_CODEC               },
    { NULL,                             NULL                                  }
};
