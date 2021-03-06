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

#ifdef IMPACK_WITH_ZLIB

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "impack_internal.h"

bool impack_compress_init_zlib(impack_compress_state_t *state) {
	
	z_stream *strm = malloc(sizeof(z_stream));
	if (strm == NULL) {
		return false;
	}
	state->output_buf = NULL;
	state->input_buf = malloc(state->bufsize);
	if (state->input_buf == NULL) {
		goto cleanup;
	}
	state->output_buf = malloc(state->bufsize);
	if (state->output_buf == NULL) {
		goto cleanup;
	}
	strm->next_in = state->input_buf;
	strm->next_out = state->output_buf;
	strm->avail_in = 0;
	strm->avail_out = state->bufsize;
	strm->zalloc = NULL;
	strm->zfree = NULL;
	strm->opaque = NULL;
	
	int res;
	if (state->is_compress) {
		if (state->level == 0) {
			state->level = Z_DEFAULT_COMPRESSION;
		}
		res = deflateInit(strm, state->level);
	} else {
		res = inflateInit2(strm, 47); // 15 + 32 enables header auto-detection
	}
	state->lib_object = strm;
	if (res == Z_OK) {
		return true;
	} else {
		goto cleanup;
	}
	
cleanup:
	free(strm);
	if (state->input_buf != NULL) {
		free(state->input_buf);
	}
	if (state->output_buf != NULL) {
		free(state->output_buf);
	}
	return false;
	
}

void impack_compress_free_zlib(impack_compress_state_t *state) {
	
	z_stream *strm = (z_stream*) state->lib_object;
	if (state->is_compress) {
		deflateEnd(strm);
	} else {
		inflateEnd(strm);
	}
	free(state->input_buf);
	free(state->output_buf);
	free(strm);
	
}

impack_compression_result_t impack_compress_read_zlib(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	z_stream *strm = (z_stream*) state->lib_object;
	int res;
	if (state->is_compress) {
		res = deflate(strm, Z_NO_FLUSH);
	} else {
		res = inflate(strm, Z_NO_FLUSH);
	}
	
	if (res != Z_OK && res != Z_STREAM_END && res != Z_BUF_ERROR) {
		return COMPRESSION_RES_ERROR;
	}
	if (strm->avail_out == 0 || res == Z_STREAM_END) {
		memcpy(buf, state->output_buf, state->bufsize - strm->avail_out);
		*lenout = state->bufsize - strm->avail_out;
		strm->next_out = state->output_buf;
		strm->avail_out = state->bufsize;
		if (res == Z_STREAM_END) {
			return COMPRESSION_RES_FINAL;
		} else {
			return COMPRESSION_RES_OK;
		}
	} else {
		return COMPRESSION_RES_AGAIN;
	}
	
}

void impack_compress_write_zlib(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	z_stream *strm = (z_stream*) state->lib_object;
	memcpy(state->input_buf, buf, len);
	strm->next_in = state->input_buf;
	strm->avail_in = len;
	
}

impack_compression_result_t impack_compress_flush_zlib(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	z_stream *strm = (z_stream*) state->lib_object;
	int res = deflate(strm, Z_FINISH);
	if (res == Z_STREAM_END) {
		memcpy(buf, state->output_buf, state->bufsize - strm->avail_out);
		*lenout = state->bufsize - strm->avail_out;
		return COMPRESSION_RES_FINAL;
	} else {
		memcpy(buf, state->output_buf, state->bufsize);
		strm->next_out = state->output_buf;
		strm->avail_out = state->bufsize;
		return COMPRESSION_RES_AGAIN;
	}
	
}

bool impack_compress_level_valid_zlib(int32_t level) {
	
	return (level <= Z_BEST_COMPRESSION);
	
}

#endif
