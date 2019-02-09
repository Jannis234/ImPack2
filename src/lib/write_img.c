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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "impack.h"
#include "impack_internal.h"

impack_error_t impack_write_img(char *output_path, FILE *output_file, uint8_t **pixeldata, uint64_t pixeldata_size, uint64_t pixeldata_pos) {

	pixeldata_pos--;
	// TODO: Allow selecting custom image size
	uint64_t width = 1;
	uint64_t height = 1;
	// Auto-select image size, should result in a nearly-square image
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

	if (pixeldata_size < (width * height * 3)) { // Need more pixels to fill in unused space
		uint8_t *newbuf = realloc(*pixeldata, width * height * 3);
		if (newbuf == NULL) {
			return ERROR_MALLOC;
		}
		*pixeldata = newbuf;
	}
	memset((*pixeldata) + pixeldata_pos, 0, pixeldata_size - pixeldata_pos);

	size_t pathlen = strlen(output_path);
	// Try to select format based on file extension
#ifdef IMPACK_WITH_PNG
	if (pathlen >= 4) {
		if (output_path[pathlen - 4] == '.' && \
			(output_path[pathlen - 3] == 'p' || output_path[pathlen - 3] == 'P') && \
			(output_path[pathlen - 2] == 'n' || output_path[pathlen - 2] == 'N') && \
			(output_path[pathlen - 1] == 'g' || output_path[pathlen - 1] == 'G')) {
			return impack_write_img_png(output_file, *pixeldata, pixeldata_size, width, height);
		}
	}
#endif

	// Unknown extension -> Try to find a default based on what's compiled in
#ifdef IMPACK_WITH_PNG
	return impack_write_img_png(output_file, *pixeldata, pixeldata_size, width, height);
#else
#error "No image formats selected in config_build.mak"
#endif
	
}

