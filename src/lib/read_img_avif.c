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

#ifdef IMPACK_WITH_AVIF

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avif/avif.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

#if (AVIF_VERSION < 802)
#define LIBAVIF_COMPAT_081
#endif

impack_error_t impack_read_img_avif(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	avifDecoder *dec = NULL;
	uint8_t *buf = NULL;
	impack_error_t ret = ERROR_MALLOC;
	
	uint64_t bufsize;
	impack_error_t loadres = impack_loadfile(input_file, &buf, &bufsize, 12);
	if (loadres != ERROR_OK) {
		return loadres;
	}
	memcpy(buf, magic, 12);
#ifdef LIBAVIF_COMPAT_081
	avifROData in;
	in.data = buf;
	in.size = bufsize;
#endif
	
	dec = avifDecoderCreate();
	if (dec == NULL) {
		goto cleanup;
	}
#ifndef LIBAVIF_COMPAT_081
	if (avifDecoderSetIOMemory(dec, buf, bufsize) != AVIF_RESULT_OK) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
#endif
#ifdef LIBAVIF_COMPAT_081
	if (avifDecoderParse(dec, &in) != AVIF_RESULT_OK) {
#else
	if (avifDecoderParse(dec) != AVIF_RESULT_OK) {
#endif
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	if (avifDecoderNextImage(dec) != AVIF_RESULT_OK) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	
	*pixeldata_size = dec->image->width * dec->image->height * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		goto cleanup;
	}
	
	avifRGBImage rgb;
	avifRGBImageSetDefaults(&rgb, dec->image);
	rgb.format = AVIF_RGB_FORMAT_RGB;
	rgb.depth = 8;
	rgb.chromaUpsampling = AVIF_CHROMA_UPSAMPLING_NEAREST; // Chroma conversion normally means a non-lossless image anyway
	rgb.ignoreAlpha = true;
	rgb.pixels = *pixeldata;
	rgb.rowBytes = dec->image->width * 3;
	if (avifImageYUVToRGB(dec->image, &rgb) != AVIF_RESULT_OK) {
		ret = ERROR_INPUT_IMG_INVALID;
		free(*pixeldata);
		goto cleanup;
	}
	
	avifDecoderDestroy(dec);
	free(buf);
	return ERROR_OK;
	
cleanup:
	if (dec != NULL) {
		avifDecoderDestroy(dec);
	}
	if (buf != NULL) {
		free(buf);
	}
	return ret;
	
}

#endif
