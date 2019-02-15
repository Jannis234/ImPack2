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

#ifndef __IMPACK_CLI_H__
#define __IMPACK_CLI_H__

#include <stddef.h>
#include "impack.h"

typedef enum {
	RETURN_OK = 0,
	RETURN_USER_ERROR = 1, // User's fault (invalid CLI options, file not found, etc.)
	RETURN_DATA_ERROR = 2, // Invalid or corrupted input data
	RETURN_SYSTEM_ERROR = 3 // Problem with the system (malloc() failed, I/O error, etc.)
} impack_return_val_t;

typedef struct {
	char *name_long; // Name of the long option (example: --encode) or NULL
	char name_short; // Name of the short option (example: -e) or 0
	bool needs_arg; // Does the option require an extra argument?
	bool found; // Set to true by impack_argparse if this option was found in argv
	char *arg_out; // If this option takes an argument, impack_argparse sets this to that argument (pointer into argv)
} impack_argparse_t;

// Somewhat similar to getopt_long
bool impack_argparse(impack_argparse_t *options, size_t options_count, char **argv, int argc);
// Find an option by name and return its index
int impack_find_option(impack_argparse_t options[], size_t options_count, bool name_long, char *name);
void impack_print_help();
void impack_print_version();
int impack_print_error(impack_error_t error);
// Read a passphrase from the terminal (or stdin, if ncurses is disabled)
char* impack_readpass();

#endif
