#!/bin/sh

set -ex

mkdir -p "${BUILD_DIR:-./build}"

CMAKE_ARGS=""

if [ -n "$CI" ]; then
    CMAKE_ARGS="-DCI=ON"
fi

sudo enable-lua ${LUA_VERSION:-5.1}

make -j $(nproc) -C ${SOURCE_DIR:-.} BUILD_DIR=${BUILD_DIR:-./build} LUA_VERSION=${LUA_VERSION:-5.1} LUA=lua doc

tar -czf doc.tar.gz -C ${BUILD_DIR:-./build}/doc .

curl -T doc.tar.gz -XPOST --user ${DOCS_CREDENTIALS} https://${DOCS_HOST}/publish/lua-libpulse-glib

sudo disable-lua ${LUA_VERSION:-5.1}

