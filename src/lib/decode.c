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

#include "config.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef IMPACK_WITH_CRYPTO
#include <nettle/aes.h>
#include <nettle/cbc.h>
#endif
#include "impack.h"
#include "impack_internal.h"

#define BUFSIZE 16384 // 16 KiB

bool pixelbuf_read(impack_decode_state_t *state, uint8_t *buf, uint64_t len) {
	
	while (len > 0) {
		if (state->pixeldata_pos == state->pixeldata_size) {
			return false;
		}
		if (((state->pixeldata_pos % 3) == 0 && (state->channels & IMPACK_CHANNEL_RED)) || \
			(((state->pixeldata_pos % 3) == 1) && (state->channels & IMPACK_CHANNEL_GREEN)) || \
			(((state->pixeldata_pos % 3) == 2) && (state->channels & IMPACK_CHANNEL_BLUE))) {
			*buf = (state->pixeldata)[state->pixeldata_pos];
			buf++;
			len--;
		}
		state->pixeldata_pos++;
	}
	return true;
	
}

impack_error_t impack_decode_stage1(impack_decode_state_t *state, char *input_path) {
	
	FILE *input_file;
	if (strlen(input_path) == 1 && input_path[0] == '-') {
		input_file = stdin;
	} else {
		input_file = fopen(input_path, "rb");
		if (input_file == NULL) {
			if (errno == ENOENT) {
				return ERROR_INPUT_NOT_FOUND;
			} else if (errno == EACCES) {
				return ERROR_INPUT_PERMISSION;
			} else if (errno == EISDIR) {
				return ERROR_INPUT_DIRECTORY;
			} else {
				return ERROR_INPUT_IO;
			}
		}
	}

	impack_error_t res = impack_read_img(input_file, &state->pixeldata, &state->pixeldata_size);
	state->pixeldata_pos = 0;
	fclose(input_file);
	if (res != ERROR_OK) {
		return res;
	}

	if (state->pixeldata_size < 3) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->channels = 0;
	if (state->pixeldata[0] == 255) {
		state->channels |= IMPACK_CHANNEL_RED;
	}
	if (state->pixeldata[1] == 255) {
		state->channels |= IMPACK_CHANNEL_GREEN;
	}
	if (state->pixeldata[2] == 255) {
		state->channels |= IMPACK_CHANNEL_BLUE;
	}
	if (state->pixeldata[0] == 0 && state->pixeldata[1] == 0 && state->pixeldata[2] == 0) {
		state->channels = IMPACK_CHANNEL_RED; // Can use any channel on a grayscale image
	}
	if (state->channels == 0) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->pixeldata_pos = 3;

	uint8_t magic_buf[IMPACK_MAGIC_NUMBER_LEN];
	if (!pixelbuf_read(state, magic_buf, IMPACK_MAGIC_NUMBER_LEN)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	uint8_t magic[] = IMPACK_MAGIC_NUMBER;
	if (memcmp(magic_buf, magic, IMPACK_MAGIC_NUMBER_LEN) != 0) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	uint8_t flags[3];
	if (!pixelbuf_read(state, flags, 3)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (flags[0] != 0) { // Version number
		free(state->pixeldata);
		return ERROR_INPUT_IMG_VERSION;
	}
	state->encryption = flags[1];
	state->compression = flags[2];
#ifdef IMPACK_WITH_CRYPTO
	if (flags[1] > 1) {
		return ERROR_ENCRYPTION_UNKNOWN;
	}
#else
	if (flags[1] > 0) {
		return ERROR_ENCRYPTION_UNAVAILABLE;
	}
#endif
	// TODO: Check compression flag

	return ERROR_OK;
	
}

impack_error_t impack_decode_stage2(impack_decode_state_t *state, char *passphrase) {

	if (!pixelbuf_read(state, (uint8_t*) &state->data_length, 8)) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (!pixelbuf_read(state, (uint8_t*) &state->filename_length, 4)) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->data_length = impack_endian64(state->data_length);
	state->filename_length = impack_endian32(state->filename_length);

	// Quick sanity check to avoid allocating giant buffers
	uint64_t bytes_remaining = state->pixeldata_size - state->pixeldata_pos;
	if (state->filename_length > bytes_remaining || state->filename_length == 0) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	if (state->data_length > bytes_remaining - state->filename_length) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}

	uint64_t crc;
	if (!pixelbuf_read(state, (uint8_t*) &crc, 8)) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->crc = impack_endian64(crc);
	uint64_t filename_length = state->filename_length;
#ifdef IMPACK_WITH_CRYPTO
	struct CBC_CTX(struct aes256_ctx, AES_BLOCK_SIZE) decrypt_ctx;
	if (state->encryption != 0) {
		if (!pixelbuf_read(state, decrypt_ctx.iv, AES_BLOCK_SIZE)) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}

		impack_derive_key(passphrase, state->aes_key, AES256_KEY_SIZE, decrypt_ctx.iv, AES_BLOCK_SIZE);
		impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		aes256_set_decrypt_key(&decrypt_ctx.ctx, state->aes_key);
		if (filename_length % AES_BLOCK_SIZE != 0) {
			filename_length += AES_BLOCK_SIZE - (filename_length % AES_BLOCK_SIZE);
		}
	}
#endif
	state->filename = malloc(filename_length);
	if (state->filename == NULL) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase(state->aes_key, AES256_KEY_SIZE);
			impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
		}
