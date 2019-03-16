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

impack_error_t impack_read_img_tiff(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size, bool le) {
	
	impack_error_t res = impack_tiff_init_read(input_file, le);
	if (res != ERROR_OK) {
		return res;
	}
	impack_error_t ret = ERROR_MALLOC;
	uint32_t *rgba = NULL;
	TIFF *img = TIFFClientOpen("", "rm", NULL, impack_tiff_read, impack_tiff_write, impack_tiff_seek, impack_tiff_close, impack_tiff_size, impack_tiff_map, impack_tiff_unmap);
	if (img == NULL) {
		goto cleanup;
	}
	
	uint32_t width, height;
	if (TIFFGetField(img, TIFFTAG_IMAGEWIDTH, &width) != 1 || TIFFGetField(img, TIFFTAG_IMAGELENGTH, &height) != 1) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	rgba = malloc(width * height * 4);
	if (rgba == NULL) {
		goto cleanup;
	}
	if (TIFFReadRGBAImageOriented(img, width, height, rgba, ORIENTATION_TOPLEFT, 1) != 1) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	TIFFClose(img);
	impack_tiff_finish_read();
	
	*pixeldata = malloc(width * height * 3);
	if (*pixeldata == NULL) {
		free(rgba);
		return ERROR_MALLOC;
	}
	*pixeldata_size = width * height * 3;
	uint64_t index_pixeldata = 0;
	for (uint64_t index_rgba = 0; index_rgba < width * height; index_rgba++) {
		(*pixeldata)[index_pixeldata] = TIFFGetR(rgba[index_rgba]);
		(*pixeldata)[index_pixeldata + 1] = TIFFGetG(rgba[index_rgba]);
		(*pixeldata)[index_pixeldata + 2] = TIFFGetB(rgba[index_rgba]);
		index_pixeldata += 3;
	}
	free(rgba);
	return ERROR_OK;
	
cleanup:
	TIFFClose(img);
	impack_tiff_finish_read();
	free(rgba);
	return ret;
	
}

#endif
