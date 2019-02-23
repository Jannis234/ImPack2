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

#ifdef IMPACK_WITH_BMP

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

// libnsbmp only does reading, but writing BMP is simple enough
impack_error_t impack_write_img_bmp(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > INT32_MAX || img_height > INT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	uint32_t row_padding = 4 - ((img_width * 3) % 4);
	if (row_padding == 4) {
		row_padding = 0;
	}
	uint64_t filesize = (img_width * img_height * 3) + (img_height * row_padding) + 54;
	if (filesize > UINT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	uint8_t header[54];
	uint8_t magic[] = IMPACK_MAGIC_BMP;
	memcpy(header, magic, 2);
	uint32_t filesize_endian = impack_endian32_le((uint32_t) filesize);
	memcpy(header + 2, (uint8_t*) &filesize_endian, 4);
	memset(header + 6, 0, 4); // "Reserved" space
	uint32_t img_offset = impack_endian32_le(54);
	memcpy(header + 10, (uint8_t*) &img_offset, 4);
	uint32_t headersize = impack_endian32_le(40);
	memcpy(header + 14, (uint8_t*) &headersize, 4);
	int32_t width_endian = impack_endian32_le((int32_t) img_width);
	memcpy(header + 18, (uint8_t*) &width_endian, 4);
	int32_t height_endian = impack_endian32_le((int32_t) img_height);
	memcpy(header + 22, (uint8_t*) &height_endian, 4);
	uint16_t colorplanes = impack_endian16_le(1);
	memcpy(header + 26, (uint8_t*) &colorplanes, 2);
	uint16_t bpp = impack_endian16_le(24);
	memcpy(header + 28, (uint8_t*) &bpp, 2);
	memset(header + 30, 0, 4); // Compression type (=0)
	uint32_t image_size = impack_endian32_le((img_width * img_height * 3) + (img_height * row_padding));
	memcpy(header + 34, (uint8_t*) &image_size, 4);
	memset(header + 38, 0, 8); // Physical image size, not known/used
	memset(header + 46, 0, 4); // Colors in palette, not used
	memset(header + 50, 0, 4); // "Important" color, not used
	if (fwrite(header, 1, 54, output_file) != 54) {
		return ERROR_OUTPUT_IO;
	}
	
	uint8_t padding[] = { 0, 0, 0 }; // May need up to 3 bytes
	for (int32_t y = img_height - 1; y >= 0; y--) {
		for (int32_t x = 0; x < img_width; x++) {
			uint8_t *pixel_rgb = pixeldata + (y * img_width * 3) + (x * 3);
			uint8_t pixel_bgr[3] = { pixel_rgb[2], pixel_rgb[1], pixel_rgb[0] };
			if (fwrite(pixel_bgr, 1, 3, output_file) != 3) {
				return ERROR_OUTPUT_IO;
			}
		}
		if (fwrite(padding, 1, row_padding, output_file) != row_padding) {
			return ERROR_OUTPUT_IO;
		}
	}
	return ERROR_OK;
	
}

#endif
