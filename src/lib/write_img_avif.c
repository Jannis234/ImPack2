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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <avif/avif.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_write_img_avif(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > INT32_MAX || img_height > INT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	avifImage *img = NULL;
	avifEncoder *enc = NULL;
	impack_error_t ret = ERROR_MALLOC;
	
	img = avifImageCreate(img_width, img_height, 8, AVIF_PIXEL_FORMAT_YUV444);
	if (img == NULL) {
		goto cleanup;
	}
	img->matrixCoefficients = AVIF_MATRIX_COEFFICIENTS_IDENTITY;
	img->colorPrimaries = AVIF_COLOR_PRIMARIES_UNSPECIFIED;
	img->transferCharacteristics = AVIF_TRANSFER_CHARACTERISTICS_UNSPECIFIED;
	img->yuvRange = AVIF_RANGE_FULL;
	
	avifRGBImage rgb;
	avifRGBImageSetDefaults(&rgb, img);
	rgb.depth = 8;
	rgb.format = AVIF_RGB_FORMAT_RGB;
	rgb.pixels = pixeldata;
	rgb.rowBytes = img_width * 3;
	if (avifImageRGBToYUV(img, &rgb) != AVIF_RESULT_OK) {
		goto cleanup;
	}
	
	enc = avifEncoderCreate();
	if (enc == NULL) {
		goto cleanup;
	}
	enc->minQuantizer = AVIF_QUANTIZER_LOSSLESS;
	enc->maxQuantizer = AVIF_QUANTIZER_LOSSLESS;
	avifRWData out = AVIF_DATA_EMPTY;
	if (avifEncoderWrite(enc, img, &out) != AVIF_RESULT_OK) {
		goto cleanup;
	}
	avifEncoderDestroy(enc);
	enc = NULL;
	avifImageDestroy(img);
	img = NULL;
	
	if (fwrite(out.data, 1, out.size, output_file) != out.size) {
		avifRWDataFree(&out);
		return ERROR_OUTPUT_IO;
	}
	avifRWDataFree(&out);
	return ERROR_OK;
	
cleanup:
	if (enc != NULL) {
		avifEncoderDestroy(enc);
	}
	if (img != NULL) {
		avifImageDestroy(img);
	}
	return ret;
	
}

#endif
