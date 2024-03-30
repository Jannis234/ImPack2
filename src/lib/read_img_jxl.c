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

#ifdef IMPACK_WITH_JXL

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jxl/decode.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

#define BUFSTEP 131072 // 128 KiB

impack_error_t impack_read_img_jxl(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	impack_error_t ret = ERROR_MALLOC;
	
	JxlDecoder *dec = NULL;
	*pixeldata = NULL;
	uint8_t *buf = malloc(BUFSTEP);
	if (buf == NULL) {
		goto cleanup;
	}
	uint64_t bufsize = BUFSTEP;
	uint64_t bufpos = 2;
	memcpy(buf, magic, 2);
	
	dec = JxlDecoderCreate(NULL);
	if (dec == NULL) {
		goto cleanup;
	}
	if (JxlDecoderSubscribeEvents(dec, JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
		goto cleanup;
	}
	
	JxlPixelFormat pixel_format;
	pixel_format.num_channels = 3;
	pixel_format.data_type = JXL_TYPE_UINT8;
	pixel_format.endianness = JXL_NATIVE_ENDIAN;
	pixel_format.align = 0;
	
	while (true) {
		size_t bytes_read = fread(buf + bufpos, 1, bufsize - bufpos, input_file);
		if (bytes_read != bufsize - bufpos) {
			if (!feof(input_file)) {
				ret = ERROR_INPUT_IO;
				goto cleanup;
			}
		}
		bufpos += bytes_read;
		
		if (JxlDecoderSetInput(dec, buf, bufpos) != JXL_DEC_SUCCESS) {
			goto cleanup;
		}
		JxlDecoderStatus status = JxlDecoderProcessInput(dec);
		if (status == JXL_DEC_ERROR) {
			ret = ERROR_INPUT_IMG_INVALID;
			goto cleanup;
		} else if (status == JXL_DEC_NEED_MORE_INPUT) {
			if (feof(input_file)) { // End of file but decoder wants more (file truncated?)
				ret = ERROR_INPUT_IMG_INVALID;
				goto cleanup;
			}
			
			if (bufpos == bufsize) { // Make the buffer larger if the decoder needs it
				uint8_t *newbuf = realloc(buf, bufsize + BUFSTEP);
				if (newbuf == NULL) {
					goto cleanup;
				}
				buf = newbuf;
				bufsize += BUFSTEP;
			}
		} else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
			if (JxlDecoderImageOutBufferSize(dec, &pixel_format, pixeldata_size) != JXL_DEC_SUCCESS) {
				ret = ERROR_INPUT_IMG_INVALID;
				goto cleanup;
			}
			*pixeldata = malloc(*pixeldata_size);
			if (*pixeldata == NULL) {
				goto cleanup;
			}
			if (JxlDecoderSetImageOutBuffer(dec, &pixel_format, *pixeldata, *pixeldata_size) != JXL_DEC_SUCCESS) {
				ret = ERROR_INPUT_IMG_INVALID;
				goto cleanup;
			}
		} else if (status == JXL_DEC_FULL_IMAGE) {
			if (*pixeldata == NULL) { // Decoder says it's done but never requested a buffer: Something went seriously wrong
				ret = ERROR_INPUT_IMG_INVALID;
				goto cleanup;
			}
			break;
		}
		
		size_t remaining = JxlDecoderReleaseInput(dec);
		if (remaining > 0) {
			if (remaining != bufpos) { // If the decoder consumed data, move the remaining bytes to the beginning of the buffer
				memmove(buf, buf + bufpos - remaining, remaining);
			}
			bufpos = remaining;
		} else {
			bufpos = 0;
		}
	}
	
	ret = ERROR_OK;
	
cleanup:
	if (dec != NULL) {
		JxlDecoderDestroy(dec);
	}
	if (buf != NULL) {
		free(buf);
	}
	if (*pixeldata != NULL && ret != ERROR_OK) {
		free(*pixeldata);
	}
	return ret;
	
}

#endif
