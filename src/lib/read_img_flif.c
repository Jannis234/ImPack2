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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <flif.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_read_img_flif(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t *buf;
	uint64_t bufsize;
	impack_error_t res = impack_loadfile(input_file, &buf, &bufsize, 4);
	if (res != ERROR_OK) {
		return res;
	}
	memcpy(buf, magic, 4);
	
	impack_error_t ret = ERROR_MALLOC;
	*pixeldata = NULL;
	FLIF_DECODER *decoder = flif_create_decoder();
	if (decoder == NULL) {
		goto cleanup;
	}
	if (flif_decoder_decode_memory(decoder, buf, bufsize) == 0) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	free(buf);
	buf = NULL;
	
	FLIF_IMAGE *img = flif_decoder_get_image(decoder, 0);
	if (img == NULL) {
		goto cleanup;
	}
	uint32_t width = flif_image_get_width(img);
	uint32_t height = flif_image_get_height(img);
	*pixeldata_size = width * height * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		goto cleanup;
	}
	uint8_t *row = malloc(width * 4);
	if (row == NULL) {
		goto cleanup;
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
	
cleanup:
	if (buf != NULL) {
		free(buf);
	}
	if (*pixeldata != NULL) {
		free(*pixeldata);
	}
	flif_destroy_decoder(decoder);
	return ret;
	
}

#endif
