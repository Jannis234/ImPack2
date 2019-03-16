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

#ifdef IMPACK_WITH_JP2K

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openjpeg.h>
#include "impack.h"
#include "img.h"

#define BUFSTEP 131072 // 128 KiB

typedef struct {
	uint8_t *buf;
	uint64_t bufsize;
	uint64_t filesize;
	uint64_t pos;
	bool is_input;
	FILE *f;
} impack_opj_stream_state_t;

impack_error_t impack_opj_stream_write_errno;

bool resize_buf_maybe(impack_opj_stream_state_t *state, uint64_t size) {
	
	if (state->bufsize < size) {
		uint64_t newsize = state->bufsize;
		while (newsize < size) {
			newsize += BUFSTEP;
		}
		uint8_t *newbuf = realloc(state->buf, newsize);
		if (newbuf == NULL) {
			return 0;
		}
		state->buf = newbuf;
		memset(state->buf + state->bufsize, 0, newsize - state->bufsize);
		state->bufsize = newsize;
	}
	return true;
	
}

OPJ_SIZE_T impack_opj_stream_read(void *buf, OPJ_SIZE_T count, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	uint64_t toread = count;
	if (toread > state->filesize) {
		toread = state->filesize - state->pos;
	}
	if (toread == 0) {
		return -1;
	}
	memcpy(buf, state->buf + state->pos, toread);
	state->pos += toread;
	return toread;
	
}

OPJ_SIZE_T impack_opj_stream_write(void *buf, OPJ_SIZE_T count, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	if (!resize_buf_maybe(state, state->pos + count)) {
		return -1;
	}
	memcpy(state->buf + state->pos, buf, count);
	state->pos += count;
	if (state->pos > state->filesize) {
		state->filesize = state->pos;
	}
	return count;
	
}

OPJ_OFF_T impack_opj_stream_skip(OPJ_OFF_T count, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	if (!resize_buf_maybe(state, state->pos + count)) {
		return -1;
	}
	state->pos += count;
	if (state->pos > state->filesize) {
		state->filesize = state->pos;
	}
	return count;
	
}

OPJ_BOOL impack_opj_stream_seek(OPJ_OFF_T offset, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	if (state->is_input) {
		if (offset > state->bufsize) {
			return false;
		}
	} else {
		if (!resize_buf_maybe(state, state->pos + offset)) {
			return false;
		}
	}
	state->pos = offset;
	if (state->pos > state->filesize) {
		state->filesize = state->pos;
	}
	return true;
	
}

void impack_opj_stream_free(void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	if (!state->is_input) {
		if (fwrite(state->buf, 1, state->filesize, state->f) != state->filesize) {
			impack_opj_stream_write_errno = ERROR_OUTPUT_IO;
		} else {
			impack_opj_stream_write_errno = ERROR_OK;
		}
	}
	fflush(state->f);
	free(state->buf);
	free(state);
	
}

opj_stream_t *impack_create_opj_stream(FILE *f, bool is_input) {
	
	opj_stream_t *strm = opj_stream_default_create(is_input);
	if (strm == NULL) {
		return NULL;
	}
	impack_opj_stream_state_t *state = malloc(sizeof(impack_opj_stream_state_t));
	if (state == NULL) {
		opj_stream_destroy(strm);
		return NULL;
	}
	state->is_input = is_input;
	state->pos = 0;
	state->filesize = 0;
	state->bufsize = BUFSTEP + 12;
	state->buf = malloc(BUFSTEP + 12);
	state->f = f;
	if (state->buf == NULL) {
		opj_stream_destroy(strm);
		free(state);
		return NULL;
	}
	if (is_input) {
		uint8_t magic[] = IMPACK_MAGIC_JP2K;
		memcpy(state->buf, magic, 12);
		state->filesize = 12;
		size_t bytes_read;
		do {
			bytes_read = fread(state->buf + state->filesize, 1, BUFSTEP, f);
			if (bytes_read == BUFSTEP) {
				uint8_t *newbuf = realloc(state->buf, state->bufsize + BUFSTEP);
				if (newbuf == NULL) {
					free(state->buf);
					free(state);
					opj_stream_destroy(strm);
					return NULL;
				}
				state->buf = newbuf;
				state->bufsize += BUFSTEP;
			}
			state->filesize += bytes_read;
		} while (bytes_read == BUFSTEP);
		opj_stream_set_user_data_length(strm, state->filesize);
	}
	opj_stream_set_user_data(strm, state, impack_opj_stream_free);
	opj_stream_set_read_function(strm, impack_opj_stream_read);
	opj_stream_set_write_function(strm, impack_opj_stream_write);
	opj_stream_set_skip_function(strm, impack_opj_stream_skip);
	opj_stream_set_seek_function(strm, impack_opj_stream_seek);
	return strm;
	
}

#endif
