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

#ifdef IMPACK_WITH_JPEGLS

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <charls/charls.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_write_img_jpegls(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > UINT32_MAX || img_height > UINT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	charls_jpegls_encoder *enc = charls_jpegls_encoder_create();
	uint8_t *out_buf = NULL;
	if (enc == NULL) {
		return ERROR_MALLOC;
	}
	
	charls_frame_info frame_info;
	frame_info.bits_per_sample = 8;
	frame_info.component_count = 3;
	frame_info.width = img_width;
	frame_info.height = img_height;
	if (charls_jpegls_encoder_set_frame_info(enc, &frame_info) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	if (charls_jpegls_encoder_set_interleave_mode(enc, CHARLS_INTERLEAVE_MODE_SAMPLE) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	if (charls_jpegls_encoder_set_near_lossless(enc, 0) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	size_t out_size;
	if (charls_jpegls_encoder_get_estimated_destination_size(enc, &out_size) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	out_buf = malloc(out_size);
	if (out_buf == NULL) {
		goto cleanup;
	}
	if (charls_jpegls_encoder_set_destination_buffer(enc, out_buf, out_size) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	
	uint64_t img_size = img_width * img_height;
	if (charls_jpegls_encoder_encode_from_buffer(enc, pixeldata, img_size * 3, 0) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	if (charls_jpegls_encoder_get_bytes_written(enc, &out_size) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	
	charls_jpegls_encoder_destroy(enc);
	if (fwrite(out_buf, 1, out_size, output_file) != out_size) {
		free(out_buf);
		return ERROR_OUTPUT_IO;
	}
	free(out_buf);
	return ERROR_OK;
	
cleanup:
	charls_jpegls_encoder_destroy(enc);
	if (out_buf != NULL) {
		free(out_buf);
	}
	return ERROR_MALLOC;
	
}

#endif
