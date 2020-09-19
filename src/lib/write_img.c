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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "impack.h"
#include "img.h"

impack_error_t impack_write_img(char *output_path, FILE *output_file, uint8_t **pixeldata, uint64_t pixeldata_size, uint64_t pixeldata_pos, uint64_t img_width, uint64_t img_height, impack_img_format_t format) {
	
	uint64_t width = img_width;
	uint64_t height = img_height;
	if (width == 0 && height == 0) { // Auto-select image size, should result in a nearly-square image
		width = 1;
		height = 1;
		while ((width * height * 3) < pixeldata_pos) {
			width *= 2;
			height *= 2;
		}
		while ((width * height * 3) > pixeldata_pos) {
			width--;
			height--;
		}
		while ((width * height * 3) < pixeldata_pos) {
			width++;
		}
	} else if (width == 0) { // One dimension selected by user, auto-select the other one
		width = (pixeldata_pos / 3) / height;
		while (width * height * 3 < pixeldata_pos) {
			width++;
		}
	} else if (height == 0) {
		height = (pixeldata_pos / 3) / width;
		while (width * height * 3 < pixeldata_pos) {
			height++;
		}
	} else { // User selected both dimension, check if the data fits
		if (width * height * 3 < pixeldata_pos) {
			return ERROR_IMG_TOO_SMALL;
		}
	}
	
	if (pixeldata_size < (width * height * 3)) { // Need more pixels to fill in unused space
		uint8_t *newbuf = realloc(*pixeldata, width * height * 3);
		if (newbuf == NULL) {
			return ERROR_MALLOC;
		}
		*pixeldata = newbuf;
	}
	memset((*pixeldata) + pixeldata_pos, 0, pixeldata_size - pixeldata_pos);
	
	if (format == FORMAT_AUTO) {
		size_t pathlen = strlen(output_path);
		char *extstart = NULL;
		for (int i = pathlen - 1; i >= 0; i--) {
			if (output_path[i] == '.') {
				extstart = output_path + i + 1;
				break;
			}
		}
		if (extstart != NULL) {
			format = impack_select_img_format(extstart, true);
		}
		if (extstart == NULL || format == FORMAT_AUTO) {
			format = impack_default_img_format();
		}
	}
	
	int current = 0;
	while (impack_img_formats[current] != NULL) {
		if (impack_img_formats[current]->id == format) {
			return impack_img_formats[current]->func_write(output_file, *pixeldata, pixeldata_size, width, height);
		}
		current++;
	}
	abort(); // Requested a format that isn't compiled in
	
}
