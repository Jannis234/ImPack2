# This file is part of ImPack2.
#
# ImPack2 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ImPack2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ImPack2. If not, see <http://www.gnu.org/licenses/>.

include config_build.mak

# Host-specific options (programs, paths, etc.)

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

CC = gcc
CCLD = $(CC)
AR = ar
RANLIB = ranlib
PKG_CONFIG = pkg-config
HELP2MAN = help2man
INSTALL = install

CFLAGS = -O2 -pipe -ggdb
LIBS =
EXEEXT =

ifeq ($(WITH_NETTLE), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags nettle)
LIBS += $(shell $(PKG_CONFIG) --libs nettle)
endif

ifeq ($(WITH_LIBPNG), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libpng)
LIBS += $(shell $(PKG_CONFIG) --libs libpng)
endif

ifeq ($(WITH_LIBWEBP), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libwebp)
LIBS += $(shell $(PKG_CONFIG) --libs libwebp)
endif

ifeq ($(WITH_ZLIB), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags zlib)
LIBS += $(shell $(PKG_CONFIG) --libs zlib)
endif

ifeq ($(WITH_ZSTD), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libzstd)
LIBS += $(shell $(PKG_CONFIG) --libs libzstd)
endif

ifeq ($(WITH_LZMA), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags liblzma)
LIBS += $(shell $(PKG_CONFIG) --libs liblzma)
endif
