NAVISERVER ?= /usr/local/ns
PKG_CONFIG ?= pkg-config
MOD = libtmfa.so
MODOBJS = library.o

TMFA_VERSION := $(shell sed -n 's/^project(tmfa VERSION \([^ ]*\).*/\1/p' CMakeLists.txt)
CFLAGS += -std=c11 -DUSE_NAVISERVER -DPROJECT_VERSION='"$(TMFA_VERSION)"'
CFLAGS += $(shell $(PKG_CONFIG) --cflags libcrypto)
MODLIBS += $(shell $(PKG_CONFIG) --libs libcrypto)

include $(NAVISERVER)/include/Makefile.module
