/* This file is part of ImPack2.
 *
 * ImPack2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ImPack2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ImPack2. If not, see <http://www.gnu.org/licenses/>. */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "impack.h"

#define BUFSTEP 131072 // 128 KiB

impack_error_t impack_loadfile(FILE *f, uint8_t **buf, uint64_t *bufsize, uint64_t skip) {
	
	*buf = malloc(BUFSTEP + skip);
	*bufsize = BUFSTEP + skip;
	if (*buf == NULL) {
		return ERROR_MALLOC;
	}
	size_t bytes_read;
	do {
		bytes_read = fread(*buf + *bufsize - BUFSTEP, 1, BUFSTEP, f);
		if (bytes_read == BUFSTEP) {
			uint8_t *newbuf = realloc(*buf, *bufsize + BUFSTEP);
			if (newbuf == NULL) {
				free(*buf);
				return ERROR_MALLOC;
			}
			*buf = newbuf;
			*bufsize += BUFSTEP;
		}
	} while (bytes_read == BUFSTEP);
	*bufsize = *bufsize - BUFSTEP + bytes_read;
	if (!feof(f)) {
		return ERROR_INPUT_IO;
	}
	return ERROR_OK;
	
}
