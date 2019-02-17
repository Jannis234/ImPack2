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
#include <stdlib.h>
#include "impack.h"
#include "cli.h"

int impack_print_error(impack_error_t error) {
	
	switch (error) {
		case ERROR_OK:
			return RETURN_OK;
		case ERROR_INPUT_NOT_FOUND:
			fprintf(stderr, "Can not read input file: No such file or directory\n");
			return RETURN_USER_ERROR;
		case ERROR_INPUT_PERMISSION:
			fprintf(stderr, "Can not read input file: Permission denied\n");
			return RETURN_SYSTEM_ERROR;
		case ERROR_INPUT_DIRECTORY:
			fprintf(stderr, "Can not read input file: Path is a directory\n");
			return RETURN_USER_ERROR;
		case ERROR_INPUT_IO:
			fprintf(stderr, "Can not read input file: I/O error\n");
			return RETURN_SYSTEM_ERROR;
		case ERROR_OUTPUT_NOT_FOUND:
			fprintf(stderr, "Can not write output file: No such directory\n");
			return RETURN_USER_ERROR;
		case ERROR_OUTPUT_PERMISSION:
			fprintf(stderr, "Can not write output file: Permission denied\n");
			return RETURN_SYSTEM_ERROR;
		case ERROR_OUTPUT_DIRECTORY:
			fprintf(stderr, "Can not write output file: Path is a directory\n");
			return RETURN_USER_ERROR;
		case ERROR_OUTPUT_IO:
			fprintf(stderr, "Can not write output file: I/O error\n");
			return RETURN_SYSTEM_ERROR;
		case ERROR_MALLOC:
			fprintf(stderr, "Out of memory\n");
			return RETURN_SYSTEM_ERROR;
		case ERROR_RANDOM:
			fprintf(stderr, "Can not gather random data for encryption\n");
			return RETURN_SYSTEM_ERROR;
		case ERROR_IMG_SIZE:
			fprintf(stderr, "Invalid image size for the selected output format\n");
			return RETURN_USER_ERROR;
		case ERROR_IMG_TOO_SMALL:
			fprintf(stderr, "Selected image size is too small\n");
			return RETURN_USER_ERROR;
		case ERROR_IMG_FORMAT_UNSUPPORTED:
			fprintf(stderr, "The image file format is unsupported by this build of ImPack2\n");
			return RETURN_DATA_ERROR;
		case ERROR_IMG_FORMAT_UNKNOWN:
			fprintf(stderr, "Unknown image file format\n");
			return RETURN_DATA_ERROR;
		case ERROR_INPUT_IMG_INVALID:
			fprintf(stderr, "The image contains invalid data\n"); // TODO: Print a message when this could be caused by an incorrect passphrase
			return RETURN_DATA_ERROR;
		case ERROR_INPUT_IMG_VERSION:
			fprintf(stderr, "The image was created by an incompatible newer version of ImPack2\n");
			return RETURN_DATA_ERROR;
		case ERROR_CRC:
			fprintf(stderr, "The data contained inside the image seems to be corrupted\n");
			return RETURN_DATA_ERROR;
		case ERROR_ENCRYPTION_UNAVAILABLE:
			fprintf(stderr, "The image contains encrypted data, but encryption is unsupported by this build of ImPack2\n");
			return RETURN_DATA_ERROR;
		case ERROR_ENCRYPTION_UNKNOWN:
			fprintf(stderr, "The image was created by an incompatible newer version of ImPack2\n");
			return RETURN_DATA_ERROR;
		case ERROR_COMPRESSION_UNAVAILABLE:
			fprintf(stderr, "The image contains compressed data, but compression is unsupported by this build of ImPack2\n");
			return RETURN_DATA_ERROR;
		case ERROR_COMPRESSION_UNSUPPORTED:
			fprintf(stderr, "The image was compressed with an algorithm that is unsupported by this build of ImPack2\n");
			return RETURN_DATA_ERROR;
		case ERROR_COMPRESSION_UNKNOWN:
			fprintf(stderr, "The image was created by an incompatible newer version of ImPack2\n");
			return RETURN_DATA_ERROR;
	}
	abort(); // Should never get here
	
}
