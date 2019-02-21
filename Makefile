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

#IMPACK_VERSION = 1.0
IMPACK_VERSION = git-$(shell git rev-parse --short HEAD)

include config_system.mak

CFLAGS += -Wall -std=c99 -Isrc/include

CLI_SRC = src/cli/main.c \
	src/cli/argparse.c \
	src/cli/help.c \
	src/cli/error.c \
	src/cli/readpass.c
LIB_SRC = src/lib/encode.c \
	src/lib/decode.c \
	src/lib/filename.c \
	src/lib/byteswap.c \
	src/lib/crc.c \
	src/lib/write_img.c \
	src/lib/write_img_png.c \
	src/lib/write_img_webp.c \
	src/lib/write_img_tiff.c \
	src/lib/read_img.c \
	src/lib/read_img_png.c \
	src/lib/read_img_webp.c \
	src/lib/libtiff_io.c \
	src/lib/secure_erase.c \
	src/lib/random.c \
	src/lib/crypt.c \
	src/lib/compress.c \
	src/lib/compress_zlib.c \
	src/lib/compress_zstd.c \
	src/lib/compress_lzma.c \
	src/lib/select.c

.PHONY: all depend clean cli man man-cli install install-cli install-man install-man-cli uninstall

all: cli man

depend: depend.mak

cli: impack$(EXEEXT)

man: man-cli

man-cli: impack.1

install: install-cli install-man

install-man: install-man-cli

install-cli: cli
	$(INSTALL) impack$(EXEEXT) $(BINDIR)

install-man-cli: man-cli
	$(INSTALL) impack.1 $(MANDIR)/man1

uninstall:
	rm -f $(BINDIR)/impack$(EXEEXT)
	rm -f $(MANDIR)/man1/impack.1

clean:
	rm -f $(CLI_SRC:.c=.o)
	rm -f $(CLI_SRC:.c=.d)
	rm -f $(LIB_SRC:.c=.o)
	rm -f $(LIB_SRC:.c=.d)
	rm -f impack$(EXEEXT) libimpack.a
	rm -f impack.1
	rm -f depend.mak
	rm -f src/include/config_generated.h

impack$(EXEEXT): libimpack.a $(CLI_SRC:.c=.o)
	$(CCLD) -o impack$(EXEEXT) $(CLI_SRC:.c=.o) libimpack.a $(LIBS)

impack.1: impack$(EXEEXT)
	help2man -N ./impack$(EXEEXT) -o impack.1

libimpack.a: $(LIB_SRC:.c=.o)
	$(AR) cr libimpack.a $(LIB_SRC:.c=.o)
	$(RANLIB) libimpack.a

depend.mak: $(CLI_SRC:.c=.d) $(LIB_SRC:.c=.d)
	cat $(CLI_SRC:.c=.d) > depend.mak
	cat $(LIB_SRC:.c=.d) >> depend.mak

src/include/config_generated.h: config_build.mak src/gen_config.sh
	sh src/gen_config.sh $(IMPACK_VERSION) $(WITH_NETTLE) $(WITH_LIBPNG) $(WITH_LIBWEBP) $(WITH_LIBTIFF) $(WITH_ZLIB) $(WITH_ZSTD) $(WITH_LZMA) > src/include/config_generated.h

%.d: %.c config_build.mak config_system.mak src/include/config_generated.h
	$(CC) $(CFLAGS) -M -MT $(<:.c=.o) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

include depend.mak
