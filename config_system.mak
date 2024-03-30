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
DESKTOPDIR = $(PREFIX)/share/applications
ICONDIR = $(PREFIX)/share/icons

BUILD = 
CC = $(BUILD)gcc
CCLD = $(CC)
AR = $(BUILD)ar
RANLIB = $(BUILD)ranlib
PKG_CONFIG = $(BUILD)pkg-config
HELP2MAN = help2man
INSTALL = install

CCFLAGS = -O2 -pipe -ggdb
LDFLAGS = 
LIBS = 
EXEEXT = 

ifeq ($(WITH_GTK), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags gtk+-3.0)
GTK_LIBS = $(shell $(PKG_CONFIG) --libs gtk+-3.0)
endif

ifeq ($(WITH_NETTLE), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags nettle)
LIBS += $(shell $(PKG_CONFIG) --libs nettle)
endif

ifeq ($(WITH_ARGON2), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libargon2)
LIBS += $(shell $(PKG_CONFIG) --libs libargon2)
endif

ifeq ($(WITH_CHARLS), 1)
LIBS += -lcharls
endif

ifeq ($(WITH_FLIF), 1)
LIBS += -lflif
endif

ifeq ($(WITH_JXRLIB), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libjxr)
LIBS += $(shell $(PKG_CONFIG) --libs libjxr)
endif

ifeq ($(WITH_LIBAVIF), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libavif)
LIBS += $(shell $(PKG_CONFIG) --libs libavif)
endif

ifeq ($(WITH_LIBHEIF), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libheif)
LIBS += $(shell $(PKG_CONFIG) --libs libheif)
endif

ifeq ($(WITH_LIBJXL), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libjxl)
LIBS += $(shell $(PKG_CONFIG) --libs libjxl)
endif

ifeq ($(WITH_LIBNSBMP), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libnsbmp)
LIBS += $(shell $(PKG_CONFIG) --libs libnsbmp)
endif

ifeq ($(WITH_LIBPNG), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libpng)
LIBS += $(shell $(PKG_CONFIG) --libs libpng)
endif

ifeq ($(WITH_LIBTIFF), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libtiff-4)
LIBS += $(shell $(PKG_CONFIG) --libs libtiff-4)
endif

ifeq ($(WITH_LIBWEBP), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libwebp)
LIBS += $(shell $(PKG_CONFIG) --libs libwebp)
endif

ifeq ($(WITH_OPENJPEG2), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libopenjp2)
LIBS += $(shell $(PKG_CONFIG) --libs libopenjp2)
endif

ifeq ($(WITH_BROTLI), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libbrotlidec) $(shell $(PKG_CONFIG) --cflags libbrotlienc)
LIBS += $(shell $(PKG_CONFIG) --libs libbrotlidec) $(shell $(PKG_CONFIG) --libs libbrotlienc)
endif

ifeq ($(WITH_BZIP2), 1)
LIBS += -lbz2
endif

ifeq ($(WITH_LZMA), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags liblzma)
LIBS += $(shell $(PKG_CONFIG) --libs liblzma)
endif

ifeq ($(WITH_ZLIB), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags zlib)
LIBS += $(shell $(PKG_CONFIG) --libs zlib)
endif

ifeq ($(WITH_ZSTD), 1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libzstd)
LIBS += $(shell $(PKG_CONFIG) --libs libzstd)
endif
