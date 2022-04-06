#pragma once

#include "lua.h"

int
context_get_server_info(lua_State*);
int
context_get_sink_info_list(lua_State*);
int
context_get_source_info_list(lua_State*);
