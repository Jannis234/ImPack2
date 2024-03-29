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

#ifndef __IMPACK_IMG_H__
#define __IMPACK_IMG_H__

#include <stdint.h>
#include <stdio.h>
#include "impack.h"

impack_error_t impack_read_img_png(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_webp(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_tiff(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_bmp(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_jp2k(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_flif(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_jxr(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_jpegls(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_heif(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_avif(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_read_img_jxl(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size);
impack_error_t impack_write_img_png(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_webp(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_tiff(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_bmp(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_jp2k(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_flif(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_jxr(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_jpegls(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_heif(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_avif(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);
impack_error_t impack_write_img_jxl(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height);

#endif
