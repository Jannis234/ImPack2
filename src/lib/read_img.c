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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "impack.h"
#include "img.h"

impack_error_t impack_read_img(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	int format_count = 0;
	int magic_len_max = 0;
	while (impack_img_formats[format_count] != NULL) {
		if (impack_img_formats[format_count]->magic_len > magic_len_max) {
			magic_len_max = impack_img_formats[format_count]->magic_len;
		}
		format_count++;
	}
	bool magic_match[format_count];
	for (int i = 0; i < format_count; i++) {
		magic_match[i] = true;
	}
	
	uint8_t buf;
	for (int i = 0; i < magic_len_max; i++) {
		if (fread(&buf, 1, 1, input_file) != 1) {
			return ERROR_INPUT_IO;
		}
		for (int j = 0; j < format_count; j++) {
			if (i < impack_img_formats[j]->magic_len) {
				magic_match[j] &= (impack_img_formats[j]->magic[i] == buf);
			}
			if (i == impack_img_formats[j]->magic_len - 1) {
				if (magic_match[j]) {
					return impack_img_formats[j]->func_read(input_file, pixeldata, pixeldata_size);
				}
			}
		}
	}
	
	return ERROR_IMG_FORMAT_UNKNOWN;
	
}
