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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <openjpeg.h>
#include "impack.h"
#include "img.h"
#include "openjpeg_io.h"

impack_error_t impack_read_img_jp2k(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	opj_stream_t *strm = impack_create_opj_stream(input_file, true);
	if (strm == NULL) {
		return ERROR_MALLOC;
	}
	if (!feof(input_file)) {
		opj_stream_destroy(strm);
		return ERROR_INPUT_IO;
	}
	
	opj_dparameters_t params;
	opj_set_default_decoder_parameters(&params);
	opj_codec_t *codec = opj_create_decompress(OPJ_CODEC_JP2);
	if (codec == NULL) {
		opj_stream_destroy(strm);
		return ERROR_MALLOC;
	}
	if (!opj_setup_decoder(codec, &params)) {
		opj_stream_destroy(strm);
		opj_destroy_codec(codec);
		return ERROR_MALLOC;
	}
	opj_image_t *img = NULL;
	if (!opj_read_header(strm, codec, &img)) {
		opj_stream_destroy(strm);
		opj_destroy_codec(codec);
		opj_image_destroy(img);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (!opj_decode(codec, strm, img)) {
		opj_stream_destroy(strm);
		opj_destroy_codec(codec);
		opj_image_destroy(img);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (!opj_end_decompress(codec, strm)) {
		opj_stream_destroy(strm);
		opj_destroy_codec(codec);
		opj_image_destroy(img);
		return ERROR_INPUT_IMG_INVALID;
	}
	opj_stream_destroy(strm);
	opj_destroy_codec(codec);
	
	if (img->color_space != OPJ_CLRSPC_SRGB && img->color_space != OPJ_CLRSPC_GRAY) {
		opj_image_destroy(img);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (img->color_space != OPJ_CLRSPC_SRGB && img->numcomps < 3) {
		opj_image_destroy(img);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (img->color_space == OPJ_CLRSPC_SRGB) {
		if (img->comps[0].w != img->comps[1].w || img->comps[0].w != img->comps[2].w || img->comps[0].h != img->comps[1].h || img->comps[0].h != img->comps[2].h) {
			opj_image_destroy(img);
			return ERROR_INPUT_IMG_INVALID;
		}
	}
	
	*pixeldata_size = img->comps[0].w * img->comps[0].h * 3;
	*pixeldata = malloc(*pixeldata_size);
	if (*pixeldata == NULL) {
		opj_image_destroy(img);
		return ERROR_MALLOC;
	}
	for (uint64_t i = 0; i < img->comps[0].w * img->comps[0].h; i++) {
		(*pixeldata)[i * 3] = img->comps[0].data[i];
		(*pixeldata)[(i * 3) + 1] = img->comps[1].data[i];
		(*pixeldata)[(i * 3) + 2] = img->comps[2].data[i];
	}
	opj_image_destroy(img);
	return ERROR_OK;
	
}

#endif
