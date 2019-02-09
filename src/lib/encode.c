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

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "impack.h"
#include "impack_internal.h"

#define BUFSIZE 16384 // 16 KiB
#define PIXELBUF_STEP 1048576 // 1 MiB

bool pixelbuf_add(uint8_t **pixeldata, uint64_t *pixeldata_size, uint64_t *pixeldata_pos, uint8_t channels, uint8_t *data, uint64_t len) {
	
	while (len > 0) {
		if (*pixeldata_pos == *pixeldata_size) {
			uint8_t *newbuf = realloc(*pixeldata, *pixeldata_size + PIXELBUF_STEP);
			if (newbuf == NULL) {
				return false;
			}
			*pixeldata = newbuf;
			*pixeldata_size += PIXELBUF_STEP;
		}

		if (((*pixeldata_pos % 3 == 0) && (channels & IMPACK_CHANNEL_RED)) || \
			((*pixeldata_pos % 3 == 1) && (channels & IMPACK_CHANNEL_GREEN)) || \
			((*pixeldata_pos % 3 == 2) && (channels & IMPACK_CHANNEL_BLUE))) { // Current channel enabled
			**pixeldata = *data;
			data++;
			len--;
		}
		(*pixeldata_pos)++;
	}
	return true;
	
}

impack_error_t impack_encode(char *input_path, char *output_path, uint32_t img_format) {
	
	FILE *input_file, *output_file;
	if (strlen(input_path) == 1 && input_path[0] == '-') {
		input_file = stdin;
	} else {
		input_file = fopen(input_path, "rb");
		if (input_file == NULL) {
			if (errno == ENOENT) {
				return ERROR_INPUT_NOT_FOUND;
			} else if (errno == EACCES) {
				return ERROR_INPUT_PERMISSION;
			} else if (errno == EISDIR) {
				return ERROR_INPUT_DIRECTORY;
			} else {
				return ERROR_INPUT_IO;
			}
		}
	}
	if (strlen(output_path) == 1 && output_path[0] == '-') {
		output_file = stdout;
	} else {
		output_file = fopen(output_path, "wb");
		if (output_file == NULL) {
			fclose(input_file);
			if (errno == ENOENT) {
				return ERROR_OUTPUT_NOT_FOUND;
			} else if (errno == EACCES) {
				return ERROR_OUTPUT_PERMISSION;
			} else if (errno == EISDIR) {
				return ERROR_OUTPUT_DIRECTORY;
			} else { 
				return ERROR_OUTPUT_IO;
			}
		}
	}

	uint8_t *input_buf = malloc(BUFSIZE);
	if (input_buf == NULL) {
		fclose(input_file);
		fclose(output_file);
		return ERROR_MALLOC;
	}
	uint8_t *pixeldata = malloc(PIXELBUF_STEP);
	if (pixeldata == NULL) {
		free(input_buf);
		fclose(input_file);
		fclose(output_file);
		return ERROR_MALLOC;
	}
	uint64_t pixeldata_size = PIXELBUF_STEP;
	uint64_t pixeldata_pos = 3;

	uint8_t channels = IMPACK_CHANNEL_RED | IMPACK_CHANNEL_GREEN | IMPACK_CHANNEL_BLUE; // TODO: Allow selecting channels
	pixeldata[0] = 0xFF;
	pixeldata[1] = 0xFF;
	pixeldata[2] = 0xFF;
	
	uint8_t magic[] = IMPACK_MAGIC_NUMBER;
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, magic, IMPACK_MAGIC_NUMBER_LEN); // These will not fail, the buffer is large enough
	uint8_t format_version = IMPACK_FORMAT_VERSION;
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &format_version, 1);
	uint8_t encryption_flag = 0; // TODO: Encryption
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &encryption_flag, 1);
	uint8_t compression_flag = 0; // TODO: Compression
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &compression_flag, 1);
	uint64_t length_offset = pixeldata_pos;
	for (int i = 0; i < 8; i++) { // Add a dummy value that will be replaced when the length is known
		uint8_t dummy = 0;
		pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &dummy, 1);
	}
	char *input_filename = impack_filename(input_path);
	uint32_t input_filename_length = strlen(input_filename);
	uint32_t input_filename_length_add = impack_endian(input_filename_length);
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, (uint8_t*) &input_filename_length_add, 4);
	uint64_t crc_offset = pixeldata_pos;
	for (int i = 0; i < 4; i++) { // More dummy bytes for the CRC
		uint8_t dummy = 0;
		pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &dummy, 1);
	}

	// TODO: Pad and encrypt the filename
	if (!pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, (uint8_t*) input_filename, input_filename_length)) {
		free(pixeldata);
		free(input_buf);
		fclose(input_file);
		fclose(output_file);
		return ERROR_MALLOC;
	}

	size_t bytes_read;
	do {
		bytes_read = fread(input_buf, 1, BUFSIZE, input_file);
		// TODO: Compression + encryption
		if (!pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, input_buf, bytes_read)) {
			free(pixeldata);
			free(input_buf);
			fclose(input_file);
			fclose(output_file);
			return ERROR_MALLOC;
		}
	} while (bytes_read == BUFSIZE);
	free(input_buf);
	if (!feof(input_file)) {
		free(pixeldata);
		fclose(input_file);
		fclose(output_file);
		return ERROR_INPUT_IO;
	}
	fclose(input_file);

	free(pixeldata);
	fclose(output_file);
	return ERROR_OK;
	
}

