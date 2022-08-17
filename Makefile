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

.PHONY: all clean doc doc-content doc-styles install uninstall test check rock

all: build doc

build: $(TARGET)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(shell dirname "$@")
	@echo "\033[1;97m$(CC) $< -o $@\033[0m"
	@$(CC) -c $(CCFLAGS) $< -o $@

$(TARGET): $(OBJS)
	@echo "\033[1;97m$(CC) -o $@\033[0m"
	@$(CC) $(LIBFLAG) -o $@ $(OBJS) $(LIBS)

$(BUILD_DIR)/doc/index.html:
	@mkdir -p "$(BUILD_DIR)/doc" "$(BUILD_DIR)/src"
	@echo "\033[1;97mPreprocess sources\033[0m"
	sh tools/process_docs.sh "$(BUILD_DIR)"
	@echo "\033[1;97mGenerate documentation\033[0m"
	ldoc --config=doc/config.ld --dir "$(BUILD_DIR)/doc" --project $(PROJECT) "$(BUILD_DIR)/src"

$(BUILD_DIR)/doc/ldoc.css: doc/ldoc.scss
	@mkdir -p "$(BUILD_DIR)/doc"
	@echo "\033[1;97mGenerate stylesheet\033[0m"
	sass doc/ldoc.scss $(BUILD_DIR)/doc/ldoc.css

doc-styles: $(BUILD_DIR)/doc/ldoc.css

doc-content: $(BUILD_DIR)/doc/index.html

doc: doc-styles doc-content

clean:
	rm -r out/

install: build doc
	@echo "\033[1;97mInstall C library\033[0m"
	install -vDm 644 -t $(INSTALL_LIBDIR) $(TARGET)

	@echo "\033[1;97mInstall documentation\033[0m"
	install -vd $(INSTALL_DOCDIR)
	cp -vr $(BUILD_DIR)/doc/* $(INSTALL_DOCDIR)

uninstall:
	rm $(INSTALL_LIBDIR)/$(PROJECT).so
	rm -r $(INSTALL_DOCDIR)

check:
	@echo "Nothing to do"

test:
	busted --config-file=.busted.lua --lua=$(LUA) $(TEST_ARGS)

rock:
	luarocks --local --lua-version $(LUA_VERSION) make rocks/lua-libpulse-glib-scm-1.rockspec
