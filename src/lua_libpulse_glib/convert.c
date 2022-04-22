#include "convert.h"

#include "proplist.h"
#include "volume.h"

#include <pulse/xmalloc.h>


void channel_map_to_lua(lua_State* L, const pa_channel_map* spec) {
    lua_createtable(L, spec->channels, 0);
    int table_index = lua_gettop(L);

    for (int i = 0; i < spec->channels; ++i) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, spec->map[i]);
        lua_settable(L, table_index);
    }
}


void sample_spec_to_lua(lua_State* L, const pa_sample_spec* spec) {
    lua_createtable(L, 0, 3);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "rate");
    lua_pushinteger(L, spec->rate);
    lua_settable(L, table_index);

    lua_pushstring(L, "channels");
    lua_pushinteger(L, spec->channels);
    lua_settable(L, table_index);

    lua_pushstring(L, "format");
    lua_pushinteger(L, spec->format);
    lua_settable(L, table_index);
}


void sink_port_info_to_lua(lua_State* L, const pa_sink_port_info* info) {
    lua_createtable(L, 0, 6);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "priority");
    lua_pushinteger(L, info->priority);
    lua_settable(L, table_index);

    lua_pushstring(L, "available");
    lua_pushinteger(L, info->available);
    lua_settable(L, table_index);

    lua_pushstring(L, "availability_group");
    lua_pushstring(L, info->availability_group);
    lua_settable(L, table_index);

    lua_pushstring(L, "type");
    lua_pushinteger(L, info->type);
    lua_settable(L, table_index);
}


void source_port_info_to_lua(lua_State* L, const pa_source_port_info* info) {
    lua_createtable(L, 0, 6);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "priority");
    lua_pushinteger(L, info->priority);
    lua_settable(L, table_index);

    lua_pushstring(L, "available");
    lua_pushinteger(L, info->available);
    lua_settable(L, table_index);

    lua_pushstring(L, "availability_group");
    lua_pushstring(L, info->availability_group);
    lua_settable(L, table_index);

    lua_pushstring(L, "type");
    lua_pushinteger(L, info->type);
    lua_settable(L, table_index);
}


void format_info_to_lua(lua_State* L, const pa_format_info* info) {
    lua_createtable(L, 0, 2);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "encoding");
    lua_pushinteger(L, info->encoding);
    lua_settable(L, table_index);

    lua_pushstring(L, "plist");
    proplist_to_lua(L, pa_proplist_copy(info->plist));
    lua_settable(L, table_index);
}


void sink_ports_to_lua(lua_State* L, pa_sink_port_info** list, int n_ports, pa_sink_port_info* active) {
    lua_createtable(L, n_ports, 1);
    int table_index = lua_gettop(L);

    for (int i = 0; i < n_ports; ++i) {
        lua_pushinteger(L, i + 1);
        sink_port_info_to_lua(L, list[i]);

        if (list[i] == active) {
            lua_pushstring(L, "active");
            lua_pushvalue(L, -2);
            lua_settable(L, table_index);
        }

        lua_settable(L, table_index);
    }
}


void source_ports_to_lua(lua_State* L, pa_source_port_info** list, int n_ports, pa_source_port_info* active) {
    lua_createtable(L, n_ports, 1);
    int table_index = lua_gettop(L);

    for (int i = 0; i < n_ports; ++i) {
        lua_pushinteger(L, i + 1);
        source_port_info_to_lua(L, list[i]);

        if (list[i] == active) {
            lua_pushstring(L, "active");
            lua_pushvalue(L, -2);
            lua_settable(L, table_index);
        }

        lua_settable(L, table_index);
    }
}


void formats_to_lua(lua_State* L, pa_format_info** list, int n_formats) {
    lua_createtable(L, n_formats, 0);
    int table_index = lua_gettop(L);

    for (int i = 0; i < n_formats; ++i) {
        lua_pushinteger(L, i + 1);
        format_info_to_lua(L, list[i]);
        lua_settable(L, table_index);
    }
}


