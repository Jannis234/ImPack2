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

#ifdef IMPACK_WITH_LZMA

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <lzma.h>
#include "impack_internal.h"

bool impack_compress_init_lzma(impack_compress_state_t *state) {
	
	lzma_stream *strm = malloc(sizeof(lzma_stream));
	if (strm == NULL) {
		return false;
	}
	memset(strm, 0, sizeof(lzma_stream));
	lzma_ret res;
	if (state->is_compress) {
		res = lzma_easy_encoder(strm, 6, LZMA_CHECK_NONE);
	} else {
		res = lzma_stream_decoder(strm, UINT64_MAX, LZMA_IGNORE_CHECK);
	}
	if (res != LZMA_OK) {
		free(strm);
		return false;
	}
	state->input_buf = malloc(state->bufsize);
	if (state->input_buf == NULL) {
		lzma_end(strm);
		free(strm);
		return false;
	}
	state->output_buf = malloc(state->bufsize);
	if (state->output_buf == NULL) {
		free(state->input_buf);
		lzma_end(strm);
		free(strm);
		return false;
	}
	strm->next_in = state->input_buf;
	strm->next_out = state->output_buf;
	strm->avail_in = 0;
	strm->avail_out = state->bufsize;
	state->lib_object = strm;
	return true;
	
}

void impack_compress_free_lzma(impack_compress_state_t *state) {
	
	lzma_stream *strm = (lzma_stream*) state->lib_object;
	lzma_end(strm);
	free(state->input_buf);
	free(state->output_buf);
	free(strm);
	
}

impack_compression_result_t impack_compress_read_lzma(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	lzma_stream *strm = (lzma_stream*) state->lib_object;
	lzma_ret res = lzma_code(strm, LZMA_RUN);
	if (res != LZMA_OK && res != LZMA_STREAM_END && res != LZMA_BUF_ERROR) {
		return COMPRESSION_RES_ERROR;
	}
	if (strm->avail_out == 0 || res == LZMA_STREAM_END) {
		memcpy(buf, state->output_buf, state->bufsize - strm->avail_out);
		*lenout = state->bufsize - strm->avail_out;
		strm->next_out = state->output_buf;
		strm->avail_out = state->bufsize;
		if (res == LZMA_STREAM_END) {
			return COMPRESSION_RES_FINAL;
		} else {
			return COMPRESSION_RES_OK;
		}
	} else {
		return COMPRESSION_RES_AGAIN;
	}
	
}

void impack_compress_write_lzma(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	lzma_stream *strm = (lzma_stream*) state->lib_object;
	memcpy(state->input_buf, buf, len);
	strm->next_in = state->input_buf;
	strm->avail_in = len;
	
}

impack_compression_result_t impack_compress_flush_lzma(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	lzma_stream *strm = (lzma_stream*) state->lib_object;
	if (lzma_code(strm, LZMA_FINISH) == LZMA_STREAM_END) {
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

#endif
