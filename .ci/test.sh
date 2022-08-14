#!/bin/sh

set -ex

mkdir -p "${BUILD_DIR:-./build}"

CMAKE_ARGS=""

if [ -n "$CI" ]; then
    CMAKE_ARGS="-DCI=ON"
fi

sudo enable-lua ${LUA_VERSION:-5.1}
eval "$(luarocks --lua-version ${LUA_VERSION:-5.1} path)"

sudo apt-get update
sudo apt-get install -y --no-install-recommends gobject-introspection libgirepository1.0-dev libpulse-dev

luarocks --lua-version ${LUA_VERSION:-5.1} install busted
luarocks --lua-version ${LUA_VERSION:-5.1} install lgi
luarocks --lua-version ${LUA_VERSION:-5.1} install ldoc
luarocks --lua-version ${LUA_VERSION:-5.1} install lua-discount

cmake -S "${SOURCE_DIR:-./}" -B "${BUILD_DIR:-./build}" ${CMAKE_ARGS}
make -j $(nproc) -C "${BUILD_DIR:-./build}"

busted \
    --lua /usr/bin/lua \
    -C "${SOURCE_DIR:-./}" \
    "--config-file=${SOURCE_DIR:-./}/.busted.lua" \
    "--cpath=${BUILD_DIR:-./build}/?.so" \
    --output=TAP

sudo disable-lua ${LUA_VERSION:-5.1}