void sink_info_to_lua(lua_State* L, const pa_sink_info* info) {
    lua_createtable(L, 0, 24);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "index");
    // Convert C's 0-based index to Lua's 1-base
    lua_pushinteger(L, info->index + 1);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);

    lua_pushstring(L, "owner_module");
    lua_pushinteger(L, info->owner_module);
    lua_settable(L, table_index);

    lua_pushstring(L, "volume");
    volume_to_lua(L, &info->volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "mute");
    lua_pushinteger(L, info->mute);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_source");
    lua_pushinteger(L, info->monitor_source);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_source_name");
    lua_pushstring(L, info->monitor_source_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "latency");
    lua_pushinteger(L, info->latency);
    lua_settable(L, table_index);

    lua_pushstring(L, "driver");
    lua_pushstring(L, info->driver);
    lua_settable(L, table_index);

    lua_pushstring(L, "flags");
    lua_pushinteger(L, info->flags);
    lua_settable(L, table_index);

    lua_pushstring(L, "proplist");
    proplist_to_lua(L, pa_proplist_copy(info->proplist));
    lua_settable(L, table_index);

    lua_pushstring(L, "base_volume");
    lua_pushinteger(L, info->base_volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "state");
    lua_pushinteger(L, info->state);
    lua_settable(L, table_index);

    lua_pushstring(L, "n_volume_steps");
    lua_pushinteger(L, info->n_volume_steps);
    lua_settable(L, table_index);

    lua_pushstring(L, "card");
    lua_pushinteger(L, info->card);
    lua_settable(L, table_index);

    lua_pushstring(L, "ports");
    sink_ports_to_lua(L, info->ports, info->n_ports, info->active_port);
    lua_settable(L, table_index);

    lua_pushstring(L, "formats");
    formats_to_lua(L, info->formats, info->n_formats);
    lua_settable(L, table_index);
}


void source_info_to_lua(lua_State* L, const pa_source_info* info) {
    lua_createtable(L, 0, 24);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "index");
    // Convert C's 0-based index to Lua's 1-base
    lua_pushinteger(L, info->index + 1);
    lua_settable(L, table_index);

    lua_pushstring(L, "description");
    lua_pushstring(L, info->description);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);

    lua_pushstring(L, "owner_module");
    lua_pushinteger(L, info->owner_module);
    lua_settable(L, table_index);

    lua_pushstring(L, "volume");
    volume_to_lua(L, &info->volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "mute");
    lua_pushinteger(L, info->mute);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_of_sink");
    lua_pushinteger(L, info->monitor_of_sink);
    lua_settable(L, table_index);

    lua_pushstring(L, "monitor_of_sink_name");
    lua_pushstring(L, info->monitor_of_sink_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "latency");
    lua_pushinteger(L, info->latency);
    lua_settable(L, table_index);

    lua_pushstring(L, "driver");
    lua_pushstring(L, info->driver);
    lua_settable(L, table_index);

    lua_pushstring(L, "flags");
    lua_pushinteger(L, info->flags);
    lua_settable(L, table_index);

    lua_pushstring(L, "proplist");
    proplist_to_lua(L, pa_proplist_copy(info->proplist));
    lua_settable(L, table_index);

    lua_pushstring(L, "configured_latency");
    lua_pushinteger(L, info->configured_latency);
    lua_settable(L, table_index);

    lua_pushstring(L, "base_volume");
    lua_pushinteger(L, info->base_volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "state");
    lua_pushinteger(L, info->state);
    lua_settable(L, table_index);

    lua_pushstring(L, "n_volume_steps");
    lua_pushinteger(L, info->n_volume_steps);
    lua_settable(L, table_index);

    lua_pushstring(L, "card");
    lua_pushinteger(L, info->card);
    lua_settable(L, table_index);

    lua_pushstring(L, "ports");
    source_ports_to_lua(L, info->ports, info->n_ports, info->active_port);
    lua_settable(L, table_index);

    lua_pushstring(L, "formats");
    formats_to_lua(L, info->formats, info->n_formats);
    lua_settable(L, table_index);
}


void server_info_to_lua(lua_State* L, const pa_server_info* info) {
    lua_createtable(L, 0, 9);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "user_name");
    lua_pushstring(L, info->user_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "host_name");
    lua_pushstring(L, info->host_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "server_version");
    lua_pushstring(L, info->server_version);
    lua_settable(L, table_index);

    lua_pushstring(L, "server_name");
    lua_pushstring(L, info->server_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "default_sink_name");
    lua_pushstring(L, info->default_sink_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "default_source_name");
    lua_pushstring(L, info->default_source_name);
    lua_settable(L, table_index);

    lua_pushstring(L, "cookie");
    lua_pushinteger(L, info->cookie);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);
}