#endif
		free(state->pixeldata);
		return ERROR_MALLOC;
	}
	if (!pixelbuf_read(state, (uint8_t*) state->filename, filename_length)) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase(state->aes_key, AES256_KEY_SIZE);
			impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
		}
#endif
		free(state->pixeldata);
		free(state->filename);
		return ERROR_INPUT_IMG_INVALID;
	}
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != 0) {
		CBC_DECRYPT(&decrypt_ctx, aes256_decrypt, filename_length, (uint8_t*) state->filename, (uint8_t*) state->filename);
		memcpy(state->aes_iv, decrypt_ctx.iv, AES_BLOCK_SIZE);
		impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
	}
#endif

	return ERROR_OK;

}

impack_error_t impack_decode_stage3(impack_decode_state_t *state, char *output_path) {
	
	FILE *output_file;
	if (strlen(output_path) == 1 && output_path[0] == '-') {
		output_file = stdout;
	} else {
		output_file = fopen(output_path, "wb");
		if (output_file == NULL) {
			if (errno == EISDIR) { // Used selected a directory, append the filename from the image
				size_t output_path_length = strlen(output_path);
				char *newname = malloc(state->filename_length + output_path_length + 2);
				if (newname == NULL) {
#ifdef IMPACK_WITH_CRYPTO
					if (state->encryption != 0) {
						impack_secure_erase(state->aes_key, AES256_KEY_SIZE);
					}
#endif
					free(state->pixeldata);
					free(state->filename);
					return ERROR_MALLOC;
				}
				strcpy(newname, output_path);
				newname[output_path_length] = '/';
				strncpy(newname + output_path_length + 1, state->filename, state->filename_length);
				newname[output_path_length + state->filename_length + 1] = 0;
				output_file = fopen(newname, "wb");
				free(newname);
			}
			if (output_file == NULL) {
#ifdef IMPACK_WITH_CRYPTO
				if (state->encryption != 0) {
					impack_secure_erase(state->aes_key, AES256_KEY_SIZE);
				}
#endif
				free(state->filename);
				free(state->pixeldata);
				if (errno == ENOENT) {
					return ERROR_OUTPUT_NOT_FOUND;
				} else if (errno == EACCES) {
					return ERROR_OUTPUT_PERMISSION;
				} else if (errno == EISDIR) {
					return ERROR_OUTPUT_DIRECTORY;
				} else {
					return ERROR_OUTPUT_IO;
				}
			}
		}
	}
	free(state->filename);

	uint8_t *buf = malloc(BUFSIZE);
	if (buf == NULL) {
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			impack_secure_erase(state->aes_key, AES256_KEY_SIZE);
		}
#endif
		free(state->pixeldata);
		fclose(output_file);
		return ERROR_MALLOC;
	}

#ifdef IMPACK_WITH_CRYPTO
	struct CBC_CTX(struct aes256_ctx, AES_BLOCK_SIZE) decrypt_ctx;
	if (state->encryption != 0) {
		aes256_set_decrypt_key(&decrypt_ctx.ctx, state->aes_key);
		memcpy(decrypt_ctx.iv, state->aes_iv, AES_BLOCK_SIZE);
		impack_secure_erase(state->aes_key, AES256_KEY_SIZE);
	}
#endif

	impack_crc_init();
	uint64_t crc = 0;
	while (state->data_length > 0) {
		uint64_t remaining = BUFSIZE;
		if (state->data_length < remaining) {
			remaining = state->data_length;
		}
#ifdef IMPACK_WITH_CRYPTO
		uint32_t padding = 0;
		if (state->encryption != 0) {
			if (remaining % AES_BLOCK_SIZE != 0) {
				padding = AES_BLOCK_SIZE - (remaining % AES_BLOCK_SIZE);
				remaining += padding;
			}
		}
#endif
		if (!pixelbuf_read(state, buf, remaining)) {
#ifdef IMPACK_WITH_CRYPTO
			if (state->encryption != 0) {
				impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
			}
#endif
			free(state->pixeldata);
			free(buf);
			fclose(output_file);
			return ERROR_INPUT_IMG_INVALID;
		}
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			CBC_DECRYPT(&decrypt_ctx, aes256_decrypt, remaining, buf, buf);
			remaining -= padding; // Don't write padding into the output file
		}
#endif
		// TODO: Decompress
		if (fwrite(buf, 1, remaining, output_file) != remaining) {
#ifdef IMPACK_WITH_CRYPTO
			if (state->encryption != 0) {
				impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
			}
#endif
			free(state->pixeldata);
			free(buf);
			fclose(output_file);
			return ERROR_OUTPUT_IO;
		}
		impack_crc(&crc, buf, remaining);
		state->data_length -= remaining;
	}

#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != 0) {
		impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
	}
#endif
	fclose(output_file);
	free(buf);
	free(state->pixeldata);
	if (crc != state->crc) {
		return ERROR_CRC;
	}
	return ERROR_OK;

}

