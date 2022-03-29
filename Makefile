PROJECT = lua_libpulse_glib
PREFIX ?= /usr/local
BUILD_DIR = out

LUA_VERSION ?= 5.1
LUA ?= $(shell command -v lua$(LUA_VERSION))
LUA_BINDIR ?= /usr/bin
LUA_LIBDIR ?= /usr/lib/x86_64-linux-gnu/lua/$(LUA_VERSION)
LUA_INCDIR ?= /usr/include/lua$(LUA_VERSION)

ifdef LIBDIR
INSTALL_LIBDIR ?= $(LIBDIR)
else
INSTALL_LIBDIR ?= $(PREFIX)/lib/lua/$(LUA_VERSION)
endif

ifdef LUADIR
INSTALL_LUADIR ?= $(LUADIR)
else
INSTALL_LUADIR ?= $(PREFIX)/share/lua/$(LUA_VERSION)
endif

ifdef DOCDIR
INSTALL_DOCDIR ?= $(DOCDIR)
else
INSTALL_DOCDIR ?= $(PREFIX)/share/doc/$(PROJECT)
endif

CC = gcc
PKG_CONFIG ?= $(shell command -v pkg-config)

PKGS = libpulse-mainloop-glib glib-2.0 gobject-2.0 gobject-introspection-1.0 lua$(LUA_VERSION)

CFLAGS ?= -fPIC
LIBFLAG ?= -shared

CCFLAGS ?= $(CFLAGS)
CCFLAGS += -Wall -g -rdynamic $(shell $(PKG_CONFIG) --cflags $(PKGS)) -I$(LUA_INCDIR) -I"./"

LIBS = -L$(shell dirname "$(shell $(CC) -print-libgcc-file-name)") -L"$(LUA_LIBDIR)" -L"./"
LIBS += $(shell $(PKG_CONFIG) --libs $(PKGS))
OBJS = $(shell find src -type f -iname '*.c' | sed 's/\(.*\)\.c$$/$(BUILD_DIR)\/\1\.o/')

TARGET = $(BUILD_DIR)/$(PROJECT).so

ifdef CI
CHECK_ARGS ?= --formatter TAP
TEST_ARGS ?= --output=TAP

CCFLAGS += -Werror
endif

.PHONY: clean doc doc-content doc-styles install test check rock

build: $(TARGET)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(shell dirname "$@")
	$(CC) -c $(CCFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LIBFLAG) -o $@ $(OBJS) $(LIBS)

doc-styles:
	@printf "\e[1;97mGenerate stylesheet\e[0m\n"
	sass doc/ldoc.scss $(BUILD_DIR)/doc/ldoc.css

doc-content:
	@mkdir -p "$(BUILD_DIR)/doc" "$(BUILD_DIR)/src"
	@printf "\e[1;97mPreprocess sources\e[0m\n"
	sh tools/process_docs.sh "$(BUILD_DIR)"
	@printf "\e[1;97mGenerate documentation\e[0m\n"
	ldoc --config=doc/config.ld --dir "$(BUILD_DIR)/doc" --project $(PROJECT) "$(BUILD_DIR)/src"

doc: doc-content doc-styles
ifdef CI
	touch "$(BUILD_DIR)/doc/.nojekyll"
endif

clean:
	rm -r out/

install: build doc
	@printf "\e[1;97mInstall C library\e[0m\n"
	xargs install -vDm 644 -t $(INSTALL_LIBDIR)/$(PROJECT) $(TARGET)

	@printf "\e[1;97mInstall documentation\e[0m\n"
	install -vd $(INSTALL_DOCDIR)
	cp -vr $(BUILD_DIR)/doc/* $(INSTALL_DOCDIR)

check:
	@echo "Nothing to do"

test:
	busted --config-file=.busted.lua --lua=$(LUA) $(TEST_ARGS)

rock:
	luarocks --local --lua-version $(LUA_VERSION) make rocks/lua-libpulse-glib-scm-1.rockspec
