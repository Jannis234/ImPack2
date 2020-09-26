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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <charls/charls.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_read_img_jpegls(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	impack_error_t err = ERROR_INPUT_IMG_INVALID;
	charls_jpegls_decoder *dec = NULL;
	uint8_t *out_buf = NULL;
	
	uint8_t *buf;
	uint64_t bufsize;
	impack_error_t loadres = impack_loadfile(input_file, &buf, &bufsize, 4);
	if (loadres != ERROR_OK) {
		return loadres;
	}
	memcpy(buf, magic, 4);
	
	dec = charls_jpegls_decoder_create();
	if (dec == NULL) {
		err = ERROR_MALLOC;
		goto cleanup;
	}
	if (charls_jpegls_decoder_set_source_buffer(dec, buf, bufsize) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		err = ERROR_MALLOC;
		goto cleanup;
	}
	
	if (charls_jpegls_decoder_read_header(dec) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	charls_frame_info frame_info;
	if (charls_jpegls_decoder_get_frame_info(dec, &frame_info) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	if (frame_info.bits_per_sample != 8 || frame_info.component_count != 3) {
		goto cleanup;
	}
	charls_interleave_mode interleave;
	if (charls_jpegls_decoder_get_interleave_mode(dec, &interleave) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	
	size_t out_size;
	if (charls_jpegls_decoder_get_destination_size(dec, 0, &out_size) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	out_buf = malloc(out_size);
	if (out_buf == NULL) {
		err = ERROR_MALLOC;
		goto cleanup;
	}
	if (charls_jpegls_decoder_decode_to_buffer(dec, out_buf, out_size, 0) != CHARLS_JPEGLS_ERRC_SUCCESS) {
		goto cleanup;
	}
	charls_jpegls_decoder_destroy(dec);
	free(buf);
	
	uint64_t img_size = frame_info.width * frame_info.height;
	if (interleave == CHARLS_INTERLEAVE_MODE_NONE) {
		uint8_t *triplet = malloc(img_size * 3);
		if (triplet == NULL) {
			free(out_buf);
			return ERROR_MALLOC;
		}
		for (uint64_t i = 0; i < img_size; i++) {
			triplet[i * 3] = out_buf[i];
			triplet[(i * 3) + 1] = out_buf[i + img_size];
			triplet[(i * 3) + 2] = out_buf[i + (img_size * 2)];
		}
		free(out_buf);
		out_buf = triplet;
	}
	*pixeldata = out_buf;
	*pixeldata_size = img_size * 3;
	return ERROR_OK;
	
cleanup:
	if (dec != NULL) {
		charls_jpegls_decoder_destroy(dec);
	}
	if (buf != NULL) {
		free(buf);
	}
	if (out_buf != NULL) {
		free(out_buf);
	}
	return err;
	
}

#endif
