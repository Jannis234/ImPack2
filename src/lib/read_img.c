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
#include "config.h"
#include "impack.h"
#include "impack_internal.h"

#define MAGIC_PNG { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A }

impack_error_t impack_read_img(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t magic_png[] = MAGIC_PNG;
	
	uint8_t buf[8];
	if (fread(buf, 1, 8, input_file) != 8) {
		return ERROR_INPUT_IO;
	}

	bool ispng = true;
	for (int i = 0; i < 8; i++) {
		ispng &= (buf[i] == magic_png[i]);
	}

	if (ispng) {
#ifdef IMPACK_WITH_PNG
		return impack_read_img_png(input_file, pixeldata, pixeldata_size);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}

	return ERROR_IMG_FORMAT_UNKNOWN;
	
}
