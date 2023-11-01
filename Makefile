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

LUA_CPATH = $(shell echo "$${PWD}/$(BUILD_DIR)/?.so;$${LUA_CPATH}")

ifdef CI
CHECK_ARGS ?= --formatter TAP
TEST_ARGS ?= --output=TAP

CCFLAGS += -Werror
endif

bold := $(shell tput bold)
orange := $(shell tput setaf 7)
title := $(bold)$(orange)
reset := $(shell tput sgr0)

.PHONY: all clean doc doc-content doc-styles install uninstall test check rock

all: build doc

build: $(TARGET)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(shell dirname "$@")
	@echo "$(title)$(CC) $< -o $@$(reset)"
	@$(CC) -c $(CCFLAGS) $< -o $@

$(TARGET): $(OBJS)
	@echo "$(title)$(CC) -o $@$(reset)"
	@$(CC) $(LIBFLAG) -o $@ $(OBJS) $(LIBS)

$(BUILD_DIR)/doc/index.html:
	@mkdir -p "$(BUILD_DIR)/doc" "$(BUILD_DIR)/src"
	@echo "$(title)Preprocess sources$(reset)"
	sh tools/process_docs.sh "$(BUILD_DIR)"
	@echo "$(title)Generate documentation$(reset)"
	ldoc --config=doc/config.ld --dir "$(BUILD_DIR)/doc" --project $(PROJECT) "$(BUILD_DIR)/src"

$(BUILD_DIR)/doc/ldoc.css: doc/ldoc.scss
	@mkdir -p "$(BUILD_DIR)/doc"
	@echo "$(title)Generate stylesheet$(reset)"
	sass doc/ldoc.scss $(BUILD_DIR)/doc/ldoc.css

doc-styles: $(BUILD_DIR)/doc/ldoc.css

doc-content: $(BUILD_DIR)/doc/index.html

doc: doc-styles doc-content

clean:
	rm -r out/

install: build doc
	@echo "$(title)Install C library\033[0m"
	install -vDm 644 -t $(INSTALL_LIBDIR) $(TARGET)

	@echo "$(title)Install documentation\033[0m"
	install -vd $(INSTALL_DOCDIR)
	cp -vr $(BUILD_DIR)/doc/* $(INSTALL_DOCDIR)

uninstall:
	rm $(INSTALL_LIBDIR)/$(PROJECT).so
	rm -r $(INSTALL_DOCDIR)

check:
	@echo "Nothing to do"

spec: build
	busted --config-file=.busted.lua --lua=$(LUA) $(TEST_ARGS)

test: build
	$(LUA) test.lua

rock:
	luarocks --local --lua-version $(LUA_VERSION) make rocks/lua-libpulse-glib-scm-1.rockspec
