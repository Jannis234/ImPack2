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

#ifdef IMPACK_WITH_TIFF

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "impack.h"
#include "libtiff_io.h"
#include <tiffio.h>

impack_error_t impack_write_img_tiff(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width >= UINT32_MAX || img_height >= UINT32_MAX || pixeldata_size >= UINT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	if (!impack_tiff_init_write()) {
		return ERROR_MALLOC;
	}
	TIFF *img = TIFFClientOpen("", "wm", NULL, impack_tiff_read, impack_tiff_write, impack_tiff_seek, impack_tiff_close, impack_tiff_size, impack_tiff_map, impack_tiff_unmap);
	if (img == NULL) {
		return ERROR_MALLOC;
	}
	
	bool ok = true;
	ok &= (TIFFSetField(img, TIFFTAG_IMAGEWIDTH, img_width) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_IMAGELENGTH, img_height) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_SAMPLESPERPIXEL, 3) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_BITSPERSAMPLE, 8) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) == 1);
	ok &= (TIFFSetField(img, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(img, img_width * 3)) == 1);
	if (!ok) {
		TIFFClose(img);
		return ERROR_MALLOC;
	}
	
	for (uint32_t i = 0; i < img_height; i++) {
		if (TIFFWriteScanline(img, pixeldata + (i * img_width * 3), i, 0) != 1) {
			TIFFClose(img);
			return ERROR_MALLOC;
		}
	}
	
	TIFFClose(img);
	if (!impack_tiff_finish_write(output_file)) {
		return ERROR_OUTPUT_IO;
	}
	return ERROR_OK;
	
}

#endif
