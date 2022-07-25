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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <jxl/encode.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

#define BUFSIZE 131072 // 128 KiB

impack_error_t impack_write_img_jxl(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	impack_error_t ret = ERROR_MALLOC;
	if (img_width > UINT32_MAX || img_height > UINT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	JxlEncoder *enc = JxlEncoderCreate(NULL);
	uint8_t *buf = NULL;
	if (enc == NULL) {
		goto cleanup;
	}
	
	JxlBasicInfo basic_info;
	JxlEncoderInitBasicInfo(&basic_info);
	basic_info.have_container = JXL_FALSE;
	basic_info.xsize = img_width;
	basic_info.ysize = img_height;
	basic_info.bits_per_sample = 8;
	basic_info.exponent_bits_per_sample = 0;
	basic_info.num_color_channels = 3;
	basic_info.num_extra_channels = 0;
	basic_info.uses_original_profile = JXL_TRUE;
	if (JxlEncoderSetBasicInfo(enc, &basic_info) != JXL_ENC_SUCCESS) {
		goto cleanup;
	}
	
	JxlColorEncoding color_encoding;
	JxlColorEncodingSetToSRGB(&color_encoding, JXL_FALSE);
	if (JxlEncoderSetColorEncoding(enc, &color_encoding) != JXL_ENC_SUCCESS) {
		goto cleanup;
	}
	
	JxlEncoderFrameSettings *frame_settings = JxlEncoderFrameSettingsCreate(enc, NULL);
	if (frame_settings == NULL) {
		goto cleanup;
	}
	//JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_COLOR_TRANSFORM, 1);
	JxlEncoderSetFrameLossless(frame_settings, JXL_TRUE);
	
	JxlPixelFormat pixel_format;
	pixel_format.num_channels = 3;
	pixel_format.data_type = JXL_TYPE_UINT8;
	pixel_format.endianness = JXL_NATIVE_ENDIAN;
	pixel_format.align = 0;
	
	if (JxlEncoderAddImageFrame(frame_settings, &pixel_format, pixeldata, img_width * img_height * 3) != JXL_ENC_SUCCESS) {
		goto cleanup;
	}
	JxlEncoderCloseInput(enc);
	
	JxlEncoderStatus status;
	buf = malloc(BUFSIZE);
	if (buf == NULL) {
		goto cleanup;
	}
	ret = ERROR_OUTPUT_IO;
	do {
		uint8_t *buf2 = buf;
		size_t avail_out = BUFSIZE;
		status = JxlEncoderProcessOutput(enc, &buf2, &avail_out);
		if (status == JXL_ENC_ERROR) {
			goto cleanup;
		}
		if (fwrite(buf, 1, BUFSIZE - avail_out, output_file) != BUFSIZE - avail_out) {
			goto cleanup;
		}
	} while (status == JXL_ENC_NEED_MORE_OUTPUT);
	
	ret = ERROR_OK;
	
cleanup:
	if (enc != NULL) {
		JxlEncoderDestroy(enc);
	}
	if (buf != NULL) {
		free(buf);
	}
	return ret;
	
}

#endif
