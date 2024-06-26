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
#include "impack.h"

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
#ifdef IMPACK_WITH_JP2K
#include <openjpeg.h>
#include <opj_config.h>
#endif
#ifdef IMPACK_WITH_JXR
#include <JXRGlue.h>
#endif
#ifdef IMPACK_WITH_JPEGLS
#include <charls/charls.h>
#endif
#ifdef IMPACK_WITH_HEIF
#include <libheif/heif.h>
#endif
#ifdef IMPACK_WITH_AVIF
#include <avif/avif.h>
#endif
#ifdef IMPACK_WITH_JXL
#include <jxl/encode.h>
#include <jxl/decode.h>
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
#ifdef IMPACK_WITH_BROTLI
#include <brotli/encode.h>
#include <brotli/decode.h>
#endif

void impack_build_info() {
	
	printf("Build information:\n");
	printf("  ImPack2 version string: %s\n", IMPACK_VERSION_STRING);
	printf("  Crypto: ");
#ifdef IMPACK_WITH_CRYPTO
	printf("Yes, Argon2: ");
#ifdef IMPACK_WITH_ARGON2
	printf("Yes\n");
#else
	printf("No\n");
#endif
#else
	printf("No\n");
#endif
	printf("  Image formats:");
	int current = 0;
	while (impack_img_formats[current] != NULL) {
		printf(" %s", impack_img_formats[current]->name);
		current++;
	}
#ifdef IMPACK_WITH_COMPRESSION
	printf("\n  Compression types:");
	current = 0;
	while (impack_compression_types[current] != NULL) {
		printf(" %s", impack_compression_types[current]->name);
		current++;
	}
#else
	printf("\n  Compression not supported");
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
#ifdef IMPACK_WITH_JP2K
	printf("  openjpeg: %d.%d.%d / %s\n", OPJ_VERSION_MAJOR, OPJ_VERSION_MINOR, OPJ_VERSION_BUILD, opj_version());
#endif
#ifdef IMPACK_WITH_JXR
	printf("  jxrlib: 0x%X / \n", PK_SDK_VERSION);
#endif
#ifdef IMPACK_WITH_JPEGLS
	int32_t charls_version[3];
	charls_get_version_number(&charls_version[0], &charls_version[1], &charls_version[2]);
	printf("  charls: %d.%d.%d / %d.%d.%d\n", CHARLS_VERSION_MAJOR, CHARLS_VERSION_MINOR, CHARLS_VERSION_PATCH, charls_version[0], charls_version[1], charls_version[2]);
#endif
#ifdef IMPACK_WITH_HEIF
	printf("  libheif %s / %s\n", LIBHEIF_VERSION, heif_get_version());
#endif
#ifdef IMPACK_WITH_AVIF
	printf("  libavif %d.%d.%d / %s\n", AVIF_VERSION_MAJOR, AVIF_VERSION_MINOR, AVIF_VERSION_PATCH, avifVersion());
#endif
#ifdef IMPACK_WITH_JXL
	printf("  libjxl decode / %u\n", JxlDecoderVersion());
	printf("  libjxl encode / %u\n", JxlEncoderVersion());
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
#ifdef IMPACK_WITH_BROTLI
	printf("  brotli encode: / %u\n", BrotliEncoderVersion());
	printf("  brotli decode: / %u\n", BrotliDecoderVersion());
#endif
	
}