void sink_input_info_to_lua(lua_State* L, const pa_sink_input_info* info) {
    lua_createtable(L, 0, 15);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "index");
    lua_pushinteger(L, info->index + 1);
    lua_settable(L, table_index);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "owner_module");
    lua_pushinteger(L, info->owner_module);
    lua_settable(L, table_index);

    lua_pushstring(L, "client");
    lua_pushinteger(L, info->client);
    lua_settable(L, table_index);

    lua_pushstring(L, "sink");
    lua_pushinteger(L, info->sink);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);

    lua_pushstring(L, "has_volume");
    lua_pushboolean(L, info->has_volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "volume_writable");
    lua_pushboolean(L, info->volume_writable);
    lua_settable(L, table_index);

    if (info->has_volume) {
        lua_pushstring(L, "volume");
        volume_to_lua(L, &info->volume);
        lua_settable(L, table_index);
    }

    lua_pushstring(L, "buffer_usec");
    lua_pushinteger(L, info->buffer_usec);
    lua_settable(L, table_index);

    lua_pushstring(L, "sink_usec");
    lua_pushinteger(L, info->sink_usec);
    lua_settable(L, table_index);

    lua_pushstring(L, "resample_method");
    lua_pushstring(L, info->resample_method);
    lua_settable(L, table_index);

    lua_pushstring(L, "driver");
    lua_pushstring(L, info->driver);
    lua_settable(L, table_index);

    lua_pushstring(L, "mute");
    lua_pushboolean(L, info->mute);
    lua_settable(L, table_index);

    lua_pushstring(L, "format");
    format_info_to_lua(L, info->format);
    lua_settable(L, table_index);

    lua_pushstring(L, "proplist");
    proplist_to_lua(L, pa_proplist_copy(info->proplist));
    lua_settable(L, table_index);
}


void source_output_info_to_lua(lua_State* L, const pa_source_output_info* info) {
    lua_createtable(L, 0, 15);
    int table_index = lua_gettop(L);

    lua_pushstring(L, "index");
    lua_pushinteger(L, info->index + 1);
    lua_settable(L, table_index);

    lua_pushstring(L, "name");
    lua_pushstring(L, info->name);
    lua_settable(L, table_index);

    lua_pushstring(L, "owner_module");
    lua_pushinteger(L, info->owner_module);
    lua_settable(L, table_index);

    lua_pushstring(L, "client");
    lua_pushinteger(L, info->client);
    lua_settable(L, table_index);

    lua_pushstring(L, "source");
    lua_pushinteger(L, info->source);
    lua_settable(L, table_index);

    lua_pushstring(L, "sample_spec");
    sample_spec_to_lua(L, &info->sample_spec);
    lua_settable(L, table_index);

    lua_pushstring(L, "channel_map");
    channel_map_to_lua(L, &info->channel_map);
    lua_settable(L, table_index);

    lua_pushstring(L, "has_volume");
    lua_pushboolean(L, info->has_volume);
    lua_settable(L, table_index);

    lua_pushstring(L, "volume_writable");
    lua_pushboolean(L, info->volume_writable);
    lua_settable(L, table_index);

    if (info->has_volume) {
        lua_pushstring(L, "volume");
        volume_to_lua(L, &info->volume);
        lua_settable(L, table_index);
    }

    lua_pushstring(L, "buffer_usec");
    lua_pushinteger(L, info->buffer_usec);
    lua_settable(L, table_index);

    lua_pushstring(L, "source_usec");
    lua_pushinteger(L, info->source_usec);
    lua_settable(L, table_index);

    lua_pushstring(L, "resample_method");
    lua_pushstring(L, info->resample_method);
    lua_settable(L, table_index);

    lua_pushstring(L, "driver");
    lua_pushstring(L, info->driver);
    lua_settable(L, table_index);

    lua_pushstring(L, "mute");
    lua_pushboolean(L, info->mute);
    lua_settable(L, table_index);

    lua_pushstring(L, "format");
    format_info_to_lua(L, info->format);
    lua_settable(L, table_index);

    lua_pushstring(L, "proplist");
    proplist_to_lua(L, pa_proplist_copy(info->proplist));
    lua_settable(L, table_index);
}
