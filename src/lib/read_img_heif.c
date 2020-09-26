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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libheif/heif.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_read_img_heif(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	struct heif_context *ctx = NULL;
	struct heif_image_handle *handle = NULL;
	struct heif_image *img = NULL;
	uint8_t *buf = NULL;
	impack_error_t ret = ERROR_MALLOC;
	
	uint64_t bufsize;
	impack_error_t loadres = impack_loadfile(input_file, &buf, &bufsize, 12);
	if (loadres != ERROR_OK) {
		return loadres;
	}
	memcpy(buf, magic, 12);
	
	ctx = heif_context_alloc();
	if (ctx == NULL) {
		goto cleanup;
	}
	struct heif_error res = heif_context_read_from_memory_without_copy(ctx, buf, bufsize, NULL);
	if (res.code != heif_error_Ok) {
		if (res.code != heif_error_Memory_allocation_error) {
			ret = ERROR_INPUT_IMG_INVALID;
		}
		goto cleanup;
	}
	res = heif_context_get_primary_image_handle(ctx, &handle);
	if (res.code != heif_error_Ok) {
		goto cleanup;
	}
	res = heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, NULL);
	if (res.code != heif_error_Ok) {
		if (res.code != heif_error_Memory_allocation_error) {
			ret = ERROR_INPUT_IMG_INVALID;
		}
		goto cleanup;
	}
	heif_image_handle_release(handle);
	handle = NULL;
	heif_context_free(ctx);
	ctx = NULL;
	free(buf);
	buf = NULL;
	
	int width = heif_image_get_width(img, heif_chroma_interleaved_RGB);
	int height = heif_image_get_height(img, heif_chroma_interleaved_RGB);
	*pixeldata_size = (width * height * 3);
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		goto cleanup;
	}
	int stride;
	const uint8_t *outbuf = heif_image_get_plane_readonly(img, heif_chroma_interleaved_RGB, &stride);
	for (int y = 0; y < height; y++) {
		memcpy(*pixeldata + (y * width * 3), outbuf + (y * stride), width * 3);
	}
	heif_image_release(img);
	return ERROR_OK;
	
cleanup:
	if (handle != NULL) {
		heif_image_handle_release(handle);
	}
	if (ctx != NULL) {
		heif_context_free(ctx);
	}
	if (buf != NULL) {
		free(buf);
	}
	if (img != NULL) {
		heif_image_release(img);
	}
	return ret;
	
}

#endif
