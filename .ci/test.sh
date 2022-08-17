#!/bin/sh

set -ex

mkdir -p "${BUILD_DIR:-./build}"

CMAKE_ARGS=""

if [ -n "$CI" ]; then
    CMAKE_ARGS="-DCI=ON"
fi

sudo enable-lua ${LUA_VERSION:-5.1}
eval "$(luarocks --lua-version ${LUA_VERSION:-5.1} path)"

luarocks --lua-version ${LUA_VERSION:-5.1} install busted
luarocks --lua-version ${LUA_VERSION:-5.1} install lgi

make -j $(nproc) -C ${SOURCE_DIR:-.} BUILD_DIR=${BUILD_DIR:-./build} LUA_VERSION=${LUA_VERSION:-5.1} LUA=lua build

sudo disable-lua ${LUA_VERSION:-5.1}
