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

#ifdef IMPACK_WITH_BMP

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libnsbmp.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

void* impack_bmp_create(int width, int height, unsigned int state) {
	
	return malloc(width * height * 4);
	
}

unsigned char* impack_bmp_get_buffer(void *bitmap) {
	
	return bitmap;
	
}

void impack_bmp_destroy(void *bitmap) {
	
	free(bitmap);
	
}

impack_error_t impack_read_img_bmp(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t *buf;
	uint64_t bufsize;
	impack_error_t loadres = impack_loadfile(input_file, &buf, &bufsize, 2);
	if (loadres != ERROR_OK) {
		return loadres;
	}
	memcpy(buf, magic, 2);
	
	bmp_bitmap_callback_vt callbacks = {
		impack_bmp_create,
		impack_bmp_destroy,
		impack_bmp_get_buffer
	};
	bmp_image img;
	bmp_create(&img, &callbacks);
	if (bmp_analyse(&img, bufsize, buf) != BMP_OK) {
		free(buf);
		bmp_finalise(&img);
		return ERROR_INPUT_IMG_INVALID;
	}
	bmp_result res = bmp_decode(&img);
	if (res != BMP_OK) {
		free(buf);
		bmp_finalise(&img);
		if (res == BMP_INSUFFICIENT_MEMORY) {
			return ERROR_MALLOC;
		} else {
			return ERROR_INPUT_IMG_INVALID;
		}
	}
	free(buf);
	
	*pixeldata_size = img.width * img.height * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		bmp_finalise(&img);
		return ERROR_MALLOC;
	}
	uint64_t pixeldata_index = 0;
	uint8_t *img_data = (uint8_t*) img.bitmap;
	for (uint64_t i = 0; i < (img.width * img.height * 4); i += 4) {
		(*pixeldata)[pixeldata_index] = img_data[i];
		(*pixeldata)[pixeldata_index + 1] = img_data[i + 1];
		(*pixeldata)[pixeldata_index + 2] = img_data[i + 2];
		pixeldata_index += 3;
	}
	bmp_finalise(&img);
	return ERROR_OK;
	
}

#endif
