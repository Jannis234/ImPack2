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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "impack.h"
#include "cli.h"
#include "config.h"

#define PASSPHRASE_BUFSTEP 25

#ifdef IMPACK_WITH_CRYPTO
int get_passphrase(char **passphrase_out, impack_argparse_t *options, int options_count, bool confirm) {
	
	int option_input = impack_find_option(options, options_count, false, "i");
	int option_passphrase = impack_find_option(options, options_count, false, "p");
	int option_passphrase_file = impack_find_option(options, options_count, true, "passphrase-file");
	if (options[option_passphrase].found) {
		*passphrase_out = malloc(strlen(options[option_passphrase].arg_out) + 1); // Need to allocate a new buffer, since me might later try writing to it
		if (*passphrase_out == NULL) {
			fprintf(stderr, "Out of memory\n");
			return RETURN_SYSTEM_ERROR;
		}
		strcpy(*passphrase_out, options[option_passphrase].arg_out);
	} else if (options[option_passphrase_file].found) {
		FILE *passfile = fopen(options[option_passphrase_file].arg_out, "rb");
		if (passfile == NULL) {
			if (errno == ENOENT) {
				fprintf(stderr, "Can not read passphrase file: No such file or directory\n");
				return RETURN_USER_ERROR;
			} else if (errno == EACCES) {
				fprintf(stderr, "Can not read passphrase file: Permission denied\n");
				return RETURN_SYSTEM_ERROR;
			} else if (errno == EISDIR) {
				fprintf(stderr, "Can not read passphrase file: Path is a directory\n");
				return RETURN_USER_ERROR;
			} else {
				fprintf(stderr, "Can not read passphrase file: I/O error\n");
				return RETURN_SYSTEM_ERROR;
			}
		}
		*passphrase_out = malloc(PASSPHRASE_BUFSTEP);
		if (*passphrase_out == NULL) {
			fprintf(stderr, "Out of memory\n");
			fclose(passfile);
			return RETURN_SYSTEM_ERROR;
		}
		uint64_t passphrase_pos = 0;
		uint64_t passphrase_size = PASSPHRASE_BUFSTEP;
		char c;
		do {
			if (fread(&c, 1, 1, passfile) != 1) {
				fprintf(stderr, "Can not read passphrase file: I/O error\n");
				fclose(passfile);
				free(*passphrase_out);
				return RETURN_SYSTEM_ERROR;
			}
			*(*passphrase_out + passphrase_pos) = c;
			passphrase_pos++;
			if (passphrase_pos == passphrase_size) {
				char *passphrase_new = realloc(*passphrase_out, passphrase_size + PASSPHRASE_BUFSTEP);
				if (passphrase_new == NULL) {
					fprintf(stderr, "Out of memory\n");
					fclose(passfile);
					free(*passphrase_out);
					return RETURN_SYSTEM_ERROR;
				}
				*passphrase_out = passphrase_new;
				passphrase_size += PASSPHRASE_BUFSTEP;
			}
			if (feof(passfile)) {
				break;
			}
		} while (c != 0 && c != '\r' && c != '\n');
		*(*passphrase_out + passphrase_pos - 1) = 0; // Replaces any possible newline character
		fclose(passfile);
	} else {
		if (strlen(options[option_input].arg_out) == 1) { // TODO: Skip this is ncurses is used
			if (options[option_input].arg_out[0] == '-') {
				fprintf(stderr, "Can not read a passphrase and the input from stdin at the same time\n");
				return RETURN_USER_ERROR;
			}
		}
		fprintf(stderr, "Please enter the passphrase: ");
		*passphrase_out = impack_readpass();
		if (*passphrase_out == NULL) {
			fprintf(stderr, "Error while reading the passphrase\n");
			return RETURN_SYSTEM_ERROR;
		}
		if (confirm) {
			fprintf(stderr, "Please confirm the passphrase: ");
			char *passphrase_confirm = impack_readpass();
			if (passphrase_confirm == NULL) {
				fprintf(stderr, "Error while reading the passphrase\n");
				free(*passphrase_out);
				return RETURN_SYSTEM_ERROR;
			}
			if (strcmp(*passphrase_out, passphrase_confirm) != 0) {
				fprintf(stderr, "The passphrases did not match\n");
				free(*passphrase_out);
				free(passphrase_confirm);
				return RETURN_USER_ERROR;
			}
			free(passphrase_confirm);
		}
	}
	return RETURN_OK;
	
}
#endif

