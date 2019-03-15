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
#include <stdint.h>
#include <stdio.h>
#include <flif.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_write_img_flif(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > UINT32_MAX || img_height > UINT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	FLIF_IMAGE *img = flif_import_image_RGB(img_width, img_height, pixeldata, img_width * 3);
	if (img == NULL) {
		return ERROR_MALLOC;
	}
	FLIF_ENCODER *encoder = flif_create_encoder();
	if (encoder == NULL) {
		flif_destroy_image(img);
		return ERROR_MALLOC;
	}
	flif_encoder_add_image_move(encoder, img);
	flif_destroy_image(img);
	uint8_t *imgdata;
	size_t imgdata_len;
	if (flif_encoder_encode_memory(encoder, (void**) &imgdata, &imgdata_len) == 0) {
		flif_destroy_encoder(encoder);
		return ERROR_MALLOC;
	}
	flif_destroy_encoder(encoder);
	
	if (fwrite(imgdata, 1, imgdata_len, output_file) != imgdata_len) {
		flif_free_memory(imgdata);
		return ERROR_OUTPUT_IO;
	}
	fflush(output_file);
	flif_free_memory(imgdata);
	return ERROR_OK;
	
}

#endif
