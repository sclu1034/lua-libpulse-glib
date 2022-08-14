#!/bin/sh

set -ex

sudo enable-lua ${LUA_VERSION:-5.1}
eval "$(luarocks --lua-version ${LUA_VERSION:-5.1} path)"

sudo apt-get update
sudo apt-get install -y --no-install-recommends gobject-introspection libgirepository1.0-dev libpulse-dev

luarocks --lua-version ${LUA_VERSION:-5.1} install lgi
luarocks --lua-version ${LUA_VERSION:-5.1} install ldoc
luarocks --lua-version ${LUA_VERSION:-5.1} install lua-discount

luarocks --lua-version ${LUA_VERSION:-5.1} make rocks/lgi-libpulse-glib-scm-1.rockspec

sudo disable-lua ${LUA_VERSION:-5.1}
