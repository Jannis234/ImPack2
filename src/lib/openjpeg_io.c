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

#define BUFSTEP 16384 // 16 KiB

typedef struct {
	uint8_t *buf;
	uint64_t bufsize;
	uint64_t filesize;
	uint64_t pos;
	bool is_input;
	FILE *f;
} impack_opj_stream_state_t;

impack_error_t impack_opj_stream_write_errno;

OPJ_SIZE_T impack_opj_stream_read(void *buf, OPJ_SIZE_T count, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	uint64_t toread = count;
	if (toread > state->filesize) {
		toread = state->filesize - state->pos;
	}
	memcpy(buf, state->buf + state->pos, toread);
	state->pos += toread;
	return toread;
	
}

OPJ_SIZE_T impack_opj_stream_write(void *buf, OPJ_SIZE_T count, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	if (state->bufsize < state->pos + count) {
		uint64_t newsize = state->bufsize;
		while (newsize < state->pos + count) {
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
	memcpy(state->buf + state->pos, buf, count);
	state->pos += count;
	if (state->pos > state->filesize) {
		state->filesize = state->pos;
	}
	return count;
	
}

OPJ_OFF_T impack_opj_stream_skip(OPJ_OFF_T count, void *userdata) {
	
	impack_opj_stream_state_t *state = (impack_opj_stream_state_t*) userdata;
	if (state->bufsize < state->pos + count) {
		uint64_t newsize = state->bufsize;
		while (newsize < state->pos + count) {
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
		if (state->bufsize < state->pos + offset) {
			uint64_t newsize = state->bufsize;
			while (newsize < state->pos + offset) {
				newsize += BUFSTEP;
			}
			uint8_t *newbuf = realloc(state->buf, newsize);
			if (newbuf == NULL) {
				return false;
			}
			state->buf = newbuf;
			memset(state->buf + state->bufsize, 0, newsize - state->bufsize);
			state->bufsize = newsize;
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
	state->bufsize = BUFSTEP;
	state->buf = malloc(BUFSTEP);
	state->f = f;
	if (state->buf == NULL) {
		opj_stream_destroy(strm);
		free(state);
		return NULL;
	}
	if (is_input) {
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
	}
	opj_stream_set_user_data(strm, state, impack_opj_stream_free);
	opj_stream_set_read_function(strm, impack_opj_stream_read);
	opj_stream_set_write_function(strm, impack_opj_stream_write);
	opj_stream_set_skip_function(strm, impack_opj_stream_skip);
	opj_stream_set_seek_function(strm, impack_opj_stream_seek);
	return strm;
	
}

#endif
