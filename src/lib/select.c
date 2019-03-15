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
#include <string.h>
#include "config.h"
#include "impack.h"

bool check_case(char *s, char *upper, char *lower) {
	
	for (size_t i = 0; i < strlen(s); i++) {
		if (s[i] != upper[i] && s[i] != lower[i]) {
			return false;
		}
	}
	return true;
	
}

impack_img_format_t impack_select_img_format(char *name, bool fileextension) {
	
	size_t namelen = strlen(name);
#ifdef IMPACK_WITH_PNG
	if (namelen == 3) {
		if (check_case(name, "PNG", "png")) {
			return FORMAT_PNG;
		}
	}
#endif
#ifdef IMPACK_WITH_WEBP
	if (namelen == 4) {
		if (check_case(name, "WEBP", "webp")) {
			return FORMAT_WEBP;
		}
	}
#endif
#ifdef IMPACK_WITH_TIFF
	if (namelen == 4) {
		if (check_case(name, "TIFF", "tiff")) {
			return FORMAT_TIFF;
		}
	}
	if (namelen == 3 && fileextension) {
		if (check_case(name, "TIF", "tif")) {
			return FORMAT_TIFF;
		}
	}
#endif
#ifdef IMPACK_WITH_BMP
	if (namelen == 3) {
		if (check_case(name, "BMP", "bmp")) {
			return FORMAT_BMP;
		}
	}
#endif
#ifdef IMPACK_WITH_JP2K
	if (fileextension && namelen == 3) {
		if (check_case(name, "JP2", "jp2")) {
			return FORMAT_JP2K;
		}
		if (check_case(name, "J2K", "j2k")) {
			return FORMAT_JP2K;
		}
	}
	if (!fileextension && namelen == 8) {
		if (check_case(name, "JPEG2000", "jpeg2000")) {
			return FORMAT_JP2K;
		}
	}
#endif
#ifdef IMPACK_WITH_FLIF
	if (namelen == 4) {
		if (check_case(name, "FLIF", "flif")) {
			return FORMAT_FLIF;
		}
	}
#endif
	return FORMAT_AUTO;
	
}

impack_img_format_t impack_default_img_format() {
	
#if defined(IMPACK_WITH_PNG)
	return FORMAT_PNG;
#elif defined(IMPACK_WITH_WEBP)
	return FORMAT_WEBP;
#elif defined(IMPACK_WITH_TIFF)
	return FORMAT_TIFF;
#elif defined(IMPACK_WITH_BMP)
	return FORMAT_BMP;
#elif defined(IMPACK_WITH_JP2K)
	return FORMAT_JP2K;
#else
#error "No image formats selected in config_build.mak"
#endif
	
}

#ifdef IMPACK_WITH_COMPRESSION
impack_compression_type_t impack_select_compression(char *name) {
	
	size_t namelen = strlen(name);
#ifdef IMPACK_WITH_ZLIB
	if (namelen == 7) {
		if (check_case(name, "DEFLATE", "deflate")) {
			return COMPRESSION_ZLIB;
		}
	}
#endif
#ifdef IMPACK_WITH_ZSTD
	if (namelen == 4) {
		if (check_case(name, "ZSTD", "zstd")) {
			return COMPRESSION_ZSTD;
		}
	}
#endif
#ifdef IMPACK_WITH_LZMA
	if (namelen == 5) {
		if (check_case(name, "LZMA2", "lzma2")) {
			return COMPRESSION_LZMA;
		}
	}
#endif
#ifdef IMPACK_WITH_BZIP2
	if (namelen == 5) {
		if (check_case(name, "BZIP2", "bzip2")) {
			return COMPRESSION_BZIP2;
		}
	}
#endif
#ifdef IMPACK_WITH_BROTLI
	if (namelen == 6) {
		if (check_case(name, "BROTLI", "brotli")) {
			return COMPRESSION_BROTLI;
		}
	}
#endif
	return COMPRESSION_NONE;
	
}

impack_compression_type_t impack_default_compression() {
	
#if defined(IMPACK_WITH_ZSTD)
	return COMPRESSION_ZSTD;
#elif defined(IMPACK_WITH_ZLIB)
	return COMPRESSION_ZLIB;
#elif defined(IMPACK_WITH_BROTLI)
	return COMPRESSION_BROTLI;
#elif defined(IMPACK_WITH_BZIP2)
	return COMPRESSION_BZIP2;
#elif defined(IMPACK_WITH_LZMA)
	return COMPRESSION_LZMA;
#else
#error "Error in config.h"
#endif
	
}
#endif

impack_encryption_type_t impack_select_encryption(char *name) {
	
	size_t namelen = strlen(name);
	if (namelen == 3) {
		if (check_case(name, "AES", "aes")) {
			return ENCRYPTION_AES;
		}
	}
	if (namelen == 7) {
		if (check_case(name, "SERPENT", "serpent")) {
			return ENCRYPTION_SERPENT;
		}
		if (check_case(name, "TWOFISH", "twofish")) {
			return ENCRYPTION_TWOFISH;
		}
	}
	if (namelen == 8) {
		if (check_case(name, "CAMELLIA", "camellia")) {
			return ENCRYPTION_CAMELLIA;
		}
	}
	return ENCRYPTION_NONE;
	
}

impack_encryption_type_t impack_default_encryption() {
	
	return ENCRYPTION_AES;
	
}
