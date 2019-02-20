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
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "impack.h"

void print_format(char *name, int *linelen, bool *is_first, bool is_default) {
	
	size_t space_needed = strlen(name);
	if (!(*is_first)) {
		space_needed += 2;
	}
	if (is_default) {
		space_needed += 10;
	}
	if (!(*is_first)) {
		printf(",");
	}
	if (79 - *linelen >= space_needed) {
		if (!(*is_first)) {
			printf(" ");
		}
	} else {
		printf("\n  ");
		*linelen = 2;
	}
	printf("%s", name);
	if (is_default) {
		printf(" (default)");
	}
	*is_first = false;
	*linelen += space_needed;
	
}

void impack_print_help() {
	
	printf("Usage: impack <options>\n");
	printf("\n");
	printf("Mode selection:\n");
	printf("  -e, --encode:       Pack a file into an image\n");
	printf("  -d, --decode:       Extract a file from an image\n");
	printf("  -h, --help:         Show this message and exit\n");
	printf("  -v, --version:      Show ImPack2's version number and exit\n");
	printf("\n");
	printf("File options:\n");
	printf("  -i, --input:        File to encode / Image to decode\n");
	printf("  -o, --output:       Path for the created image / extracted file\n");
	printf("                      When decoding, the filename stored inside the image is\n");
	printf("                      used if this option is omitted\n");
	printf("  If any filename is set to \"-\", stdin / stdout are used\n");
	printf("\n");
	printf("  -n, --no-filename:  Do not include the original filename in the image\n");
	printf("  --custom-filename:  Include a custom filename instead of the original one\n");
	printf("\n");
#ifdef IMPACK_WITH_CRYPTO
	printf("Encryption:\n");
	printf("  -c, --encrypt:      Encrypt the file before encoding\n");
	printf("  -p, --passphrase:   Specify a passphrase on the command line instead of\n");
	printf("                      reading it from the terminal (for automation/scripting)\n");
	printf("                      Be aware that this will show up in the process list!\n");
	printf("  --passphrase-file:  Read the passphrase from a file (for automation/scripting)\n");
	printf("                      This will only read the first line from the file\n");
	printf("\n");
#endif
#ifdef IMPACK_WITH_COMPRESSION
	printf("Compression:\n");
	printf("  -z, --compress:     Compress the file before encoding\n");
	printf("  --compression-type: Select the compression algorithm that will be used\n");
	printf("\n");
#endif
	printf("Image customization (when encoding):\n");
	printf("  -f, --format:       Select the image format\n");
	printf("  -w, --width,\n");
	printf("  -h, --height:       Select a custom image size\n");
	printf("                      By default, a square image is created.\n");
	printf("                      If only one of these parameters is set, the other one is\n");
	printf("                      auto-selected to make the image large enough\n");
	printf("                      If you select both parameters, you must make sure that\n");
	printf("                      the image is large enough yourself\n");
	printf("  --channel-red,\n");
	printf("  --channel-green,\n");
	printf("  --channel-blue,\n");
	printf("  --grayscale:        Select color channels that should be used to store data\n");
	printf("                      By default, all channels are used\n");
	printf("\n");
	
	printf("Supported image formats:\n");
	int linelen = 2;
	bool is_first = true;
	impack_img_format_t default_format = impack_default_img_format();
	printf("  ");
#ifdef IMPACK_WITH_PNG
	print_format("PNG", &linelen, &is_first, default_format == FORMAT_PNG);
#endif
#ifdef IMPACK_WITH_WEBP
	print_format("WebP", &linelen, &is_first, default_format == FORMAT_WEBP);
#endif
	printf("\n");
	
#ifdef IMPACK_WITH_COMPRESSION
	printf("\n");
	printf("Supported compression types:\n");
	linelen = 2;
	is_first = true;
	impack_compression_type_t default_compression = impack_default_compression();
	printf("  ");
#ifdef IMPACK_WITH_ZLIB
	print_format("Deflate", &linelen, &is_first, default_compression == COMPRESSION_ZLIB);
#endif
#ifdef IMPACK_WITH_LZMA
	print_format("LZMA2", &linelen, &is_first, default_compression == COMPRESSION_LZMA);
#endif
#ifdef IMPACK_WITH_ZSTD
	print_format("Zstd", &linelen, &is_first, default_compression == COMPRESSION_ZSTD);
#endif
	printf("\n");
#endif
	
}

void impack_print_version() {
	
	printf("ImPack2 %s\n", IMPACK_VERSION_STRING);
	
}
