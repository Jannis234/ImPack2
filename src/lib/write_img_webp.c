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

#ifdef IMPACK_WITH_WEBP

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <webp/encode.h>
#include "impack.h"

impack_error_t impack_write_img_webp(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > 16383 || img_height > 16383) {
		return ERROR_IMG_SIZE;
	}
	
	uint8_t *img;
	size_t img_size = WebPEncodeLosslessRGB(pixeldata, img_width, img_height, img_width * 3, &img);
	if (img_size == 0) {
		return ERROR_MALLOC;
	}
	if (fwrite(img, 1, img_size, output_file) != img_size) {
		WebPFree(img);
		return ERROR_OUTPUT_IO;
	}
	fflush(output_file);
	WebPFree(img);
	return ERROR_OK;
	
}

#endif
