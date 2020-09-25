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

#ifdef IMPACK_WITH_HEIF

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libheif/heif.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

#if (LIBHEIF_NUMERIC_VERSION < ((1 << 24) | (9 << 16)))
// Libheif 1.8.0 and older have a bug that can cause an abort() when writing lossless images with YUV444 chroma
#warning "Building ImPack2 using libheif older than version 1.9.0 can cause problems, it is recommended to update libheif and recompile ImPack2!"
#endif

struct heif_error impack_heif_write(struct heif_context* ctx, const void* data, size_t len, void *arg) {
	
	struct heif_error res;
	FILE *output_file = (FILE*) arg;
	if (fwrite(data, 1, len, output_file) == len) {
		res.code = heif_error_Ok;
	} else {
		res.code = heif_error_Encoding_error;
	}
	return res;
	
}

impack_error_t impack_write_img_heif(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > INT32_MAX || img_height > INT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	struct heif_context *ctx = NULL;
	struct heif_encoder *enc = NULL;
	struct heif_image *img = NULL;
	impack_error_t ret = ERROR_MALLOC;
	
	ctx = heif_context_alloc();
	if (ctx == NULL) {
		goto cleanup;
	}
	struct heif_error res = heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &enc);
	if (res.code != heif_error_Ok) {
		goto cleanup;
	}
	
	res = heif_image_create(img_width, img_height, heif_colorspace_RGB, heif_chroma_interleaved_RGB, &img);
	if (res.code != heif_error_Ok) {
		goto cleanup;
	}
	res = heif_image_add_plane(img, heif_channel_interleaved, img_width, img_height, 8);
	if (res.code != heif_error_Ok) {
		goto cleanup;
	}
	int stride;
	uint8_t *plane = heif_image_get_plane(img, heif_channel_interleaved, &stride);
	for (uint64_t y = 0; y < img_height; y++) {
		memcpy(plane + (stride * y), pixeldata + (y * img_width * 3), img_width * 3);
	}
	
	struct heif_color_profile_nclx prof;
	prof.version = 1;
	prof.matrix_coefficients = heif_matrix_coefficients_RGB_GBR;
	prof.transfer_characteristics = heif_transfer_characteristic_unspecified;
	prof.color_primaries = heif_color_primaries_unspecified;
	prof.full_range_flag = true;
	heif_image_set_nclx_color_profile(img, &prof);
	heif_encoder_set_lossless(enc, true);
	heif_encoder_set_parameter_string(enc, "chroma", "444");
	
	res = heif_context_encode_image(ctx, img, enc, NULL, NULL);
	if (res.code != heif_error_Ok) {
		goto cleanup;
	}
	heif_image_release(img);
	img = NULL;
	heif_encoder_release(enc);
	enc = NULL;
	
	struct heif_writer writer;
	writer.writer_api_version = 1;
	writer.write = impack_heif_write;
	res = heif_context_write(ctx, &writer, output_file);
	heif_context_free(ctx);
	if (res.code == heif_error_Ok) {
		return ERROR_OK;
	} else {
		return ERROR_OUTPUT_IO;
	}
	
cleanup:
	if (img != NULL) {
		heif_image_release(img);
	}
	if (enc != NULL) {
		heif_encoder_release(enc);
	}
	if (ctx != NULL) {
		heif_context_free(ctx);
	}
	return ret;
	
}

#endif
