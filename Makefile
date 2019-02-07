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
include config_system.mak

CFLAGS += -Wall -std=c99 -Isrc/include

CLI_SRC = src/cli/main.c
LIB_SRC = src/lib/encode.c

.PHONY: all depend clean

all: impack

depend: depend.mak

clean:
	rm -f $(CLI_SRC:.c=.o)
	rm -f $(CLI_SRC:.c=.d)
	rm -f $(LIB_SRC:.c=.o)
	rm -f $(LIB_SRC:.c=.d)
	rm -f impack libimpack.a
	rm -f depend.mak

impack: libimpack.a $(CLI_SRC:.c=.o)
	$(CCLD) -o impack $(CLI_SRC:.c=.o) libimpack.a

libimpack.a: $(LIB_SRC:.c=.o)
	$(AR) cr libimpack.a $(LIB_SRC:.c=.o)
	$(RANLIB) libimpack.a

depend.mak: $(CLI_SRC:.c=.d) $(LIB_SRC:.c=.d)
	cat $(CLI_SRC:.c=.d) $(LIB_SRC:.c=.d) > depend.mak

%.d: %.c
	$(CC) $(CFLAGS) -M -MT $(<:.c=.o) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

include depend.mak

