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

#ifdef IMPACK_WITH_FLIF

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <flif.h>
#include "impack.h"
#include "img.h"

#define BUFSTEP 16384 // 16 KiB

impack_error_t impack_read_img_flif(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t *buf = malloc(BUFSTEP + 4);
	uint64_t bufsize = BUFSTEP + 4;
	if (buf == NULL) {
		return ERROR_MALLOC;
	}
	uint8_t magic[] = IMPACK_MAGIC_FLIF;
	memcpy(buf, magic, 4);
	size_t bytes_read;
	do {
		bytes_read = fread(buf + bufsize - BUFSTEP, 1, BUFSTEP, input_file);
		if (bytes_read == BUFSTEP) {
			uint8_t *newbuf = realloc(buf, bufsize + BUFSTEP);
			if (newbuf == NULL) {
				free(buf);
				return ERROR_MALLOC;
			}
			buf = newbuf;
			bufsize += BUFSTEP;
		}
	} while (bytes_read == BUFSTEP);
	bufsize = bufsize - BUFSTEP + bytes_read;
	if (!feof(input_file)) {
		return ERROR_INPUT_IO;
	}
	
	FLIF_DECODER *decoder = flif_create_decoder();
	if (decoder == NULL) {
		free(buf);
		return ERROR_MALLOC;
	}
	if (flif_decoder_decode_memory(decoder, buf, bufsize) == 0) {
		free(buf);
		flif_destroy_decoder(decoder);
		return ERROR_INPUT_IMG_INVALID;
	}
	free(buf);
	
	FLIF_IMAGE *img = flif_decoder_get_image(decoder, 0);
	if (img == NULL) {
		flif_destroy_decoder(decoder);
		return ERROR_MALLOC;
	}
	uint32_t width = flif_image_get_width(img);
	uint32_t height = flif_image_get_height(img);
	*pixeldata_size = width * height * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		flif_destroy_decoder(decoder);
		return ERROR_MALLOC;
	}
	uint8_t *row = malloc(width * 4);
	if (row == NULL) {
		flif_destroy_decoder(decoder);
		free(*pixeldata);
		return ERROR_MALLOC;
	}
	
	for (uint32_t y = 0; y < height; y++) {
		flif_image_read_row_RGBA8(img, y, row, width * 4);
		for (uint32_t x = 0; x < width; x++) {
			(*pixeldata)[(y * width * 3) + (x * 3)] = row[x * 4];
			(*pixeldata)[(y * width * 3) + (x * 3) + 1] = row[(x * 4) + 1];
			(*pixeldata)[(y * width * 3) + (x * 3) + 2] = row[(x * 4) + 2];
		}
	}
	free(row);
	flif_destroy_decoder(decoder);
	return ERROR_OK;
	
}

#endif
