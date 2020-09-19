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

#ifdef IMPACK_WITH_JXR

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <JXRGlue.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_read_img_jxr(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t *buf;
	uint64_t bufsize;
	impack_error_t res = impack_loadfile(input_file, &buf, &bufsize, 8);
	if (res != ERROR_OK) {
		return res;
	}
	memcpy(buf, impack_magic_jxr, 8);
	
	impack_error_t ret = ERROR_MALLOC;
	PKImageDecode *decoder = NULL;
	PKFormatConverter *converter = NULL;
	struct WMPStream *strm = NULL;
	*pixeldata = NULL;
	if (CreateWS_Memory(&strm, buf, bufsize) != WMP_errSuccess) {
		goto cleanup;
	}
	if (PKImageDecode_Create_WMP(&decoder) != WMP_errSuccess) {
		goto cleanup;
	}
	if (decoder->Initialize(decoder, strm) != WMP_errSuccess) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	decoder->WMP.wmiSCP.uAlphaMode = 0;
	int32_t width, height;
	decoder->GetSize(decoder, &width, &height);
	*pixeldata_size = width * height * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		goto cleanup;
	}
	
	PKRect rect;
	rect.X = 0;
	rect.Y = 0;
	rect.Width = width;
	rect.Height = height;
	if (PKCodecFactory_CreateFormatConverter(&converter) != WMP_errSuccess) {
		goto cleanup;
	}
	if (converter->Initialize(converter, decoder, NULL, GUID_PKPixelFormat24bppRGB) != WMP_errSuccess) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	if (converter->Copy(converter, &rect, *pixeldata, width * 3) != WMP_errSuccess) {
		ret = ERROR_INPUT_IMG_INVALID;
		goto cleanup;
	}
	converter->Release(&converter);
	decoder->Release(&decoder);
	CloseWS_Memory(&strm);
	free(buf);
	return ERROR_OK;
	
cleanup:
	if (decoder != NULL) {
		decoder->Release(&decoder);
	}
	if (strm != NULL) {
		CloseWS_Memory(&strm);
	}
	if (converter != NULL) {
		converter->Release(&converter);
	}
	if (buf != NULL) {
		free(buf);
	}
	if (*pixeldata != NULL) {
		free(*pixeldata);
	}
	return ret;
	
}

#endif
