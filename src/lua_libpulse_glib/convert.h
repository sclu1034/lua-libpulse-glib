#pragma once

#include <lua.h>
#include <pulse/error.h>
#include <pulse/introspect.h>


void channel_map_to_lua(lua_State*, const pa_channel_map*);
void sample_spec_to_lua(lua_State*, const pa_sample_spec*);
void sink_port_info_to_lua(lua_State*, const pa_sink_port_info*);
void source_port_info_to_lua(lua_State*, const pa_source_port_info*);
void format_info_to_lua(lua_State*, const pa_format_info*);
void sink_ports_to_lua(lua_State*, pa_sink_port_info**, int, pa_sink_port_info*);
void source_ports_to_lua(lua_State*, pa_source_port_info**, int, pa_source_port_info*);
void formats_to_lua(lua_State*, pa_format_info**, int);
void sink_info_to_lua(lua_State*, const pa_sink_info*);
void source_info_to_lua(lua_State*, const pa_source_info*);
void server_info_to_lua(lua_State*, const pa_server_info*);
void sink_input_info_to_lua(lua_State*, const pa_sink_input_info*);
void source_output_info_to_lua(lua_State*, const pa_source_output_info*);
