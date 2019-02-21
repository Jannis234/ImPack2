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

#include "config.h"

#ifdef IMPACK_WITH_TIFF

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "impack.h"
#include <tiffio.h>

#define BUFSTEP 1048576 // 1 MiB

uint8_t *impack_tiff_filebuf;
uint64_t impack_tiff_bufsize;
uint64_t impack_tiff_filesize;
uint64_t impack_tiff_fileoff;
bool impack_tiff_writing;

bool impack_tiff_init_read(FILE *input_file) {
	
	impack_tiff_bufsize = BUFSTEP;
	impack_tiff_filebuf = malloc(impack_tiff_bufsize);
	if (impack_tiff_filebuf == NULL) {
		return false;
	}
	impack_tiff_filesize = 0;
	size_t bytes_read;
	do {
		if (impack_tiff_filesize == impack_tiff_bufsize) {
			uint8_t *newbuf = realloc(impack_tiff_filebuf, impack_tiff_bufsize + BUFSTEP);
			if (newbuf == NULL) {
				free(impack_tiff_filebuf);
				return false;
			}
			impack_tiff_filebuf = newbuf;
			impack_tiff_bufsize += BUFSTEP;
		}
		bytes_read = fread(impack_tiff_filebuf + impack_tiff_filesize, 1, BUFSTEP, input_file);
		impack_tiff_filesize += bytes_read;
	} while (bytes_read == BUFSTEP);
	impack_tiff_fileoff = 0;
	impack_tiff_writing = false;
	return true;
	
}

bool impack_tiff_init_write() {
	
	impack_tiff_filebuf = malloc(BUFSTEP);
	if (impack_tiff_filebuf == NULL) {
		return false;
	}
	memset(impack_tiff_filebuf, 0, BUFSTEP);
	impack_tiff_filesize = 0;
	impack_tiff_bufsize = BUFSTEP;
	impack_tiff_writing = true;
	return true;
	
}

void impack_tiff_finish_read() {
	
	free(impack_tiff_filebuf);
	
}

bool impack_tiff_finish_write(FILE *output_file) {
	
	if (fwrite(impack_tiff_filebuf, 1, impack_tiff_filesize, output_file) != impack_tiff_filesize) {
		free(impack_tiff_filebuf);
		return false;
	}
	free(impack_tiff_filebuf);
	return true;
	
}

tsize_t impack_tiff_read(thandle_t data, tdata_t buf, tsize_t len) {
	
	if (impack_tiff_fileoff >= impack_tiff_filesize) {
		return 0;
	}
	uint64_t res = len;
	if (impack_tiff_fileoff + len >= impack_tiff_filesize) {
		res = impack_tiff_filesize - impack_tiff_fileoff;
	}
	memcpy(buf, impack_tiff_filebuf + impack_tiff_fileoff, res);
	impack_tiff_fileoff += res;
	return res;
	
}

tsize_t impack_tiff_write(thandle_t data, tdata_t buf, tsize_t len) {
	
	if (impack_tiff_fileoff + len >= impack_tiff_bufsize) {
		uint64_t newsize = impack_tiff_bufsize;
		while (impack_tiff_fileoff + len >= newsize) {
			newsize += BUFSTEP;
		}
		uint8_t *newbuf = realloc(impack_tiff_filebuf, newsize);
		if (newbuf == NULL) {
			return -1;
		}
		impack_tiff_filebuf = newbuf;
		memset(impack_tiff_filebuf + impack_tiff_fileoff, 0, newsize - impack_tiff_bufsize);
		impack_tiff_bufsize = newsize;
	}
	memcpy(impack_tiff_filebuf + impack_tiff_fileoff, buf, len);
	impack_tiff_fileoff += len;
	if (impack_tiff_fileoff > impack_tiff_filesize) {
		impack_tiff_filesize = impack_tiff_fileoff;
	}
	return len;
	
}

toff_t impack_tiff_seek(thandle_t data, toff_t offset, int whence) {
	
	uint64_t newoff;
	switch (whence) {
		case SEEK_SET:
			newoff = offset;
			break;
		case SEEK_CUR:
			newoff = impack_tiff_fileoff + offset;
			break;
		case SEEK_END:
			newoff = impack_tiff_filesize + offset;
			break;
		default:
			return -1;
	}
	if (impack_tiff_writing) {
		impack_tiff_fileoff = newoff;
		if (newoff > impack_tiff_filesize) {
			impack_tiff_filesize = newoff;
		}
		return newoff;
	} else {
		if (newoff >= impack_tiff_filesize) {
			return -1;
		}
		impack_tiff_fileoff = newoff;
		return newoff;
	}
	
}

int impack_tiff_close(thandle_t data) {
	
	return 0;
	
}

toff_t impack_tiff_size(thandle_t data) {
	
	return impack_tiff_filesize;
	
}

int impack_tiff_map(thandle_t data, tdata_t *buf, toff_t *len) {
	
	return 0; // Dummy function
	
}

void impack_tiff_unmap(thandle_t data, tdata_t buf, toff_t len) {
	
	return; // Dummy function
	
}

#endif
