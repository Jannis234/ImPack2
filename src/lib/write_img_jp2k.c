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

#ifdef IMPACK_WITH_JP2K

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <openjpeg.h>
#include "impack.h"
#include "impack_internal.h"
#include "openjpeg_io.h"

impack_error_t impack_write_img_jp2k(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > UINT32_MAX || img_height > UINT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	opj_image_cmptparm_t cmptparam[3];
	memset(cmptparam, 0, sizeof(opj_image_cmptparm_t) * 3);
	for (int i = 0; i < 3; i++) {
		cmptparam[i].prec = 8;
		cmptparam[i].sgnd = 0;
		cmptparam[i].dx = 1;
		cmptparam[i].dy = 1;
		cmptparam[i].w = img_width;
		cmptparam[i].h = img_height;
	}
	opj_image_t *img = opj_image_create(3, cmptparam, OPJ_CLRSPC_SRGB);
	if (img == NULL) {
		return ERROR_MALLOC;
	}
	img->x0 = 0;
	img->y0 = 0;
	img->x1 = img_width;
	img->y1 = img_height;
	for (uint64_t i = 0; i < img_width * img_height; i++) {
		img->comps[0].data[i] = pixeldata[i * 3];
		img->comps[1].data[i] = pixeldata[(i * 3) + 1];
		img->comps[2].data[i] = pixeldata[(i * 3) + 2];
	}
	
	opj_cparameters_t params;
	opj_set_default_encoder_parameters(&params);
	params.numresolution = 1;
	impack_error_t ret = ERROR_OUTPUT_IO;
	opj_stream_t *strm = NULL;
	opj_codec_t *codec = opj_create_compress(OPJ_CODEC_JP2);
	if (codec == NULL) {
		ret = ERROR_MALLOC;
		goto cleanup;
	}
	if (!opj_setup_encoder(codec, &params, img)) {
		ret = ERROR_MALLOC;
		goto cleanup;
	}
	
	strm = impack_create_opj_stream(output_file, false);
	if (!opj_start_compress(codec, img, strm)) {
		goto cleanup;
	}
	if (!opj_encode(codec, strm)) {
		goto cleanup;
	}
	if (!opj_end_compress(codec, strm)) {
		goto cleanup;
	}
	opj_stream_destroy(strm);
	opj_image_destroy(img);
	opj_destroy_codec(codec);
	return impack_opj_stream_write_errno;
	
cleanup:
	if (strm != NULL) {
		opj_stream_destroy(strm);
	}
	opj_image_destroy(img);
	if (codec != NULL) {
		opj_destroy_codec(codec);
	}
	return ret;
	
}

#endif
