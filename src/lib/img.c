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
#include <stddef.h>
#include <stdint.h>
#include "config.h"
#include "impack.h"
#include "img.h"

#ifdef IMPACK_WITH_PNG
const uint8_t impack_magic_png[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
const impack_img_format_desc_t impack_img_format_png = {
	FORMAT_PNG,
	"PNG",
	"*.png",
	NULL,
	false,
	impack_read_img_png,
	impack_write_img_png,
	impack_magic_png,
	8
};
#endif

#ifdef IMPACK_WITH_WEBP
const uint8_t impack_magic_webp[] = { 82, 73, 70, 70 };
const impack_img_format_desc_t impack_img_format_webp = {
	FORMAT_WEBP,
	"WebP",
	"*.webp",
	NULL,
	false,
	impack_read_img_webp,
	impack_write_img_webp,
	impack_magic_webp,
	4
};
#endif

#ifdef IMPACK_WITH_TIFF
const uint8_t impack_magic_tiff_le[] = { 73, 73, 42, 0 };
const uint8_t impack_magic_tiff_be[] = { 77, 77, 0, 42 };
const char *impack_extension_alt_tiff[] = { "*.tif", NULL };
const impack_img_format_desc_t impack_img_format_tiff_le = {
	FORMAT_TIFF,
	"TIFF",
	"*.tiff",
	impack_extension_alt_tiff,
	false,
	impack_read_img_tiff_le,
	impack_write_img_tiff,
	impack_magic_tiff_le,
	4
};
const impack_img_format_desc_t impack_img_format_tiff_be = {
	FORMAT_TIFF,
	"TIFF",
	"*.tiff",
	impack_extension_alt_tiff,
	true,
	impack_read_img_tiff_be,
	impack_write_img_tiff,
	impack_magic_tiff_be,
	4
};
#endif

#ifdef IMPACK_WITH_BMP
const uint8_t impack_magic_bmp[] = { 66, 77 };
const impack_img_format_desc_t impack_img_format_bmp = {
	FORMAT_BMP,
	"BMP",
	"*.bmp",
	NULL,
	false,
	impack_read_img_bmp,
	impack_write_img_bmp,
	impack_magic_bmp,
	2
};
#endif

#ifdef IMPACK_WITH_JP2K
const uint8_t impack_magic_jp2k[] = { 0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A };
const char *impack_extension_alt_jp2k[] = { "*.j2k", NULL };
const impack_img_format_desc_t impack_img_format_jp2k = {
	FORMAT_JP2K,
	"JPEG2000",
	"*.jp2",
	impack_extension_alt_jp2k,
	false,
	impack_read_img_jp2k,
	impack_write_img_jp2k,
	impack_magic_jp2k,
	12
};
#endif

#ifdef IMPACK_WITH_FLIF
const uint8_t impack_magic_flif[] = { 70, 76, 73, 70 };
const impack_img_format_desc_t impack_img_format_flif = {
	FORMAT_FLIF,
	"FLIF",
	"*.flif",
	NULL,
	false,
	impack_read_img_flif,
	impack_write_img_flif,
	impack_magic_flif,
	4
};
#endif

#ifdef IMPACK_WITH_JXR
const uint8_t impack_magic_jxr[] = { 0x49, 0x49, 0xBC, 0x01, 0x20, 0x00, 0x00, 0x00 };
const char *impack_extension_alt_jxr[] = { "*.wdp", "*.hdp", NULL };
const impack_img_format_desc_t impack_img_format_jxr = {
	FORMAT_JXR,
	"JPEG-XR",
	"*.jxr",
	impack_extension_alt_jxr,
	false,
	impack_read_img_jxr,
	impack_write_img_jxr,
	impack_magic_jxr,
	8
};
#endif

#ifdef IMPACK_WITH_JPEGLS
const uint8_t impack_magic_jpegls[] = { 0xFF, 0xD8, 0xFF, 0xF7 };
const char *impack_extension_alt_jpegls[] = { "*.jpg", "*.jpeg", NULL };
const impack_img_format_desc_t impack_img_format_jpegls = {
	FORMAT_JPEGLS,
	"JPEG-LS",
	"*.jls",
	impack_extension_alt_jpegls,
	false,
	impack_read_img_jpegls,
	impack_write_img_jpegls,
	impack_magic_jpegls,
	4
};
#endif

const impack_img_format_desc_t *impack_img_formats[] = {
#ifdef IMPACK_WITH_BMP
	&impack_img_format_bmp,
#endif
#ifdef IMPACK_WITH_FLIF
	&impack_img_format_flif,
#endif
#ifdef IMPACK_WITH_JPEGLS
	&impack_img_format_jpegls,
#endif
#ifdef IMPACK_WITH_JXR
	&impack_img_format_jxr,
#endif
#ifdef IMPACK_WITH_JP2K
	&impack_img_format_jp2k,
#endif
#ifdef IMPACK_WITH_PNG
	&impack_img_format_png,
#endif
#ifdef IMPACK_WITH_TIFF
	&impack_img_format_tiff_le,
	&impack_img_format_tiff_be,
#endif
#ifdef IMPACK_WITH_WEBP
	&impack_img_format_webp,
#endif
	NULL
};
