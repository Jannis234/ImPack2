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

#ifdef IMPACK_WITH_PNG

#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <png.h>
#include "impack.h"

impack_error_t impack_read_img_png(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	png_structp read_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (read_struct == NULL) {
		return ERROR_MALLOC;
	}
	png_infop info_struct = png_create_info_struct(read_struct);
	if (info_struct == NULL) {
		png_destroy_read_struct(&read_struct, NULL, NULL);
		return ERROR_MALLOC;
	}

	*pixeldata = NULL;
	uint8_t **row_pointers = NULL;
	if (setjmp(png_jmpbuf(read_struct))) {
		png_destroy_read_struct(&read_struct, &info_struct, NULL);
		if (pixeldata != NULL) {
			free(pixeldata);
		}
		if (row_pointers != NULL) {
			free(row_pointers);
		}
		return ERROR_INPUT_IO;
	}

	png_init_io(read_struct, input_file);
	png_set_sig_bytes(read_struct, 8); // Skip the magic number that was already read previously
	png_set_user_limits(read_struct, INT32_MAX, INT32_MAX); // Let the user process stupidly large images (if they have the required memory)
	
	png_read_info(read_struct, info_struct);
	uint32_t width, height;
	int32_t bit_depth, color_type;
	png_get_IHDR(read_struct, info_struct, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	if (bit_depth > 8) {
		png_destroy_read_struct(&read_struct, &info_struct, NULL);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(read_struct);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(read_struct);
	}
	if (bit_depth < 8) {
		png_set_packing(read_struct);
	}
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(read_struct);
	}

	*pixeldata = malloc((uint64_t) width * (uint64_t) height * 3);
	if (*pixeldata == NULL) {
		png_destroy_read_struct(&read_struct, &info_struct, NULL);
		return ERROR_MALLOC;
	}
	*pixeldata_size = (uint64_t) width * (uint64_t) height * 3;
	row_pointers = malloc(sizeof(uint8_t*) * height);
	if (row_pointers == NULL) {
		png_destroy_read_struct(&read_struct, &info_struct, NULL);
		free(*pixeldata);
		return ERROR_MALLOC;
	}
	for (uint64_t i = 0; i < height; i++) {
		row_pointers[i] = (*pixeldata) + (i * width * 3);
	}
	png_read_image(read_struct, row_pointers);
	png_read_end(read_struct, NULL);
	png_destroy_read_struct(&read_struct, &info_struct, NULL);
	free(row_pointers);
	return ERROR_OK;

}

#endif
