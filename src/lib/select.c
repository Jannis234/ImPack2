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

#include <stddef.h>
#include <string.h>
#include "config.h"
#include "impack.h"

impack_img_format_t impack_select_img_format(char *name) {
	
	size_t namelen = strlen(name);
#ifdef IMPACK_WITH_PNG
	if (namelen == 3) {
		if ((name[0] == 'P' || name[0] == 'p') && \
			(name[1] == 'N' || name[1] == 'n') && \
			(name[2] == 'G' || name[2] == 'g')) {
			return FORMAT_PNG;
		}
	}
#endif
#ifdef IMPACK_WITH_WEBP
	if (namelen == 4) {
		if ((name[0] == 'W' || name[0] == 'w') && \
			(name[1] == 'E' || name[1] == 'e') && \
			(name[2] == 'B' || name[2] == 'b') && \
			(name[3] == 'P' || name[3] == 'p')) {
			return FORMAT_WEBP;
		}
	}
#endif
	return FORMAT_AUTO;
	
}

impack_img_format_t impack_default_img_format() {
	
#if defined(IMPACK_WITH_PNG)
	return FORMAT_PNG;
#elif defined(IMPACK_WITH_WEBP)
#else
#error "No image formats selected in config_build.mak"
#endif
	
}

#ifdef IMPACK_WITH_COMPRESSION
impack_compression_type_t impack_select_compression(char *name) {
	
	size_t namelen = strlen(name);
#ifdef IMPACK_WITH_ZLIB
	if (namelen == 7) {
		if ((name[0] == 'D' || name[0] == 'd') && \
			(name[1] == 'E' || name[1] == 'e') && \
			(name[2] == 'F' || name[2] == 'f') && \
			(name[3] == 'L' || name[3] == 'l') && \
			(name[4] == 'A' || name[4] == 'a') && \
			(name[5] == 'T' || name[5] == 't') && \
			(name[6] == 'E' || name[6] == 'e')) {
			return COMPRESSION_ZLIB;
		}
	}
#endif
	return COMPRESSION_NONE;
	
}

impack_compression_type_t impack_default_compression() {
	
#ifdef IMPACK_WITH_ZLIB
	return COMPRESSION_ZLIB;
#else
#error "Error in config.h"
#endif
	
}
#endif
