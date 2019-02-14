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
#include <stdio.h>
#include "impack.h"
#include "cli.h"
#include "config.h"

int main(int argc, char **argv) {
	
	impack_argparse_t options[] = {             // Array indices for convenience:
		{ "help", 'h', false, false, NULL },    // 0
		{ "version", 'v', false, false, NULL }, // 1
		{ "encode", 'e', false, false, NULL },  // 2
		{ "decode", 'd', false, false, NULL },  // 3
		{ "input", 'i', true, false, NULL },    // 4
		{ "output", 'o', true, false, NULL },   // 5
	};
	if (!impack_argparse(options, sizeof(options) / sizeof(impack_argparse_t), argv, argc)) {
		return 1;
	}
	
	if (options[0].found) {
		impack_print_help();
		return RETURN_OK;
	} else if (options[1].found) {
		impack_print_version();
		return RETURN_OK;
	}
	
	if (options[2].found && options[3].found) {
		fprintf(stderr, "You can only select one of encode or decode at the same time\n");
		return RETURN_USER_ERROR;
	}
	if (!options[2].found && !options[3].found) {
		fprintf(stderr, "No operation selected\n");
		return RETURN_USER_ERROR;
	}
	if (!options[4].found) {
		fprintf(stderr, "No input file specified\n");
		return RETURN_USER_ERROR;
	}
	if (options[2].found && !options[5].found) {
		fprintf(stderr, "No output file specified\n");
		return RETURN_USER_ERROR;
	}
	
	if (options[2].found) {
		impack_error_t res = impack_encode(options[4].arg_out, options[5].arg_out, false, NULL, COMPRESSION_NONE);
		return impack_print_error(res);
	} else {
		impack_decode_state_t state;
		impack_error_t res = impack_decode_stage1(&state, options[4].arg_out);
		if (res != ERROR_OK) {
			return impack_print_error(res);
		}
		if (state.encryption != 0) {
			return -1; // TODO
		}
		res = impack_decode_stage2(&state, NULL);
		if (res != ERROR_OK) {
			return impack_print_error(res);
		}
		char *out_path = ".";
		if (options[5].arg_out != NULL) {
			out_path = options[5].arg_out;
		}
		res = impack_decode_stage3(&state, out_path);
		return impack_print_error(res);
	}
	
}
