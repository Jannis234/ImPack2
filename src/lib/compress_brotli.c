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

#ifdef IMPACK_WITH_BROTLI

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <brotli/decode.h>
#include <brotli/encode.h>
#include "impack_internal.h"

typedef struct {
	uint8_t *next_in;
	size_t avail_in;
	uint8_t *next_out;
	size_t avail_out;
	BrotliEncoderState *enc;
	BrotliDecoderState *dec;
} impack_brotli_state_t;

void* impack_brotli_malloc(void *opaque, long unsigned int size) {
	
	return malloc(size);
	
}

void impack_brotli_free(void *opaque, void *mem) {
	
	free(mem);
	
}

bool impack_compress_init_brotli(impack_compress_state_t *state) {
	
	impack_brotli_state_t *strm = malloc(sizeof(impack_brotli_state_t));
	if (strm == NULL) {
		return ERROR_MALLOC;
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
	strm->next_in = state->input_buf;
	strm->next_out = state->output_buf;
	strm->avail_in = 0;
	strm->avail_out = state->bufsize;
	
	if (state->is_compress) {
		strm->enc = BrotliEncoderCreateInstance(impack_brotli_malloc, impack_brotli_free, NULL);
		if (strm->enc == 0) {
			free(strm);
			free(state->input_buf);
			free(state->output_buf);
			return false;
		}
		if (state->level == 0) {
			state->level = BROTLI_DEFAULT_QUALITY;
		}
		BrotliEncoderSetParameter(strm->enc, BROTLI_PARAM_QUALITY, state->level);
	} else {
		strm->dec = BrotliDecoderCreateInstance(impack_brotli_malloc, impack_brotli_free, NULL);
		if (strm->dec == 0) {
			free(strm);
			free(state->input_buf);
			free(state->output_buf);
			return false;
		}
	}
	state->lib_object = strm;
	return true;
	
}

void impack_compress_free_brotli(impack_compress_state_t *state) {
	
	impack_brotli_state_t *strm = (impack_brotli_state_t*) state->lib_object;
	if (state->is_compress) {
		BrotliEncoderDestroyInstance(strm->enc);
	} else {
		BrotliDecoderDestroyInstance(strm->dec);
	}
	free(state->input_buf);
	free(state->output_buf);
	free(strm);
	
}

impack_compression_result_t impack_compress_read_brotli(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	impack_brotli_state_t *strm = (impack_brotli_state_t*) state->lib_object;
	size_t dummy = 0;
	if (state->is_compress) {
		BrotliEncoderCompressStream(strm->enc, BROTLI_OPERATION_PROCESS, &strm->avail_in, (const uint8_t**) &strm->next_in, &strm->avail_out, &strm->next_out, &dummy);
	} else {
		BrotliDecoderResult res = BrotliDecoderDecompressStream(strm->dec, &strm->avail_in, (const uint8_t**) &strm->next_in, &strm->avail_out, &strm->next_out, &dummy);
		if (res == BROTLI_DECODER_RESULT_ERROR) {
			return COMPRESSION_RES_ERROR;
		}
	}
	bool final = false;
	if (!state->is_compress) {
		if (BrotliDecoderIsFinished(strm->dec)) {
			final = true;
		}
	}
	if (strm->avail_out == 0 || final) {
		memcpy(buf, state->output_buf, state->bufsize - strm->avail_out);
		*lenout = state->bufsize - strm->avail_out;
		strm->next_out = state->output_buf;
		strm->avail_out = state->bufsize;
		if (final) {
			return COMPRESSION_RES_FINAL;
		} else {
			return COMPRESSION_RES_OK;
		}
	} else {
		return COMPRESSION_RES_AGAIN;
	}
	
}

void impack_compress_write_brotli(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	impack_brotli_state_t *strm = (impack_brotli_state_t*) state->lib_object;
	memcpy(state->input_buf, buf, len);
	strm->next_in = state->input_buf;
	strm->avail_in = len;
	
}

impack_compression_result_t impack_compress_flush_brotli(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	impack_brotli_state_t *strm = (impack_brotli_state_t*) state->lib_object;
	size_t dummy = 0;
	BrotliEncoderCompressStream(strm->enc, BROTLI_OPERATION_FINISH, &strm->avail_in, (const uint8_t**) &strm->next_in, &strm->avail_out, &strm->next_out, &dummy);
	if (BrotliEncoderIsFinished(strm->enc)) {
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

bool impack_compress_level_valid_brotli(int32_t level) {
	
	return (level <= BROTLI_MAX_QUALITY);
	
}

#endif
