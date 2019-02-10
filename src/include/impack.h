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

#include <stdint.h>

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
	ERROR_IMG_FORMAT_UNSUPPORTED, // File format not compiled in
	ERROR_IMG_FORMAT_UNKNOWN, // Unknown file format
	ERROR_INPUT_IMG_INVALID, // Invalid image contents
	ERROR_INPUT_IMG_VERSION // Incompatible format, created with a newer version of ImPack2
} impack_error_t;

#define IMPACK_CHANNEL_RED 1
#define IMPACK_CHANNEL_GREEN 2
#define IMPACK_CHANNEL_BLUE 4

typedef struct {
	uint8_t *pixeldata;
	uint64_t pixeldata_size;
	uint64_t pixeldata_pos;
	uint8_t channels;
	uint8_t encryption;
	uint8_t compression;
	uint64_t crc;
	uint8_t aes_key[32];
	uint8_t aes_iv[16];
	char *filename;
} impack_decode_state_t;

impack_error_t impack_encode(char *input_path, char *output_path);
// Decode stage 1: Load the image and check if the content is encrypted (may ask for the passphrase after this)
impack_error_t impack_decode_stage1(impack_decode_state_t *state, char *input_path);
// Decode stage 2: Extract the included filename (select final output path after this)
impack_error_t impack_decode_stage2(impack_decode_state_t *state);
// Decode stage 3: Extract and save the actual content
impack_error_t impack_decode_stage3(impack_decode_state_t *state, char *output_path);

#endif

