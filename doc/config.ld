project = 'lua-libpulse-glib'
title = 'lua-libpulse-glib'
description = "Lua bindings for PulseAudio's libpulse, using the GLib Main Loop."

full_description = [[
While libpulse provides different event loop implementations, these bindings focus on the GLib comaptibility only.
Therefore, all asynchronous functions need to be executed inside a running GLib Main Loop.
For now, this loop has to run on GLib's default main context. Custom loop contexts are currently not supported.

The majority of the API is callback-based asynchronous. Callbacks always receive any potential errors as their
first parameter, usually just the error message as string, or `nil` when there was no error. Additional parameters
may either be just a boolean, for operations that don't return data, or the actual queried data.

All numeric indices (such as sink or source indices) are adjusted to be 1-based in typical Lua fashion.
This means that when comparing the output of calls like @{Context:get_sinks} to the output of tools like `pactl`,
indices will be off by one.
]]

template = true

format = 'discount'

pretty = 'lua'
-- prettify_files = 'show'
backtick_references = false
wrap = true
no_space_before_args = true

custom_tags = {
    {
        "async",
        hidden = true,
    },
    {
        "repository",
        hidden = true,
    },
    {
        "package",
        hidden = true,
    }
}
