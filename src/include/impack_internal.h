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

#ifndef __IMPACK_INTERNAL_H__
#define __IMPACK_INTERNAL_H__

#include "config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef IMPACK_WITH_CRYPTO
#include <nettle/aes.h>
#include <nettle/camellia.h>
#include <nettle/serpent.h>
#include <nettle/twofish.h>
#endif
#include "impack.h"

#define IMPACK_FORMAT_VERSION 0

#define IMPACK_MAGIC_NUMBER { 73, 109, 80, 50 } // ASCII string "ImP2"

typedef enum {
	COMPRESSION_RES_OK, // Success, data written to buffer
	COMPRESSION_RES_AGAIN, // Buffer not full, need to refill input
	COMPRESSION_RES_FINAL, // Compressed data ended
	COMPRESSION_RES_ERROR
} impack_compression_result_t;

#ifdef IMPACK_WITH_CRYPTO
typedef struct {
	struct aes256_ctx aes;
	struct camellia256_ctx camellia;
	struct serpent_ctx serpent;
	struct twofish_ctx twofish;
	uint8_t iv[IMPACK_CRYPT_BLOCK_SIZE];
} impack_crypt_ctx_t;
#endif

typedef struct {
	void *lib_object;
	impack_compression_type_t type;
	int32_t level;
	bool is_compress;
	uint8_t *input_buf;
	uint8_t *output_buf;
	uint64_t bufsize;
} impack_compress_state_t;

typedef bool (*impack_compress_func_init_t)(impack_compress_state_t* state);
typedef void (*impack_compress_func_free_t)(impack_compress_state_t* state);
typedef impack_compression_result_t (*impack_compress_func_read_t)(impack_compress_state_t* state, uint8_t* buf, uint64_t* lenout);
typedef void (*impack_compress_func_write_t)(impack_compress_state_t* state, uint8_t* buf, uint64_t len);
typedef impack_compression_result_t (*impack_compress_func_flush_t)(impack_compress_state_t* state, uint8_t* buf, uint64_t* lenout);
typedef bool (*impack_compress_func_level_valid_t)(int32_t level);

// Get the filename from a path (similar to basename())
char* impack_filename(char *path);
// Convert numbers to network byte order, if needed (like htonl()/ntohl())
uint64_t impack_endian64(uint64_t val);
uint32_t impack_endian32(uint32_t val);
uint32_t impack_endian32_le(uint32_t val);
uint16_t impack_endian16_le(uint16_t val);
// CRC-64 calculation
void impack_crc_init();
void impack_crc(uint64_t *crc, uint8_t *buf, size_t buflen);
// Image read/write helpers (with library/format-specific code)
impack_error_t impack_write_img(char *output_path, FILE *output_file, uint8_t **pixeldata, uint64_t pixeldata_size, uint64_t pixeldata_pos, uint64_t img_width, uint64_t img_height, impack_img_format_t format);
impack_error_t impack_read_img(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size);
// Get secure random data
bool impack_random(uint8_t *dst, size_t count);
// Zero-out an area of memory, without the compiler optimizing it out
void impack_secure_erase(uint8_t *buf, size_t len);
// Derive a key from a passphrase
bool impack_derive_key(char *passphrase, uint8_t *keyout, size_t keysize, uint8_t *salt, size_t saltsize, impack_encryption_type_t type);
void impack_derive_key_legacy(char *passphrase, uint8_t *keyout, size_t keysize, uint8_t *salt, size_t saltsize);
#ifdef IMPACK_WITH_CRYPTO
void impack_set_encrypt_key(impack_crypt_ctx_t *ctx, uint8_t *key, impack_encryption_type_t type);
void impack_set_decrypt_key(impack_crypt_ctx_t *ctx, uint8_t *key, impack_encryption_type_t type);
void impack_encrypt(impack_crypt_ctx_t *ctx, uint8_t *data, uint64_t length, impack_encryption_type_t type);
void impack_decrypt(impack_crypt_ctx_t *ctx, uint8_t *data, uint64_t length, impack_encryption_type_t type);
#endif
bool impack_compress_init(impack_compress_state_t *state);
void impack_compress_free(impack_compress_state_t *state);
impack_compression_result_t impack_compress_read(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
void impack_compress_write(impack_compress_state_t *state, uint8_t *buf, uint64_t len);
impack_compression_result_t impack_compress_flush(impack_compress_state_t *state, uint8_t *buf, uint64_t *lenout);
// Load a file into memory, leave some bytes free at the start of the buffer if requested
impack_error_t impack_loadfile(FILE *f, uint8_t **buf, uint64_t *bufsize, uint64_t skip);

#endif
