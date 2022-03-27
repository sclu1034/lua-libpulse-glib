local lgi = require("lgi")
local pulseaudio = require("lgi_pulseaudio")

local loop = lgi.GLib.MainLoop.new()

print(pulseaudio.new)
print(loop:get_context())
-- print(pulseaudio.new(loop:get_context()))
