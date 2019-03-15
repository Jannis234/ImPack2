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
#include "img.h"

impack_error_t impack_read_img(FILE *input_file, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	uint8_t magic_png[] = IMPACK_MAGIC_PNG;
	uint8_t magic_webp[] = IMPACK_MAGIC_WEBP;
	uint8_t magic_tiff_le[] = IMPACK_MAGIC_TIFF_LE;
	uint8_t magic_tiff_be[] = IMPACK_MAGIC_TIFF_BE;
	uint8_t magic_bmp[] = IMPACK_MAGIC_BMP;
	uint8_t magic_jp2k[] = IMPACK_MAGIC_JP2K;
	uint8_t magic_flif[] = IMPACK_MAGIC_FLIF;
	
	uint8_t buf[4];
	if (fread(buf, 1, 2, input_file) != 2) {
		return ERROR_INPUT_IO;
	}
	bool ispng = true;
	bool iswebp = true;
	bool istiff_le = true;
	bool istiff_be = true;
	bool isbmp = true;
	bool isjp2k = true;
	bool isflif = true;
	for (int i = 0; i < 2; i++) {
		ispng &= (buf[i] == magic_png[i]);
		iswebp &= (buf[i] == magic_webp[i]);
		istiff_le &= (buf[i] == magic_tiff_le[i]);
		istiff_be &= (buf[i] == magic_tiff_be[i]);
		isbmp &= (buf[i] == magic_bmp[i]);
		isjp2k &= (buf[i] == magic_jp2k[i]);
		isflif &= (buf[i] == magic_flif[i]);
	}
	if (isbmp) {
#ifdef IMPACK_WITH_BMP
		return impack_read_img_bmp(input_file, pixeldata, pixeldata_size);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}
	
	if (fread(buf, 1, 2, input_file) != 2) {
		return ERROR_INPUT_IO;
	}
	for (int i = 0; i < 2; i++) {
		ispng &= (buf[i] == magic_png[i + 2]);
		iswebp &= (buf[i] == magic_webp[i + 2]);
		istiff_le &= (buf[i] == magic_tiff_le[i + 2]);
		istiff_be &= (buf[i] == magic_tiff_be[i + 2]);
		isjp2k &= (buf[i] == magic_jp2k[i + 2]);
		isflif &= (buf[i] == magic_flif[i + 2]);
	}
	if (iswebp) {
#ifdef IMPACK_WITH_WEBP
		return impack_read_img_webp(input_file, pixeldata, pixeldata_size);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}
	if (istiff_le || istiff_be) {
#ifdef IMPACK_WITH_TIFF
		return impack_read_img_tiff(input_file, pixeldata, pixeldata_size, istiff_le);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}
	if (isflif) {
#ifdef IMPACK_WITH_FLIF
		return impack_read_img_flif(input_file, pixeldata, pixeldata_size);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}
	
	if (fread(buf, 1, 4, input_file) != 4) {
		return ERROR_INPUT_IO;
	}
	for (int i = 0; i < 4; i++) {
		ispng &= (buf[i] == magic_png[i + 4]);
		isjp2k &= (buf[i] == magic_jp2k[i + 4]);
	}
	if (ispng) {
#ifdef IMPACK_WITH_PNG
		return impack_read_img_png(input_file, pixeldata, pixeldata_size);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}
	
	if (fread(buf, 1, 4, input_file) != 4) {
		return ERROR_INPUT_IO;
	}
	for (int i = 0; i < 4; i++) {
		isjp2k &= (buf[i] == magic_jp2k[i + 8]);
	}
	if (isjp2k) {
#ifdef IMPACK_WITH_JP2K
		return impack_read_img_jp2k(input_file, pixeldata, pixeldata_size);
#else
		return ERROR_IMG_FORMAT_UNSUPPORTED;
#endif
	}
	
	return ERROR_IMG_FORMAT_UNKNOWN;
	
}
