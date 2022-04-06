local lgi = require("lgi")
local pulseaudio = require("lua_libpulse_glib")
local ppretty = require("pl.pretty")

local loop = lgi.GLib.MainLoop.new()

local pa = pulseaudio.new()
print("pulseaudio: ", pa)
local ctx = pa:context("test")
print("context: ", ctx)

local function context_state_to_string(state)
    return ({
        "unconnected",
        "connecting",
        "authorizing",
        "setting_name",
        "ready",
        "failed",
        "terminated"
    })[state + 1]
end

print("calling connect")
ctx:connect("localhost", function(_, state)
    print("pulse connection: ", context_state_to_string(state))

    if state == 4 then
        ctx:get_server_info(function(_, info)
            print("server info")
            ppretty.dump(info)
        end)

        ctx:get_sinks(function(_, list)
            print("sinks")
            ppretty.dump(list)
        end)

        ctx:get_sources(function(_, list)
            print("sources")
            ppretty.dump(list)
        end)
    end
end)

loop:run()
