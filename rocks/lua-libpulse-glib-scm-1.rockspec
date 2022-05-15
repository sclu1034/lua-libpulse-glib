package = "lua-libpulse-glib"
version = "scm-1"

source = {
    url = "git://github.com/sclu1034/lua-libpulse-glib.git"
}

description = {
    summary = "Lua bindings for PulseAudio's libpulse, using the GLib Main Loop.",
    homepage = "https://github.com/sclu1034/lua-libpulse-glib",
    license = "GPLv3"
}

dependencies = {
    "lua >= 5.1"
}

build = {
    type = "make",
    build_variables = {
        CFLAGS="$(CFLAGS)",
        LIBFLAG="$(LIBFLAG)",
        LUA_LIBDIR="$(LUA_LIBDIR)",
        LUA_BINDIR="$(LUA_BINDIR)",
        LUA_INCDIR="$(LUA_INCDIR)",
        LUA="$(LUA)",
    },
    install_variables = {
        PREFIX="$(PREFIX)",
        INSTALL_BINDIR="$(BINDIR)",
        INSTALL_LIBDIR="$(LIBDIR)",
        INSTALL_LUADIR="$(LUADIR)",
        INSTALL_CONFDIR="$(CONFDIR)",
        INSTALL_DOCDIR="$(DOCDIR)",
    },
    copy_directories = {
        "spec"
    }
}
