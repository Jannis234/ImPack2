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

#ifdef IMPACK_WITH_ZSTD

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>
#include "impack_internal.h"

typedef struct {
	ZSTD_CStream *cstrm;
	ZSTD_DStream *dstrm;
	ZSTD_inBuffer inbuf;
	ZSTD_outBuffer outbuf;
} impack_zstd_state_t;

bool impack_compress_init_zstd(impack_compress_state_t *state) {
	
	impack_zstd_state_t *zstate = malloc(sizeof(impack_zstd_state_t));
	if (zstate == NULL) {
		return false;
	}
	state->lib_object = zstate;
	state->input_buf = malloc(state->bufsize);
	zstate->inbuf.src = state->input_buf;
	if (zstate->inbuf.src == NULL) {
		free(zstate);
		return false;
	}
	zstate->inbuf.size = state->bufsize;
	zstate->inbuf.pos = state->bufsize;
	state->output_buf = malloc(state->bufsize);
	zstate->outbuf.dst = state->output_buf;
	if (zstate->outbuf.dst == NULL) {
		free((void*) zstate->inbuf.src);
		free(zstate);
		return false;
	}
	zstate->outbuf.size = state->bufsize;
	zstate->outbuf.pos = 0;
	zstate->cstrm = NULL;
	zstate->dstrm = NULL;
	if (state->is_compress) {
		zstate->cstrm = ZSTD_createCStream();
		if (zstate->cstrm == NULL) {
			free((void*) zstate->inbuf.src);
			free(zstate->outbuf.dst);
			free(zstate);
			return false;
		}
		if (ZSTD_isError(ZSTD_initCStream(zstate->cstrm, 3))) { // Default compression level 3, from zstd source code
			free((void*) zstate->inbuf.src);
			free(zstate->outbuf.dst);
			ZSTD_freeCStream(zstate->cstrm);
			free(zstate);
			return false;
		}
	} else {
		zstate->dstrm = ZSTD_createDStream();
		if (zstate->dstrm == NULL) {
			free((void*) zstate->inbuf.src);
			free(zstate->outbuf.dst);
			free(zstate);
			return false;
		}
		if (ZSTD_isError(ZSTD_initDStream(zstate->dstrm))) {
			free((void*) zstate->inbuf.src);
			free(zstate->outbuf.dst);
			ZSTD_freeDStream(zstate->dstrm);
			free(zstate);
			return false;
		}
	}
	return true;
	
}

void impack_compress_free_zstd(impack_compress_state_t *state) {
	
	impack_zstd_state_t *zstate = (impack_zstd_state_t*) state->lib_object;
	free((void*) zstate->inbuf.src);
	free(zstate->outbuf.dst);
	if (zstate->cstrm != NULL) {
		ZSTD_freeCStream(zstate->cstrm);
	}
	if (zstate->dstrm != NULL) {
		ZSTD_freeDStream(zstate->dstrm);
	}
	free(zstate);
	
}

impack_compression_result_t impack_compress_read_zstd(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	impack_zstd_state_t *zstate = (impack_zstd_state_t*) state->lib_object;
	if (state->is_compress) {
		size_t res = ZSTD_compressStream(zstate->cstrm, &zstate->outbuf, &zstate->inbuf);
		if (ZSTD_isError(res)) {
			return COMPRESSION_RES_ERROR;
		}
		if (zstate->outbuf.pos == state->bufsize) {
			memcpy(buf, zstate->outbuf.dst, zstate->outbuf.pos);
			*lenout = zstate->outbuf.pos;
			zstate->outbuf.pos = 0;
			return COMPRESSION_RES_OK;
		} else {
			return COMPRESSION_RES_AGAIN;
		}
	} else {
		size_t res = ZSTD_decompressStream(zstate->dstrm, &zstate->outbuf, &zstate->inbuf);
		if (ZSTD_isError(res)) {
			return COMPRESSION_RES_ERROR;
		}
		if (zstate->outbuf.pos == state->bufsize || res == 0) {
			memcpy(buf, zstate->outbuf.dst, zstate->outbuf.pos);
			*lenout = zstate->outbuf.pos;
			zstate->outbuf.pos = 0;
			if (res == 0) {
				return COMPRESSION_RES_FINAL;
			} else {
				return COMPRESSION_RES_OK;
			}
		} else {
			return COMPRESSION_RES_AGAIN;
		}
	}
	
}

void impack_compress_write_zstd(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	impack_zstd_state_t *zstate = (impack_zstd_state_t*) state->lib_object;
	memcpy((void*) zstate->inbuf.src, buf, len);
	zstate->inbuf.pos = 0;
	zstate->inbuf.size = len;
	
}

impack_compression_result_t impack_compress_flush_zstd(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	impack_zstd_state_t *zstate = (impack_zstd_state_t*) state->lib_object;
	if (zstate->inbuf.pos != zstate->inbuf.size) { // Process remaining data, if any
		impack_compression_result_t res = impack_compress_read_zstd(state, buf, lenout);
		if (res == COMPRESSION_RES_ERROR) {
			return res;
		} else if (res == COMPRESSION_RES_OK) {
			return COMPRESSION_RES_AGAIN; // outbuf is full
		}
	}
	
	size_t res = ZSTD_endStream(zstate->cstrm, &zstate->outbuf);
	if (ZSTD_isError(res)) {
		return COMPRESSION_RES_ERROR;
	}
	if (res == 0) {
		memcpy(buf, zstate->outbuf.dst, zstate->outbuf.pos);
		*lenout = zstate->outbuf.pos;
		return COMPRESSION_RES_FINAL;
	} else {
		memcpy(buf, zstate->outbuf.dst, state->bufsize);
		zstate->outbuf.pos = 0;
		return COMPRESSION_RES_AGAIN;
	}
	
}

#endif
