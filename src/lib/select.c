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

char convert_lowercase(char c) {
	
	if (c >= 'A' && c <= 'Z') {
		return c + ('a' - 'A');
	}
	return c;
	
}

bool compare_case(char *s1, char *s2) {
	
	if (strlen(s1) != strlen(s2)) {
		return false;
	}
	while (*s1) {
		if (convert_lowercase(*s1) != convert_lowercase(*s2)) {
			return false;
		}
		s1++;
		s2++;
	}
	return true;
	
}

impack_img_format_t impack_select_img_format(char *name, bool fileextension) {
	
	int i = 0;
	while (impack_img_formats[i] != NULL) {
		const impack_img_format_desc_t *current = impack_img_formats[i];
		if (fileextension) {
			if (compare_case(name, current->extension + 2)) {
				return current->id;
			}
			if (current->extension_alt != NULL) {
				int j = 0;
				while (current->extension_alt[j] != NULL) {
					if (compare_case(name, ((char*) current->extension_alt[j]) + 2)) {
						return current->id;
					}
					j++;
				}
			}
		} else {
			if (compare_case(name, current->name)) {
				return current->id;
			}
		}
		i++;
	}
	return FORMAT_AUTO;
	
}

impack_img_format_t impack_default_img_format() {
	
#if defined(IMPACK_WITH_PNG)
	return FORMAT_PNG;
#elif defined(IMPACK_WITH_WEBP)
	return FORMAT_WEBP;
#elif defined(IMPACK_WITH_TIFF)
	return FORMAT_TIFF;
#elif defined(IMPACK_WITH_HEIF)
	return FORMAT_HEIF;
#elif defined(IMPACK_WITH_AVIF)
	return FORMAT_AVIF;
#elif defined(IMPACK_WITH_BMP)
	return FORMAT_BMP;
#elif defined(IMPACK_WITH_JP2K)
	return FORMAT_JP2K;
#elif defined(IMPACK_WITH_JXL)
	return FORMAT_JXL;
#elif defined(IMPACK_WITH_FLIF)
	return FORMAT_FLIF;
#elif defined(IMPACK_WITH_JPEGLS)
	return FORMAT_JPEGLS;
#elif defined(IMPACK_WITH_JXR)
	return FORMAT_JXR;
#else
#error "No image formats selected in config_build.mak"
#endif
	
}

#ifdef IMPACK_WITH_COMPRESSION
impack_compression_type_t impack_select_compression(char *name) {
	
	int current = 0;
	while (impack_compression_types[current] != NULL) {
		if (compare_case(name, impack_compression_types[current]->name)) {
			return impack_compression_types[current]->id;
		}
		current++;
	}
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

impack_encryption_type_t impack_select_encryption(char *name, bool force_pbkdf2) {
	
	if (compare_case(name, "aes")) {
		if (force_pbkdf2) {
			return ENCRYPTION_AES;
		} else {
			return ENCRYPTION_AES_ARGON2;
		}
	}
	if (compare_case(name, "serpent")) {
		if (force_pbkdf2) {
			return ENCRYPTION_SERPENT;
		} else {
			return ENCRYPTION_SERPENT_ARGON2;
		}
	}
	if (compare_case(name, "twofish")) {
		if (force_pbkdf2) {
			return ENCRYPTION_TWOFISH;
		} else {
			return ENCRYPTION_TWOFISH_ARGON2;
		}
	}
	if (compare_case(name, "camellia")) {
		if (force_pbkdf2) {
			return ENCRYPTION_CAMELLIA;
		} else {
			return ENCRYPTION_CAMELLIA_ARGON2;
		}
	}
	return ENCRYPTION_NONE;
	
}

impack_encryption_type_t impack_default_encryption(bool force_pbkdf2) {
	
#ifdef IMPACK_WITH_ARGON2
	if (force_pbkdf2) {
		return ENCRYPTION_AES;
	} else {
		return ENCRYPTION_AES_ARGON2;
	}
#else
	return ENCRYPTION_AES;
#endif
	
}
