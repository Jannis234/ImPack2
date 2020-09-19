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

#ifndef __IMPACK_H__
#define __IMPACK_H__

#include "config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
	ERROR_OK, // Success
	ERROR_MALLOC, // malloc() failed
	ERROR_INPUT_NOT_FOUND, // Input file not found
	ERROR_INPUT_PERMISSION, // Access to input file denied
	ERROR_INPUT_DIRECTORY, // Input path is a directory
	ERROR_INPUT_IO, // I/O error while reading input file
	ERROR_OUTPUT_NOT_FOUND, // Output path not found
	ERROR_OUTPUT_PERMISSION, // Access to output file denied
	ERROR_OUTPUT_DIRECTORY, // Output path is a directory
	ERROR_OUTPUT_IO, // I/O error while writing output file
	ERROR_IMG_SIZE, // Invalid size for output image
	ERROR_IMG_TOO_SMALL, // Data doesn't fit into the image
	ERROR_IMG_FORMAT_UNSUPPORTED, // File format not compiled in
	ERROR_IMG_FORMAT_UNKNOWN, // Unknown file format
	ERROR_INPUT_IMG_INVALID, // Invalid image contents
	ERROR_INPUT_IMG_VERSION, // Incompatible format, created with a newer version of ImPack2
	ERROR_CRC, // CRC mismatch after decoding
	ERROR_RANDOM, // Can't get random data for encryption
	ERROR_ENCRYPTION_UNAVAILABLE, // Image encrypted, but encryption not compiled in
	ERROR_ENCRYPTION_UNKNOWN, // Unknown encryption algorithm
	ERROR_COMPRESSION_UNAVAILABLE, // Compression not compiled in at all
	ERROR_COMPRESSION_UNSUPPORTED, // Required compression algorithm not compiled in
	ERROR_COMPRESSION_UNKNOWN // Unknown compression algorithm
} impack_error_t;

#define IMPACK_CRYPT_BLOCK_SIZE 16 // 128 bits
#define IMPACK_CRYPT_KEY_SIZE 32 // 256 bits

typedef enum {
	ENCRYPTION_NONE = 0,
	ENCRYPTION_AES = 1,
	ENCRYPTION_CAMELLIA = 2,
	ENCRYPTION_SERPENT = 3,
	ENCRYPTION_TWOFISH = 4
} impack_encryption_type_t;

typedef enum {
	COMPRESSION_NONE = 0,
	COMPRESSION_ZLIB = 1,
	COMPRESSION_ZSTD = 2,
	COMPRESSION_LZMA = 3,
	COMPRESSION_BZIP2 = 4,
	COMPRESSION_BROTLI = 5
} impack_compression_type_t;

typedef enum {
	FORMAT_AUTO,
	FORMAT_PNG,
	FORMAT_WEBP,
	FORMAT_TIFF,
	FORMAT_BMP,
	FORMAT_JP2K,
	FORMAT_FLIF,
	FORMAT_JXR,
	FORMAT_JPEGLS
} impack_img_format_t;

typedef impack_error_t (*impack_read_img_t)(FILE* input_file, uint8_t** pixeldata, uint64_t* pixeldata_size);
typedef impack_error_t (*impack_write_img_t)(FILE* output_file, uint8_t* pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);

typedef struct {
	impack_img_format_t id;
	char *name; // Name displayed in CLI/GUI
	char *extension; // Default file extension (format "*.ext" used for GTK)
	const char **extension_alt; // Alternative file extensions for format auto-detection from filename
	bool hidden; // Exclude from lists in CLI/GUI (used to create alternative entries for the same format)
	impack_read_img_t func_read;
	impack_write_img_t func_write;
	const uint8_t *magic; // Magic number + length for format auto-detection
	int magic_len;
} impack_img_format_desc_t;

extern const impack_img_format_desc_t *impack_img_formats[];

typedef enum {
	CHANNEL_RED = 1,
	CHANNEL_GREEN = 2,
	CHANNEL_BLUE = 4
} impack_channel_t;

typedef struct {
	uint8_t *pixeldata;
	uint64_t pixeldata_size;
	uint64_t pixeldata_pos;
	uint8_t channels;
	uint8_t encryption;
	uint8_t compression;
	uint64_t crc;
	uint8_t checksum_legacy[64];
	bool legacy;
	uint8_t crypt_key[IMPACK_CRYPT_KEY_SIZE];
	uint8_t crypt_iv[IMPACK_CRYPT_BLOCK_SIZE];
	uint64_t data_length;
	uint32_t filename_length;
	char *filename;
} impack_decode_state_t;

impack_error_t impack_encode(char *input_path, char *output_path, impack_encryption_type_t encrypt, char *passphrase, impack_compression_type_t compress, int32_t compress_level, uint8_t channels, uint64_t img_width, uint64_t img_height, impack_img_format_t format, char *filename_include);
// Decode stage 1: Load the image and check if the content is encrypted (may ask for the passphrase after this)
impack_error_t impack_decode_stage1(impack_decode_state_t *state, char *input_path);
// Decode stage 2: Extract the included filename (select final output path after this)
impack_error_t impack_decode_stage2(impack_decode_state_t *state, char *passphrase);
// Decode stage 3: Extract and save the actual content
impack_error_t impack_decode_stage3(impack_decode_state_t *state, char *output_path);

// Helpers to select things depending on what's compiled in
impack_img_format_t impack_select_img_format(char *name, bool fileextension);
impack_img_format_t impack_default_img_format();
impack_compression_type_t impack_select_compression(char *name);
impack_compression_type_t impack_default_compression();
impack_encryption_type_t impack_select_encryption(char *name);
impack_encryption_type_t impack_default_encryption();

bool impack_compress_level_valid(impack_compression_type_t type, int32_t level);

#endif
