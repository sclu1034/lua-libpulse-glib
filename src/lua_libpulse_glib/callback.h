#ifndef callback_h_INCLUDED
#define callback_h_INCLUDED


#include <lua.h>
#include <pulse/context.h>
#include <stdbool.h>

typedef struct simple_callback_data {
    lua_State* L;
    int thread_ref;
    // PulseAudio's introspection callbacks are used for operations that return single values, as well as operations
    // that return a list. But the callback itself cannot know in which context it is called, so we have to provide
    // that information explicitly.
    bool is_list;
} simple_callback_data;


// Prepares a Lua thread that can call a Lua function as
// callback inside a PulseAudio callback.
//
// Assumes that the Lua function to call is at the top of the stack and copies it to the
// new thread.
//
// The thread will be kept in memory by a unique ref in the registry, so the callback has to
// `luaL_unref` that, to mark it for the garbage collector.
//
// The returned callback data needs to be `free()`d at the end of the callback. `free_lua_callback`
// handles both `free()` and `luaL_unref()`.
simple_callback_data* prepare_lua_callback(lua_State* L);


// Removes the thread reference, to allow the thread to be garbage collected, and `free`s
// the callback data.
void free_lua_callback(simple_callback_data* data);


// Simple implementation of `pa_context_success_cb_t` that calls a provided Lua function.
void success_callback(pa_context* c, int success, void* userdata);

#endif // callback_h_INCLUDED

