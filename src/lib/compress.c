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

#ifdef IMPACK_WITH_ZLIB
const impack_compression_desc_t impack_compression_zlib = {
	COMPRESSION_ZLIB,
	"Deflate",
	(impack_compress_func_generic_t) impack_compress_init_zlib,
	(impack_compress_func_generic_t) impack_compress_free_zlib,
	(impack_compress_func_generic_t) impack_compress_read_zlib,
	(impack_compress_func_generic_t) impack_compress_write_zlib,
	(impack_compress_func_generic_t) impack_compress_flush_zlib,
	(impack_compress_func_generic_t) impack_compress_level_valid_zlib
};
#endif

#ifdef IMPACK_WITH_ZSTD
const impack_compression_desc_t impack_compression_zstd = {
	COMPRESSION_ZSTD,
	"ZSTD",
	(impack_compress_func_generic_t) impack_compress_init_zstd,
	(impack_compress_func_generic_t) impack_compress_free_zstd,
	(impack_compress_func_generic_t) impack_compress_read_zstd,
	(impack_compress_func_generic_t) impack_compress_write_zstd,
	(impack_compress_func_generic_t) impack_compress_flush_zstd,
	(impack_compress_func_generic_t) impack_compress_level_valid_zstd
};
#endif

#ifdef IMPACK_WITH_LZMA
const impack_compression_desc_t impack_compression_lzma = {
	COMPRESSION_LZMA,
	"LZMA2",
	(impack_compress_func_generic_t) impack_compress_init_lzma,
	(impack_compress_func_generic_t) impack_compress_free_lzma,
	(impack_compress_func_generic_t) impack_compress_read_lzma,
	(impack_compress_func_generic_t) impack_compress_write_lzma,
	(impack_compress_func_generic_t) impack_compress_flush_lzma,
	(impack_compress_func_generic_t) impack_compress_level_valid_lzma
};
#endif

#ifdef IMPACK_WITH_BZIP2
const impack_compression_desc_t impack_compression_bzip2 = {
	COMPRESSION_BZIP2,
	"Bzip2",
	(impack_compress_func_generic_t) impack_compress_init_bzip2,
	(impack_compress_func_generic_t) impack_compress_free_bzip2,
	(impack_compress_func_generic_t) impack_compress_read_bzip2,
	(impack_compress_func_generic_t) impack_compress_write_bzip2,
	(impack_compress_func_generic_t) impack_compress_flush_bzip2,
	(impack_compress_func_generic_t) impack_compress_level_valid_bzip2
};
#endif

#ifdef IMPACK_WITH_BROTLI
const impack_compression_desc_t impack_compression_brotli = {
	COMPRESSION_BROTLI,
	"Brotli",
	(impack_compress_func_generic_t) impack_compress_init_brotli,
	(impack_compress_func_generic_t) impack_compress_free_brotli,
	(impack_compress_func_generic_t) impack_compress_read_brotli,
	(impack_compress_func_generic_t) impack_compress_write_brotli,
	(impack_compress_func_generic_t) impack_compress_flush_brotli,
	(impack_compress_func_generic_t) impack_compress_level_valid_brotli
};
#endif

const impack_compression_desc_t *impack_compression_types[] = {
#ifdef IMPACK_WITH_BROTLI
	&impack_compression_brotli,
#endif
#ifdef IMPACK_WITH_BZIP2
	&impack_compression_bzip2,
#endif
#ifdef IMPACK_WITH_ZLIB
	&impack_compression_zlib,
#endif
#ifdef IMPACK_WITH_LZMA
	&impack_compression_lzma,
#endif
#ifdef IMPACK_WITH_ZSTD
	&impack_compression_zstd,
#endif
	NULL
};

bool impack_compress_init(impack_compress_state_t *state) {
	
	int i = 0;
	while (impack_compression_types[i] != NULL) {
		if (impack_compression_types[i]->id == state->type) {
			return ((impack_compress_func_init_t) impack_compression_types[i]->func_init)(state);
		}
		i++;
	}
	abort();
	
}

void impack_compress_free(impack_compress_state_t *state) {
	
	int i = 0;
	while (impack_compression_types[i] != NULL) {
		if (impack_compression_types[i]->id == state->type) {
			((impack_compress_func_free_t) impack_compression_types[i]->func_free)(state);
			return;
		}
		i++;
	}
	abort();
	
}

impack_compression_result_t impack_compress_read(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	int i = 0;
	while (impack_compression_types[i] != NULL) {
		if (impack_compression_types[i]->id == state->type) {
			return ((impack_compress_func_read_t) impack_compression_types[i]->func_read)(state, buf, lenout);
		}
		i++;
	}
	abort();
	
}

void impack_compress_write(impack_compress_state_t *state, uint8_t *buf, uint64_t len) {
	
	int i = 0;
	while (impack_compression_types[i] != NULL) {
		if (impack_compression_types[i]->id == state->type) {
			((impack_compress_func_write_t) impack_compression_types[i]->func_write)(state, buf, len);
			return;
		}
		i++;
	}
	abort();
	
}

impack_compression_result_t impack_compress_flush(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout) {
	
	int i = 0;
	while (impack_compression_types[i] != NULL) {
		if (impack_compression_types[i]->id == state->type) {
			return ((impack_compress_func_flush_t) impack_compression_types[i]->func_flush)(state, buf, lenout);
		}
		i++;
	}
	abort();
	
}

bool impack_compress_level_valid(impack_compression_type_t type, int32_t level) {
	
	int i = 0;
	while (impack_compression_types[i] != NULL) {
		if (impack_compression_types[i]->id == type) {
			return ((impack_compress_func_level_valid_t) impack_compression_types[i]->func_level_valid)(level);
		}
		i++;
	}
	abort();
	
}

#endif
