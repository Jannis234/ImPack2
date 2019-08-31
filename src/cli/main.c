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
		FILE *passfile = fopen(options[option_passphrase_file].arg_out, "rb+"); // Need to request write access to detect if the file is a directory
		if (passfile == NULL && errno != EISDIR) { // Not a directory, try without requesting write access (in case we don't have write permissions)
			passfile = fopen(options[option_passphrase_file].arg_out, "rb");
		}
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
		if (strlen(options[option_input].arg_out) == 1) {
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
		{ "channel-red", 0, false, false, NULL },
		{ "channel-green", 0, false, false, NULL },
		{ "channel-blue", 0, false, false, NULL },
		{ "grayscale", 0, false, false, NULL },
		{ "width", 0, true, false, NULL },
		{ "height", 0, true, false, NULL },
		{ "format", 'f', true, false, NULL },
		{ "no-filename", 'n', false, false, NULL },
		{ "custom-filename", 0, true, false, NULL },
#ifdef IMPACK_WITH_CRYPTO
		{ "encrypt", 'c', false, false, NULL },
		{ "encryption-type", 0, true, false, NULL },
		{ "passphrase", 'p', true, false, NULL },
		{ "passphrase-file", 0, true, false, NULL },
#endif
#ifdef IMPACK_WITH_COMPRESSION
		{ "compress", 'z', false, false, NULL },
		{ "compression-type", 0, true, false, NULL },
		{ "compression-level", 0, true, false, NULL },
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
	
	// The size of the options array changes depending on what's compiled in, so it's easier to search them at runtime
	int option_encode = impack_find_option(options, options_count, false, "e");
	int option_decode = impack_find_option(options, options_count, false, "d");
	int option_input = impack_find_option(options, options_count, false, "i");
	int option_output = impack_find_option(options, options_count, false, "o");
	int option_channel_red = impack_find_option(options, options_count, true, "channel-red");
	int option_channel_green = impack_find_option(options, options_count, true, "channel-green");
	int option_channel_blue = impack_find_option(options, options_count, true, "channel-blue");
	int option_grayscale = impack_find_option(options, options_count, true, "grayscale");
	int option_width = impack_find_option(options, options_count, true, "width");
	int option_height = impack_find_option(options, options_count, true, "height");
	int option_format = impack_find_option(options, options_count, false, "f");
	int option_no_filename = impack_find_option(options, options_count, false, "n");
	int option_custom_filename = impack_find_option(options, options_count, true, "custom-filename");
#ifdef IMPACK_WITH_CRYPTO
	int option_encrypt = impack_find_option(options, options_count, false, "c");
	int option_encryption_type = impack_find_option(options, options_count, true, "encryption-type");
#endif
#ifdef IMPACK_WITH_COMPRESSION
	int option_compress = impack_find_option(options, options_count, false, "z");
	int option_compression_type = impack_find_option(options, options_count, true, "compression-type");
	int option_compression_level = impack_find_option(options, options_count, true, "compression-level");
#endif
	
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
	if (options[option_decode].found) {
#ifdef IMPACK_WITH_CRYPTO
		if (options[option_encrypt].found || options[option_encryption_type].found) {
			fprintf(stderr, "Can not request encryption when decoding\n");
			return RETURN_USER_ERROR;
		}
#endif
#ifdef IMPACK_WITH_COMPRESSION
		if (options[option_compress].found || options[option_compression_type].found || options[option_compression_level].found) {
			fprintf(stderr, "Can not request compression when decoding\n");
			return RETURN_USER_ERROR;
		}
#endif
		if (options[option_channel_red].found || options[option_channel_green].found || options[option_channel_blue].found || options[option_grayscale].found) {
			fprintf(stderr, "Can not select color channels when decoding\n");
			return RETURN_USER_ERROR;
		}
		if (options[option_width].found || options[option_height].found) {
			fprintf(stderr, "Can not select the image size when decoding\n");
			return RETURN_USER_ERROR;
		}
		if (options[option_format].found) {
			fprintf(stderr, "Can not select the image format when decoding\n");
			return RETURN_USER_ERROR;
		}
		if (options[option_no_filename].found || options[option_custom_filename].found) {
			fprintf(stderr, "Can not select the included filename when decoding\n");
			return RETURN_USER_ERROR;
		}
	}
	if (options[option_grayscale].found && (options[option_channel_red].found || options[option_channel_green].found || options[option_channel_blue].found)) {
		fprintf(stderr, "Can not select color channels in grayscale mode\n");
		return RETURN_USER_ERROR;
	}
	if (options[option_custom_filename].found) {
		if (options[option_no_filename].found) {
			fprintf(stderr, "Can not select a custom filename when no filename should be included\n");
			return RETURN_USER_ERROR;
		}
		if (strlen(options[option_custom_filename].arg_out) == 0) {
			fprintf(stderr, "Custom filename can not be empty\n");
			return RETURN_USER_ERROR;
		}
	}
#ifdef IMPACK_WITH_CRYPTO
	if (!options[option_encrypt].found && options[option_encryption_type].found) {
		fprintf(stderr, "Can not select the encryption type when encryption is disabled\n");
		return RETURN_USER_ERROR;
	}
#endif
#ifdef IMPACK_WITH_COMPRESSION
	if (!options[option_compress].found) {
		if (options[option_compression_type].found) {
			fprintf(stderr, "Can not select the compression type when compression is disabled\n");
			return RETURN_USER_ERROR;
		}
		if (options[option_compression_level].found) {
			fprintf(stderr, "Can not select the compression level when compression is disabled\n");
			return RETURN_USER_ERROR;
		}
	}
#endif
	
#ifdef IMPACK_WITH_CRYPTO
	int option_passphrase = impack_find_option(options, options_count, false, "p");
	int option_passphrase_file = impack_find_option(options, options_count, true, "passphrase-file");
#endif
	if (options[option_encode].found) {
		int64_t width = 0;
		int64_t height = 0;
		if (options[option_width].found) {
			char *endptr;
			width = strtol(options[option_width].arg_out, &endptr, 10);
			if (*endptr != 0 || strlen(options[option_width].arg_out) == 0) {
				fprintf(stderr, "Invalid argument for image width\n");
				return RETURN_USER_ERROR;
			}
			if (width <= 0) {
				fprintf(stderr, "Invalid argument for image width\n");
				return RETURN_USER_ERROR;
			}
		}
		if (options[option_height].found) {
			char *endptr;
			height = strtol(options[option_height].arg_out, &endptr, 10);
			if (*endptr != 0 || strlen(options[option_height].arg_out) == 0) {
				fprintf(stderr, "Invalid argument for image height\n");
				return RETURN_USER_ERROR;
			}
			if (height <= 0) {
				fprintf(stderr, "Invalid argument for image height\n");
				return RETURN_USER_ERROR;
			}
		}
		
		uint8_t compression = COMPRESSION_NONE;
		int32_t compression_level = 0;
#ifdef IMPACK_WITH_COMPRESSION
		if (options[option_compress].found) {
			if (options[option_compression_type].found) {
				compression = impack_select_compression(options[option_compression_type].arg_out);
				if (compression == COMPRESSION_NONE) { 
					fprintf(stderr, "Unknown compression type\n");
					return RETURN_USER_ERROR;
				}
			} else {
				compression = impack_default_compression();
			}
			
			if (options[option_compression_level].found) {
				char *endptr;
				compression_level = strtol(options[option_compression_level].arg_out, &endptr, 10);
				if (*endptr != 0 || strlen(options[option_compression_level].arg_out) == 0) {
					fprintf(stderr, "Invalid compression level\n");
					return RETURN_USER_ERROR;
				}
				if (!impack_compress_level_valid(compression, compression_level) || compression_level <= 0) {
					fprintf(stderr, "Invalid compression level\n");
					return RETURN_USER_ERROR;
				}
			}
		}
#endif
		
		impack_img_format_t format = FORMAT_AUTO;
		if (options[option_format].found) {
			format = impack_select_img_format(options[option_format].arg_out, false);
			if (format == FORMAT_AUTO) {
				fprintf(stderr, "Unknown image format\n");
				return RETURN_USER_ERROR;
			}
		}
		
		impack_encryption_type_t encrypt = ENCRYPTION_NONE;
		char *passphrase = NULL;
#ifdef IMPACK_WITH_CRYPTO
		if (options[option_encrypt].found) {
			encrypt = impack_default_encryption();
			if (options[option_encryption_type].found) {
				encrypt = impack_select_encryption(options[option_encryption_type].arg_out);
				if (encrypt == ENCRYPTION_NONE) {
					fprintf(stderr, "Unknown encryption type\n");
					return RETURN_USER_ERROR;
				}
			}
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
		uint8_t channels = 0;
		if (options[option_channel_red].found) {
			channels |= CHANNEL_RED;
		}
		if (options[option_channel_green].found) {
			channels |= CHANNEL_GREEN;
		}
		if (options[option_channel_blue].found) {
			channels |= CHANNEL_BLUE;
		}
		if (channels == 0 && !options[option_grayscale].found) {
			channels = CHANNEL_RED | CHANNEL_GREEN | CHANNEL_BLUE;
		}
		
		char *filename_include = options[option_input].arg_out;
		if (strlen(filename_include) == 1 && filename_include[0] == '-') {
			filename_include = "stdin";
		}
		if (options[option_no_filename].found) {
			filename_include = "out"; // Include a placeholder instead
		} else if (options[option_custom_filename].found) {
			filename_include = options[option_custom_filename].arg_out;
		}
		impack_error_t res = impack_encode(options[option_input].arg_out, options[option_output].arg_out, encrypt, passphrase, compression, compression_level, channels, width, height, format, filename_include);
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
#endif
		char *passphrase = NULL;
		impack_decode_state_t state;
		impack_error_t res = impack_decode_stage1(&state, options[option_input].arg_out);
		if (res != ERROR_OK) {
			return impack_print_error(res);
		}
#ifdef IMPACK_WITH_CRYPTO
		if (state.encryption != 0) {
			res = get_passphrase(&passphrase, options, options_count, false);
			if (res != ERROR_OK) {
				free(state.pixeldata);
				return res;
			}
		}
#endif
		res = impack_decode_stage2(&state, passphrase);
		if (res != ERROR_OK) {
			int return_val = impack_print_error(res);
#ifdef IMPACK_WITH_CRYPTO
			if (res == ERROR_INPUT_IMG_INVALID && state.encryption != 0) {
				fprintf(stderr, "Note: This may be caused by an incorrect passphrase\n");
			}
#endif
			return return_val;
		}
		char *out_path = ".";
		if (options[option_output].arg_out != NULL) {
			out_path = options[option_output].arg_out;
		}
		res = impack_decode_stage3(&state, out_path);
		int return_val = impack_print_error(res);
#ifdef IMPACK_WITH_CRYPTO
		if ((res == ERROR_INPUT_IMG_INVALID || res == ERROR_CRC) && state.encryption != 0) {
			fprintf(stderr, "Note: This may be caused by an incorrect passphrase\n");
		}
#endif
#ifndef IMPACK_WITH_CRYPTO
		if (res == ERROR_OK && state.legacy) {
			fprintf(stderr, "Warning: This build of ImPack2 can not check if the data in legacy images is corrupted\n");
		}
#endif
		return return_val;
	}
	
}
