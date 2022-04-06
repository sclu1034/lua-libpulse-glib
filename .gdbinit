symbol-file -readnow lua.debug
# source ~/.config/gdb/lua-gdb.py
set directories src/lua_libpulse_glib:/home/gerlui/build/lua-5.1.5/src
# set directories src/lua_libpulse_glib
set args test.lua
