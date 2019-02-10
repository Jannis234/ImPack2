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
#include <string.h>
#include "impack.h"
#include "impack_internal.h"

bool pixelbuf_read(impack_decode_state_t *state, uint8_t *buf, uint64_t len) {
	
	while (len > 0) {
		if (state->pixeldata_pos == state->pixeldata_size) {
			return false;
		}
		if (((state->pixeldata_pos % 3) == 0 && (state->channels & IMPACK_CHANNEL_RED)) || \
			(((state->pixeldata_pos % 3) == 1) && (state->channels & IMPACK_CHANNEL_GREEN)) || \
			(((state->pixeldata_pos % 3) == 2) && (state->channels & IMPACK_CHANNEL_BLUE))) {
			*buf = (state->pixeldata)[state->pixeldata_pos];
			buf++;
			len--;
		}
		state->pixeldata_pos++;
	}
	return true;
	
}

impack_error_t impack_decode_stage1(impack_decode_state_t *state, char *input_path) {
	
	FILE *input_file;
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

	impack_error_t res = impack_read_img(input_file, &state->pixeldata, &state->pixeldata_size);
	state->pixeldata_pos = 0;
	fclose(input_file);
	if (res != ERROR_OK) {
		return res;
	}

	if (state->pixeldata_size < 3) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->channels = 0;
	if (state->pixeldata[0] == 255) {
		state->channels |= IMPACK_CHANNEL_RED;
	}
	if (state->pixeldata[1] == 255) {
		state->channels |= IMPACK_CHANNEL_GREEN;
	}
	if (state->pixeldata[2] == 255) {
		state->channels |= IMPACK_CHANNEL_BLUE;
	}
	if (state->pixeldata[0] == 0 && state->pixeldata[1] == 0 && state->pixeldata[2] == 0) {
		state->channels = IMPACK_CHANNEL_RED; // Can use any channel on a grayscale image
	}
	if (state->channels == 0) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->pixeldata_pos = 3;

	uint8_t magic_buf[IMPACK_MAGIC_NUMBER_LEN];
	if (!pixelbuf_read(state, magic_buf, IMPACK_MAGIC_NUMBER_LEN)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	uint8_t magic[] = IMPACK_MAGIC_NUMBER;
	if (memcmp(magic_buf, magic, IMPACK_MAGIC_NUMBER_LEN) != 0) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	uint8_t flags[3];
	if (!pixelbuf_read(state, flags, 3)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (flags[0] != 0) { // Version number
		free(state->pixeldata);
		return ERROR_INPUT_IMG_VERSION;
	}
	// TODO: Check encryption and compression
	state->encryption = flags[1];
	state->encryption = flags[2];

	return ERROR_OK;
	
}

impack_error_t impack_decode_stage2(impack_decode_state_t *state) {

	uint64_t data_length;
	uint32_t filename_length;
	if (!pixelbuf_read(state, (uint8_t*) &data_length, 8)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (!pixelbuf_read(state, (uint8_t*) &filename_length, 4)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	data_length = impack_endian64(data_length);
	filename_length = impack_endian32(filename_length);

	// Quick sanity check to avoid allocating giant buffers
	uint64_t bytes_remaining = state->pixeldata_size - state->pixeldata_pos;
	if (filename_length > bytes_remaining) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (data_length > bytes_remaining - filename_length) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}

	uint64_t crc;
	if (!pixelbuf_read(state, (uint8_t*) &crc, 8)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->crc = impack_endian64(crc);
	if (state->encryption != 0) {
		if (!pixelbuf_read(state, state->aes_iv, 16)) {
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}
	}
	state->filename = malloc(filename_length);
	if (state->filename == NULL) {
		free(state->pixeldata);
		return ERROR_MALLOC;
	}
	if (!pixelbuf_read(state, (uint8_t*) state->filename, filename_length)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}

	return ERROR_OK;

}

impack_error_t impack_decode_stage3(impack_decode_state_t *state, char *output_path) {

	// TODO
	
	return ERROR_OK;

}

