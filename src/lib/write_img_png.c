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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "impack.h"

impack_error_t impack_write_img_png(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {

	if (img_width > INT32_MAX || img_height > INT32_MAX) { // Maximum dimensions for PNG
		return ERROR_IMG_SIZE;
	}

	png_structp write_struct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (write_struct == NULL) {
		return ERROR_MALLOC;
	}
	png_infop info_struct = png_create_info_struct(write_struct);
	if (info_struct == NULL) {
		png_destroy_write_struct(&write_struct, NULL);
		return ERROR_MALLOC;
	}

	uint8_t **row_pointers = malloc(sizeof(uint8_t*) * img_height);
	if (row_pointers == NULL) {
		png_destroy_write_struct(&write_struct, NULL);
		return ERROR_MALLOC;
	}
	for (uint64_t i = 0; i < img_height; i++) {
		row_pointers[i] = pixeldata + (i * img_width * 3);
	}
	
	if (setjmp(png_jmpbuf(write_struct))) {
		png_destroy_write_struct(&write_struct, &info_struct);
		free(row_pointers);
		return ERROR_OUTPUT_IO;
	}

	png_init_io(write_struct, output_file);
	png_set_IHDR(write_struct, info_struct, img_width, img_height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_rows(write_struct, info_struct, row_pointers);
	png_write_png(write_struct, info_struct, PNG_TRANSFORM_IDENTITY, NULL);

	png_destroy_write_struct(&write_struct, &info_struct);
	free(row_pointers);
	return ERROR_OK;

}

#endif

