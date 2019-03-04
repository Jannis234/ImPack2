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

include config_system.mak

CFLAGS += $(CCFLAGS) -Wall -std=c99 -Isrc/include

CLI_SRC = src/cli/main.c \
	src/cli/argparse.c \
	src/cli/help.c \
	src/cli/error.c \
	src/cli/readpass.c
GUI_SRC = src/gui/main.c \
	src/gui/gresources_generated.c \
	src/gui/window_main.c \
	src/gui/work_thread.c
GUI_RES = src/gui/res/gresources.xml \
	src/gui/res/window_main.ui \
	src/gui/res/icon.png
TEST_SRC = src/test/main.c \
	src/test/build_info.c
LIB_SRC = src/lib/encode.c \
	src/lib/decode.c \
	src/lib/filename.c \
	src/lib/byteswap.c \
	src/lib/crc.c \
	src/lib/write_img.c \
	src/lib/write_img_png.c \
	src/lib/write_img_webp.c \
	src/lib/write_img_tiff.c \
	src/lib/write_img_bmp.c \
	src/lib/write_img_jp2k.c \
	src/lib/read_img.c \
	src/lib/read_img_png.c \
	src/lib/read_img_webp.c \
	src/lib/read_img_tiff.c \
	src/lib/read_img_bmp.c \
	src/lib/read_img_jp2k.c \
	src/lib/libtiff_io.c \
	src/lib/openjpeg_io.c \
	src/lib/secure_erase.c \
	src/lib/random.c \
	src/lib/crypt.c \
	src/lib/compress.c \
	src/lib/compress_zlib.c \
	src/lib/compress_zstd.c \
	src/lib/compress_lzma.c \
	src/lib/compress_bzip2.c \
	src/lib/compress_brotli.c \
	src/lib/select.c

.PHONY: all depend clean check cli man gui install install-cli install-man install-gui uninstall

ifeq ($(WITH_GTK), 1)
all: cli man gui
else
all: cli man
endif

depend: depend.mak

cli: impack$(EXEEXT)

man: impack.1

gui: impack-gtk$(EXEEXT)

ifeq ($(WITH_GTK), 1)
install: install-cli install-man install-gui
else
install: install-cli install-man
endif

install-cli: cli
	mkdir -p $(BINDIR)
	$(INSTALL) impack$(EXEEXT) $(BINDIR)

install-man: man
	mkdir -p $(MANDIR)
	$(INSTALL) -m 644 impack.1 $(MANDIR)/man1

install-gui: gui
	mkdir -p $(BINDIR)
	$(INSTALL) impack-gtk$(EXEEXT) $(BINDIR)
	mkdir -p $(DESKTOPDIR)
	$(INSTALL) -m 644 src/gui/res/impack.desktop $(DESKTOPDIR)
	mkdir -p $(ICONDIR)/hicolor/256x256
	$(INSTALL) -m 644 src/gui/res/icon.png $(ICONDIR)/hicolor/256x256/impack.png

uninstall:
	rm -f $(BINDIR)/impack$(EXEEXT)
	rm -f $(MANDIR)/man1/impack.1

check: testsuite$(EXEEXT)
	./testsuite$(EXEEXT)

clean:
	rm -f $(CLI_SRC:.c=.o) $(CLI_SRC:.c=.d)
	rm -f $(GUI_SRC:.c=.o) $(GUI_SRC:.c=.d)
	rm -f $(TEST_SRC:.c=.o) $(TEST_SRC:.c=.d)
	rm -f $(LIB_SRC:.c=.o) $(LIB_SRC:.c=.d)
	rm -f impack$(EXEEXT) impack-gtk$(EXEEXT) testsuite$(EXEEXT) libimpack.a
	rm -f impack.1
	rm -f depend.mak
	rm -f src/include/config_generated.h
	rm -f src/gui/gresources_generated.c
	rm -f testout_encode.tmp testout_decode.tmp

impack$(EXEEXT): libimpack.a $(CLI_SRC:.c=.o)
	$(CCLD) -o impack$(EXEEXT) $(CLI_SRC:.c=.o) libimpack.a $(LIBS)

impack-gtk$(EXEEXT): libimpack.a $(GUI_SRC:.c=.o)
	$(CCLD) -o impack-gtk$(EXEEXT) $(GUI_SRC:.c=.o) libimpack.a $(LIBS) $(GTK_LIBS)

testsuite$(EXEEXT): libimpack.a $(TEST_SRC:.c=.o)
	$(CCLD) -o testsuite$(EXEEXT) $(TEST_SRC:.c=.o) libimpack.a $(LIBS)

impack.1: impack$(EXEEXT)
	help2man -N ./impack$(EXEEXT) -o impack.1

libimpack.a: $(LIB_SRC:.c=.o)
	$(AR) cr libimpack.a $(LIB_SRC:.c=.o)
	$(RANLIB) libimpack.a

depend.mak: $(CLI_SRC:.c=.d) $(GUI_SRC:.c=.d) $(LIB_SRC:.c=.d)
	cat $(CLI_SRC:.c=.d) > depend.mak
ifeq ($(WITH_GTK), 1)
	cat $(GUI_SRC:.c=.d) >> depend.mak
endif
	cat $(LIB_SRC:.c=.d) >> depend.mak

src/include/config_generated.h: config_build.mak src/gen_config.sh
	sh src/gen_config.sh $(IMPACK_VERSION) $(WITH_NETTLE) $(WITH_LIBPNG) $(WITH_LIBWEBP) $(WITH_LIBTIFF) $(WITH_LIBNSBMP) $(WITH_OPENJPEG2) $(WITH_ZLIB) $(WITH_ZSTD) $(WITH_LZMA) $(WITH_BZIP2) $(WITH_BROTLI) > src/include/config_generated.h

src/gui/gresources_generated.c: $(GUI_RES)
	glib-compile-resources src/gui/res/gresources.xml --generate-source --sourcedir=src/gui/res --target=src/gui/gresources_generated.c

%.d: %.c config_build.mak config_system.mak src/include/config_generated.h
	$(CC) $(CFLAGS) -M -MT $(<:.c=.o) -o $@ $<

%.o: %.c #config_build.mak config_system.mak src/include/config_generated.h
	$(CC) $(CFLAGS) -c -o $@ $<

include depend.mak
