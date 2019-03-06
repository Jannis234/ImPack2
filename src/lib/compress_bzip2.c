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

#ifdef IMPACK_WITH_BZIP2

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "impack_internal.h"
#include <bzlib.h>

bool impack_compress_init_bzip2(impack_compress_state_t *state) {
	
	bz_stream *strm = malloc(sizeof(bz_stream));
	if (strm == NULL) {
		return false;
	}
	state->input_buf = malloc(state->bufsize);
	if (state->input_buf == NULL) {
		free(strm);
		return false;
	}
	state->output_buf = malloc(state->bufsize);
	if (state->output_buf == NULL) {
		free(state->input_buf);
		free(strm);
		return false;
	}
	strm->next_in = (char*) state->input_buf;
	strm->next_out = (char*) state->output_buf;
	strm->avail_in = 0;
	strm->avail_out = state->bufsize;
	strm->bzalloc = NULL;
	strm->bzfree = NULL;
	strm->opaque = NULL;
	
	int res;
	if (state->is_compress) {
		if (state->level == 0) {
			state->level = 9; // Default from bzip2 manpage
		}
		res = BZ2_bzCompressInit(strm, state->level, 0, 0);
	} else {
		res = BZ2_bzDecompressInit(strm, 0, 0);
	}
	state->lib_object = strm;
	if (res == BZ_OK) {
		return true;
	} else {
		free(strm);
		free(state->input_buf);
		free(state->output_buf);
		return false;
	}
	
}

void impack_compress_free_bzip2(impack_compress_state_t *state) {
	
	bz_stream *strm = (bz_stream*) state->lib_object;
	if (state->is_compress) {
		BZ2_bzCompressEnd(strm);
	} else {
		BZ2_bzDecompressEnd(strm);
	}
	free(state->input_buf);
	free(state->output_buf);
	free(strm);
	
}

impack_compression_result_t impack_compress_read_bzip2(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	bz_stream *strm = (bz_stream*) state->lib_object;
	int res;
	if (state->is_compress) {
		res = BZ2_bzCompress(strm, BZ_RUN);
	} else {
		res = BZ2_bzDecompress(strm);
	}
	
	if (res != BZ_OK && res != BZ_STREAM_END && res != BZ_RUN_OK && res != BZ_PARAM_ERROR) {
		return COMPRESSION_RES_ERROR;
	}
	if (strm->avail_out == 0 || res == BZ_STREAM_END) {
		memcpy(buf, state->output_buf, state->bufsize - strm->avail_out);
		*lenout = state->bufsize - strm->avail_out;
		strm->next_out = (char*) state->output_buf;
		strm->avail_out = state->bufsize;
		if (res == BZ_STREAM_END) {
			return COMPRESSION_RES_FINAL;
		} else {
			return COMPRESSION_RES_OK;
		}
	} else {
		return COMPRESSION_RES_AGAIN;
	}
	
}

void impack_compress_write_bzip2(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	bz_stream *strm = (bz_stream*) state->lib_object;
	memcpy(state->input_buf, buf, len);
	strm->next_in = (char*) state->input_buf;
	strm->avail_in = len;
	
}

impack_compression_result_t impack_compress_flush_bzip2(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	bz_stream *strm = (bz_stream*) state->lib_object;
	int res = BZ2_bzCompress(strm, BZ_FINISH);
	if (res == BZ_STREAM_END) {
		memcpy(buf, state->output_buf, state->bufsize - strm->avail_out);
		*lenout = state->bufsize - strm->avail_out;
		return COMPRESSION_RES_FINAL;
	} else {
		memcpy(buf, state->output_buf, state->bufsize);
		strm->next_out = (char*) state->output_buf;
		strm->avail_out = state->bufsize;
		return COMPRESSION_RES_AGAIN;
	}
	
}

bool impack_compress_level_valid_bzip2(int32_t level) {
	
	return (level <= 9);
	
}

#endif