int main(int argc, char **argv) {
	
	impack_argparse_t options[] = {
		{ "help", 'h', false, false, NULL },
		{ "version", 'v', false, false, NULL },
		{ "encode", 'e', false, false, NULL },
		{ "decode", 'd', false, false, NULL },
		{ "input", 'i', true, false, NULL },
		{ "output", 'o', true, false, NULL },
#ifdef IMPACK_WITH_CRYPTO
		{ "encrypt", 'c', false, false, NULL },
		{ "passphrase", 'p', true, false, NULL },
		{ "passphrase-file", 0, true, false, NULL },
#endif
	};
	size_t options_count = sizeof(options) / sizeof(impack_argparse_t);
	if (!impack_argparse(options, options_count, argv, argc)) {
		return RETURN_USER_ERROR;
	}
	
	if (options[impack_find_option(options, options_count, false, "h")].found) {
		impack_print_help();
		return RETURN_OK;
	} else if (options[impack_find_option(options, options_count, false, "v")].found) {
		impack_print_version();
		return RETURN_OK;
	}
	
	int option_encode = impack_find_option(options, options_count, false, "e");
	int option_decode = impack_find_option(options, options_count, false, "d");
	int option_input = impack_find_option(options, options_count, false, "i");
	int option_output = impack_find_option(options, options_count, false, "o");
	if (options[option_encode].found && options[option_decode].found) {
		fprintf(stderr, "You can only select one of encode or decode at the same time\n");
		return RETURN_USER_ERROR;
	}
	if (!options[option_encode].found && !options[option_decode].found) {
		fprintf(stderr, "No operation selected\n");
		return RETURN_USER_ERROR;
	}
	if (!options[option_input].found) {
		fprintf(stderr, "No input file specified\n");
		return RETURN_USER_ERROR;
	}
	if (options[option_encode].found && !options[option_output].found) {
		fprintf(stderr, "No output file specified\n");
		return RETURN_USER_ERROR;
	}
	
#ifdef IMPACK_WITH_CRYPTO
	int option_passphrase = impack_find_option(options, options_count, false, "p");
	int option_passphrase_file = impack_find_option(options, options_count, true, "passphrase-file");
#endif
	if (options[option_encode].found) {
		bool encrypt = false;
		char *passphrase = NULL;
#ifdef IMPACK_WITH_CRYPTO
		if (options[impack_find_option(options, options_count, false, "c")].found) {
			encrypt = true;
			if (options[option_passphrase].found && options[option_passphrase_file].found) {
				fprintf(stderr, "Multiple sources for a passphrase specified\n");
				return RETURN_USER_ERROR;
			}
			int res = get_passphrase(&passphrase, options, options_count, true);
			if (res != RETURN_OK) {
				return res;
			}
		} else {
			if (options[option_passphrase].found || options[option_passphrase_file].found) {
				fprintf(stderr, "Passphrase specified but encryption not requested\n");
				return RETURN_USER_ERROR;
			}
		}
#endif
		impack_error_t res = impack_encode(options[option_input].arg_out, options[option_output].arg_out, encrypt, passphrase, COMPRESSION_NONE);
#ifdef IMPACK_WITH_CRYPTO
		free(passphrase);
#endif
		return impack_print_error(res);
	} else {
#ifdef IMPACK_WITH_CRYPTO
		if (options[option_passphrase].found && options[option_passphrase_file].found) {
			fprintf(stderr, "Multiple sources for a passphrase specified\n");
			return RETURN_USER_ERROR;
		}
		char *passphrase;
#endif
		impack_decode_state_t state;
		impack_error_t res = impack_decode_stage1(&state, options[option_input].arg_out);
		if (res != ERROR_OK) {
			return impack_print_error(res);
		}
		if (state.encryption != 0) {
			int res = get_passphrase(&passphrase, options, options_count, false);
			if (res != ERROR_OK) {
				free(state.pixeldata);
				return res;
			}
		}
		res = impack_decode_stage2(&state, passphrase);
		if (res != ERROR_OK) {
			return impack_print_error(res);
		}
		char *out_path = ".";
		if (options[option_output].arg_out != NULL) {
			out_path = options[option_output].arg_out;
		}
		res = impack_decode_stage3(&state, out_path);
		return impack_print_error(res);
	}
	
}
