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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <webp/decode.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_read_img_webp(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t *buf = malloc(12);
	if (buf == NULL) {
		return ERROR_MALLOC;
	}
	impack_error_t ret = ERROR_INPUT_IMG_INVALID;
	*pixeldata = NULL;
	memcpy(buf, magic, 4);
	if (fread(buf + 4, 1, 8, input_file) != 8) {
		ret = ERROR_INPUT_IO;
		goto cleanup;
	}
	
	uint32_t filesize;
	memcpy((uint8_t*) &filesize, buf + 4, 4);
	filesize = impack_endian32_le(filesize);
	if (filesize > UINT32_MAX - 10) {
		goto cleanup;
	}
	uint8_t *newbuf = realloc(buf, filesize + 8);
	if (newbuf == NULL) {
		ret = ERROR_MALLOC;
		goto cleanup;
	}
	buf = newbuf;
	
	if (fread(buf + 12, 1, filesize - 4, input_file) != filesize - 4) {
		ret = ERROR_INPUT_IO;
		goto cleanup;
	}
	int32_t width, height;
	if (!WebPGetInfo(buf, filesize + 8, &width, &height)) {
		goto cleanup;
	}
	*pixeldata_size = width * height * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		ret = ERROR_MALLOC;
		goto cleanup;
	}
	if (WebPDecodeRGBInto(buf, filesize + 8, *pixeldata, *pixeldata_size, width * 3) == NULL) {
		goto cleanup;
	}
	free(buf);
	return ERROR_OK;
	
cleanup:
	free(buf);
	if (*pixeldata != NULL) {
		free(*pixeldata);
	}
	return ret;
	
}

#endif
