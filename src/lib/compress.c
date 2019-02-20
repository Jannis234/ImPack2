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

#ifdef IMPACK_WITH_COMPRESSION

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "impack_internal.h"
#include "compression.h"

bool impack_compress_init(impack_compress_state_t *state) {
	
	switch (state->type) {
#ifdef IMPACK_WITH_ZLIB
		case COMPRESSION_ZLIB:
			return impack_compress_init_zlib(state);
#endif
#ifdef IMPACK_WITH_ZSTD
		case COMPRESSION_ZSTD:
			return impack_compress_init_zstd(state);
#endif
#ifdef IMPACK_WITH_LZMA
		case COMPRESSION_LZMA:
			return impack_compress_init_lzma(state);
#endif
		default:
			abort();
	}
	
}

void impack_compress_free(impack_compress_state_t *state) {
	
	switch (state->type) {
#ifdef IMPACK_WITH_ZLIB
		case COMPRESSION_ZLIB:
			impack_compress_free_zlib(state);
			return;
#endif
#ifdef IMPACK_WITH_ZSTD
		case COMPRESSION_ZSTD:
			impack_compress_free_zstd(state);
			return;
#endif
#ifdef IMPACK_WITH_LZMA
		case COMPRESSION_LZMA:
			impack_compress_free_lzma(state);
			return;
#endif
		default:
			abort();
	}
	
}

impack_compression_result_t impack_compress_read(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	switch (state->type) {
#ifdef IMPACK_WITH_ZLIB
		case COMPRESSION_ZLIB:
			return impack_compress_read_zlib(state, buf, lenout);
#endif
#ifdef IMPACK_WITH_ZSTD
		case COMPRESSION_ZSTD:
			return impack_compress_read_zstd(state, buf, lenout);
#endif
#ifdef IMPACK_WITH_LZMA
		case COMPRESSION_LZMA:
			return impack_compress_read_lzma(state, buf, lenout);
#endif
		default:
			abort();
	}
	
}

void impack_compress_write(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	switch (state->type) {
#ifdef IMPACK_WITH_ZLIB
		case COMPRESSION_ZLIB:
			impack_compress_write_zlib(state, buf, len);
			return;
#endif
#ifdef IMPACK_WITH_ZSTD
		case COMPRESSION_ZSTD:
			impack_compress_write_zstd(state, buf, len);
			return;
#endif
#ifdef IMPACK_WITH_LZMA
		case COMPRESSION_LZMA:
			impack_compress_write_lzma(state, buf, len);
			return;
#endif
		default:
			abort();
	}
	
}

impack_compression_result_t impack_compress_flush(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	switch (state->type) {
#ifdef IMPACK_WITH_ZLIB
		case COMPRESSION_ZLIB:
			return impack_compress_flush_zlib(state, buf, lenout);
#endif
#ifdef IMPACK_WITH_ZSTD
		case COMPRESSION_ZSTD:
			return impack_compress_flush_zstd(state, buf, lenout);
#endif
#ifdef IMPACK_WITH_LZMA
		case COMPRESSION_LZMA:
			return impack_compress_flush_lzma(state, buf, lenout);
#endif
		default:
			abort();
	}
	
}

#endif
