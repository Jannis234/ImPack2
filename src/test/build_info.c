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

#include <stdio.h>
#include "config.h"

#ifdef IMPACK_WITH_CRYPTO
#include <nettle/version.h>
#endif
#ifdef IMPACK_WITH_PNG
#include <png.h>
#endif
#ifdef IMPACK_WITH_WEBP
#include <webp/decode.h>
#include <webp/encode.h>
#endif
#ifdef IMPACK_WITH_TIFF
#include <tiffio.h>
#endif
#ifdef IMPACK_WITH_ZLIB
#include <zlib.h>
#endif
#ifdef IMPACK_WITH_ZSTD
#include <zstd.h>
#endif
#ifdef IMPACK_WITH_LZMA
#include <lzma.h>
#endif

void impack_build_info() {
	
	printf("Build information:\n");
	printf("  Crypto: ");
#ifdef IMPACK_WITH_CRYPTO
	printf("Yes\n");
#else
	printf("No\n");
#endif
	printf("  Image formats:");
#ifdef IMPACK_WITH_PNG
	printf(" PNG");
#endif
#ifdef IMPACK_WITH_WEBP
	printf(" WebP");
#endif
#ifdef IMPACK_WITH_TIFF
	printf(" TIFF");
#endif
#ifdef IMPACK_WITH_BMP
	printf(" BMP");
#endif
	printf("\n  Compression types:");
#ifdef IMPACK_WITH_ZLIB
	printf(" Deflate");
#endif
#ifdef IMPACK_WITH_ZSTD
	printf(" ZSTD");
#endif
#ifdef IMPACK_WITH_LZMA
	printf(" LZMA2");
#endif
#ifdef IMPACK_WITH_BZIP2
	printf(" Bzip2");
#endif
	printf("\n\n");
	
	printf("Third party libraries (build / runtime):\n");
#ifdef IMPACK_WITH_CRYPTO
	printf("  nettle: %d.%d / %d.%d\n", NETTLE_VERSION_MAJOR, NETTLE_VERSION_MINOR, nettle_version_major(), nettle_version_minor());
#endif
#ifdef IMPACK_WITH_PNG
	printf("  libpng: %s / %d\n", PNG_LIBPNG_VER_STRING, png_access_version_number());
#endif
#ifdef IMPACK_WITH_WEBP
	printf("  libwebp decode: 0x%X / 0x%X\n", WEBP_DECODER_ABI_VERSION, WebPGetDecoderVersion());
	printf("  libwebp encode: 0x%X / 0x%X\n", WEBP_ENCODER_ABI_VERSION, WebPGetEncoderVersion());
#endif
#ifdef IMPACK_WITH_TIFF
	printf("  libtiff: %d / ", TIFFLIB_VERSION);
	const char *tiffver = TIFFGetVersion();
	while (*tiffver != '\n') {
		tiffver++;
	}
	while (*tiffver != ' ') {
		tiffver--;
	}
	tiffver++;
	while (*tiffver != '\n') {
		putchar(*tiffver);
		tiffver++;
	}
	printf("\n");
#endif
#ifdef IMPACK_WITH_ZLIB
	printf("  zlib: %s / %s\n", ZLIB_VERSION, zlibVersion());
#endif
#ifdef IMPACK_WITH_ZSTD
	printf("  zstd: %d / %d\n", ZSTD_VERSION_NUMBER, ZSTD_versionNumber());
#endif
#ifdef IMPACK_WITH_LZMA
	printf("  liblzma: %s / %s\n", LZMA_VERSION_STRING, lzma_version_string());
#endif
	
}
