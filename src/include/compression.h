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

#ifndef __IMPACK_COMPRESSION_H__
#define __IMPACK_COMPRESSION_H__

#include <stdbool.h>
#include <stdint.h>
#include "impack_internal.h"

bool impack_compress_init_zlib(impack_compress_state_t *state);
bool impack_compress_init_zstd(impack_compress_state_t *state);
bool impack_compress_init_lzma(impack_compress_state_t *state);
bool impack_compress_init_bzip2(impack_compress_state_t *state);
bool impack_compress_init_brotli(impack_compress_state_t *state);
void impack_compress_free_zlib(impack_compress_state_t *state);
void impack_compress_free_zstd(impack_compress_state_t *state);
void impack_compress_free_lzma(impack_compress_state_t *state);
void impack_compress_free_bzip2(impack_compress_state_t *state);
void impack_compress_free_brotli(impack_compress_state_t *state);
impack_compression_result_t impack_compress_read_zlib(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_read_zstd(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_read_lzma(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_read_bzip2(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_read_brotli(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
void impack_compress_write_zlib(impack_compress_state_t *state, uint8_t *buf, uint64_t len);
void impack_compress_write_zstd(impack_compress_state_t *state, uint8_t *buf, uint64_t len);
void impack_compress_write_lzma(impack_compress_state_t *state, uint8_t *buf, uint64_t len);
void impack_compress_write_bzip2(impack_compress_state_t *state, uint8_t *buf, uint64_t len);
void impack_compress_write_brotli(impack_compress_state_t *state, uint8_t *buf, uint64_t len);
impack_compression_result_t impack_compress_flush_zlib(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_flush_zstd(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_flush_lzma(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_flush_bzip2(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
impack_compression_result_t impack_compress_flush_brotli(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
bool impack_compress_level_valid_zlib(int32_t level);
bool impack_compress_level_valid_zstd(int32_t level);
bool impack_compress_level_valid_lzma(int32_t level);
bool impack_compress_level_valid_bzip2(int32_t level);
bool impack_compress_level_valid_brotli(int32_t level);

#endif
