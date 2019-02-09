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

#ifndef __IMPACK_H__
#define __IMPACK_H__

typedef enum {
	ERROR_OK, // Success
	ERROR_MALLOC, // malloc() failed
	ERROR_INPUT_NOT_FOUND, // Input file not found
	ERROR_INPUT_PERMISSION, // Access to input file denied
	ERROR_INPUT_DIRECTORY, // Input path is a directory
	ERROR_INPUT_IO, // I/O error while reading input file
	ERROR_OUTPUT_NOT_FOUND, // Output path not found
	ERROR_OUTPUT_PERMISSION, // Access to output file denied
	ERROR_OUTPUT_DIRECTORY, // Output path is a directory
	ERROR_OUTPUT_IO, // I/O error while writing output file
	ERROR_IMG_SIZE // Invalid size for output image
} impack_error_t;

#define IMPACK_CHANNEL_RED 1
#define IMPACK_CHANNEL_GREEN 2
#define IMPACK_CHANNEL_BLUE 4

impack_error_t impack_encode(char *input_path, char *output_path);

#endif

