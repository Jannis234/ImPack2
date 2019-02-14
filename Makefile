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
	src/cli/error.c
LIB_SRC = src/lib/encode.c \
	src/lib/decode.c \
	src/lib/filename.c \
	src/lib/byteswap.c \
	src/lib/crc.c \
	src/lib/write_img.c \
	src/lib/write_img_png.c \
	src/lib/read_img.c \
	src/lib/read_img_png.c \
	src/lib/secure_erase.c \
	src/lib/random.c \
	src/lib/crypt.c \
	src/lib/compress.c \
	src/lib/compress_zlib.c

.PHONY: all depend clean cli

all: cli

depend: depend.mak

clean:
	rm -f $(CLI_SRC:.c=.o)
	rm -f $(CLI_SRC:.c=.d)
	rm -f $(LIB_SRC:.c=.o)
	rm -f $(LIB_SRC:.c=.d)
	rm -f impack$(EXEEXT) libimpack.a
	rm -f depend.mak
	rm -f src/include/config_generated.h

cli: impack$(EXEEXT)

impack$(EXEEXT): libimpack.a $(CLI_SRC:.c=.o)
	$(CCLD) -o impack$(EXEEXT) $(CLI_SRC:.c=.o) libimpack.a $(LIBS)

libimpack.a: $(LIB_SRC:.c=.o)
	$(AR) cr libimpack.a $(LIB_SRC:.c=.o)
	$(RANLIB) libimpack.a

depend.mak: $(CLI_SRC:.c=.d) $(LIB_SRC:.c=.d)
	cat $(CLI_SRC:.c=.d) > depend.mak
	cat $(LIB_SRC:.c=.d) >> depend.mak

src/include/config_generated.h: config_build.mak src/gen_config.sh
	sh src/gen_config.sh $(IMPACK_VERSION) $(WITH_NETTLE) $(WITH_LIBPNG) $(WITH_ZLIB) > src/include/config_generated.h

%.d: %.c config_build.mak config_system.mak src/include/config_generated.h
	$(CC) $(CFLAGS) -M -MT $(<:.c=.o) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

include depend.mak
