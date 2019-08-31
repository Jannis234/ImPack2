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
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef IMPACK_WITH_CRYPTO
#include <nettle/sha2.h>
#endif
#include "impack.h"
#include "impack_internal.h"

#define BUFSIZE 16384 // 16 KiB

bool pixelbuf_read(impack_decode_state_t *state, uint8_t *buf, uint64_t len) {
	
	while (len > 0) {
		if (state->pixeldata_pos == state->pixeldata_size) {
			return false;
		}
		if (((state->pixeldata_pos % 3) == 0 && (state->channels & CHANNEL_RED)) || \
			(((state->pixeldata_pos % 3) == 1) && (state->channels & CHANNEL_GREEN)) || \
			(((state->pixeldata_pos % 3) == 2) && (state->channels & CHANNEL_BLUE))) {
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
		input_file = fopen(input_path, "rb+");
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
	
	impack_error_t ret = ERROR_INPUT_IMG_INVALID;
	if (state->pixeldata_size < 3) {
		goto cleanup;
	}
	state->channels = 0;
	if (state->pixeldata[0] == 255) {
		state->channels |= CHANNEL_RED;
	}
	if (state->pixeldata[1] == 255) {
		state->channels |= CHANNEL_GREEN;
	}
	if (state->pixeldata[2] == 255) {
		state->channels |= CHANNEL_BLUE;
	}
	if (state->pixeldata[0] == 0 && state->pixeldata[1] == 0 && state->pixeldata[2] == 0) {
		state->channels = CHANNEL_RED; // Can use any channel on a grayscale image
	}
	if (state->channels == 0) {
		goto cleanup;
	}
	state->pixeldata_pos = 3;
	
	uint8_t magic_buf[4];
	if (!pixelbuf_read(state, magic_buf, 4)) {
		goto cleanup;
	}
	uint8_t magic[] = IMPACK_MAGIC_NUMBER;
	if (memcmp(magic_buf, magic, 3) != 0) {
		goto cleanup;
	}
	state->legacy = false;
	if (magic_buf[3] != magic[3]) {
		if (magic_buf[3] == 97) { // 'a', could be a legacy file
			if (!pixelbuf_read(state, magic_buf, 2)) {
				goto cleanup;
			}
			if (magic_buf[0] != 99) { // 'c'
				goto cleanup;
			}
			state->legacy = true;
			if (magic_buf[1] == 107) { // 'k'
				state->compression = COMPRESSION_NONE;
			} else if (magic_buf[1] == 90) { // 'Z'
				state->compression = COMPRESSION_ZLIB;
			} else {
				goto cleanup;
			}
		} else {
			goto cleanup;
		}
	}
	
	if (!state->legacy) {
		uint8_t flags[3];
		if (!pixelbuf_read(state, flags, 3)) {
			goto cleanup;
		}
		if (flags[0] != 0) { // Version number
			goto cleanup;
		}
		state->encryption = flags[1];
		state->compression = flags[2];
	} else {
		uint32_t data_length;
		if (!pixelbuf_read(state, (uint8_t*) &data_length, 4)) {
			goto cleanup;
		}
		if (!pixelbuf_read(state, (uint8_t*) &state->filename_length, 4)) {
			goto cleanup;
		}
		state->data_length = impack_endian32_le(data_length);
		state->filename_length = impack_endian32_le(state->filename_length);
		if (!pixelbuf_read(state, state->checksum_legacy, 64)) {
			goto cleanup;
		}
		uint8_t flag;
		if (!pixelbuf_read(state, &flag, 1)) {
			goto cleanup;
		}
		state->encryption = (flag == 255) ? ENCRYPTION_AES : ENCRYPTION_NONE;
	}
	
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption > ENCRYPTION_TWOFISH) {
		ret = ERROR_ENCRYPTION_UNKNOWN;
		goto cleanup;
	}
#else
	if (state->encryption > ENCRYPTION_NONE) {
		ret = ERROR_ENCRYPTION_UNAVAILABLE;
		goto cleanup;
	}
#endif
	
	ret = ERROR_COMPRESSION_UNSUPPORTED;
	if (state->compression > COMPRESSION_NONE) {
#ifdef IMPACK_WITH_COMPRESSION
		switch (state->compression) {
			case COMPRESSION_ZLIB:
#ifdef IMPACK_WITH_ZLIB
				break;
#else
				goto cleanup;
#endif
			case COMPRESSION_ZSTD:
#ifdef IMPACK_WITH_ZSTD
				break;
#else
				goto cleanup;
#endif
			case COMPRESSION_LZMA:
#ifdef IMPACK_WITH_LZMA
				break;
#else
				goto cleanup;
#endif
			case COMPRESSION_BZIP2:
#ifdef IMPACK_WITH_BZIP2
				break;
#else
				goto cleanup;
#endif
			case COMPRESSION_BROTLI:
#ifdef IMPACK_WITH_BROTLI
				break;
#else
				goto cleanup;
#endif
			default:
				ret = ERROR_COMPRESSION_UNKNOWN;
				goto cleanup;
		}
#else
		ret = ERROR_COMPRESSION_UNAVAILABLE;
		goto cleanup;
#endif
	}
	
	return ERROR_OK;
	
cleanup:
	free(state->pixeldata);
	return ret;
	
}

impack_error_t impack_decode_stage2(impack_decode_state_t *state, char *passphrase) {
	
	impack_error_t ret = ERROR_INPUT_IMG_INVALID;
	state->filename = NULL;
	if (!state->legacy) {
		if (!pixelbuf_read(state, (uint8_t*) &state->data_length, 8)) {
			goto cleanup;
		}
		if (!pixelbuf_read(state, (uint8_t*) &state->filename_length, 4)) {
			goto cleanup;
		}
		state->data_length = impack_endian64(state->data_length);
		state->filename_length = impack_endian32(state->filename_length);
	}
	
	// Quick sanity check to avoid allocating giant buffers
	uint64_t bytes_remaining = state->pixeldata_size - state->pixeldata_pos;
	if (state->filename_length > bytes_remaining || state->filename_length == 0) {
		goto cleanup;
	}
	if (state->data_length > bytes_remaining - state->filename_length) {
		goto cleanup;
	}
	
	uint64_t filename_length;
#ifdef IMPACK_WITH_CRYPTO
	impack_crypt_ctx_t decrypt_ctx;
#endif
	if (!state->legacy) {
		uint64_t crc;
		if (!pixelbuf_read(state, (uint8_t*) &crc, 8)) {
			goto cleanup;
		}
		state->crc = impack_endian64(crc);
		filename_length = state->filename_length;
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != ENCRYPTION_NONE) {
			if (!pixelbuf_read(state, decrypt_ctx.iv, IMPACK_CRYPT_BLOCK_SIZE)) {
				goto cleanup;
			}
			impack_derive_key(passphrase, state->crypt_key, IMPACK_CRYPT_KEY_SIZE, decrypt_ctx.iv, IMPACK_CRYPT_BLOCK_SIZE);
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
			impack_set_decrypt_key(&decrypt_ctx, state->crypt_key, state->encryption);
			if (filename_length % IMPACK_CRYPT_BLOCK_SIZE != 0) {
				filename_length += IMPACK_CRYPT_BLOCK_SIZE - (filename_length % IMPACK_CRYPT_BLOCK_SIZE);
			}
		}
#endif
	} else {
		filename_length = state->filename_length;
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != ENCRYPTION_NONE) {
			if (!pixelbuf_read(state, state->crypt_iv, IMPACK_CRYPT_BLOCK_SIZE)) {
				goto cleanup;
			}
			uint8_t salt[32];
			if (!pixelbuf_read(state, salt, 32)) {
				goto cleanup;
			}
			impack_derive_key_legacy(passphrase, state->crypt_key, IMPACK_CRYPT_KEY_SIZE, salt, 32);
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
	}
	
	state->filename = malloc(filename_length);
	if (state->filename == NULL) {
		ret = ERROR_MALLOC;
		goto cleanup;
	}
	if (!pixelbuf_read(state, (uint8_t*) state->filename, filename_length)) {
		goto cleanup;
	}
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != ENCRYPTION_NONE && !state->legacy) {
		impack_decrypt(&decrypt_ctx, (uint8_t*) state->filename, filename_length, state->encryption);
		memcpy(state->crypt_iv, decrypt_ctx.iv, IMPACK_CRYPT_BLOCK_SIZE);
		impack_secure_erase((uint8_t*) &decrypt_ctx, sizeof(impack_crypt_ctx_t));
	}
#endif
	
	for (uint32_t i = 0; i < state->filename_length; i++) {
		if (state->filename[i] == 0) { // Check if the filename is a valid string without NULL bytes
			goto cleanup;
		}
	}
	
	return ERROR_OK;
	
cleanup:
	free(state->pixeldata);
	if (state->filename != NULL) {
		free(state->filename);
	}
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != ENCRYPTION_NONE) {
		impack_secure_erase(state->crypt_key, IMPACK_CRYPT_KEY_SIZE);
		impack_secure_erase((uint8_t*) &decrypt_ctx, sizeof(impack_crypt_ctx_t));
	}
#endif
	return ret;
	
}

impack_error_t impack_decode_stage3(impack_decode_state_t *state, char *output_path) {
	
	impack_error_t ret = ERROR_INPUT_IMG_INVALID;
	uint8_t *buf = NULL;
#ifdef IMPACK_WITH_COMPRESSION
	impack_compress_state_t decompress_state;
	decompress_state.bufsize = 0;
#endif
#ifdef IMPACK_WITH_CRYPTO
	impack_crypt_ctx_t decrypt_ctx;
#endif
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
					ret = ERROR_MALLOC;
					goto cleanup;
				}
				strcpy(newname, output_path);
				newname[output_path_length] = '/'; // This should also work on windows
				strncpy(newname + output_path_length + 1, state->filename, state->filename_length);
				newname[output_path_length + state->filename_length + 1] = 0;
				output_file = fopen(newname, "wb");
				free(newname);
			}
			if (output_file == NULL) {
				if (errno == ENOENT) {
					ret = ERROR_OUTPUT_NOT_FOUND;
				} else if (errno == EACCES) {
					ret = ERROR_OUTPUT_PERMISSION;
				} else if (errno == EISDIR) {
					ret = ERROR_OUTPUT_DIRECTORY;
				} else {
					ret = ERROR_OUTPUT_IO;
				}
				goto cleanup;
			}
		}
	}
	free(state->filename);
	state->filename = NULL;
	
	buf = malloc(BUFSIZE);
	if (buf == NULL) {
		ret = ERROR_MALLOC;
		goto cleanup;
	}
	
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != ENCRYPTION_NONE) {
		impack_set_decrypt_key(&decrypt_ctx, state->crypt_key, state->encryption);
		memcpy(decrypt_ctx.iv, state->crypt_iv, IMPACK_CRYPT_BLOCK_SIZE);
		impack_secure_erase(state->crypt_key, IMPACK_CRYPT_KEY_SIZE);
	}
#endif
	
#ifdef IMPACK_WITH_COMPRESSION
	if (state->compression != COMPRESSION_NONE) {
		decompress_state.type = state->compression;
		decompress_state.is_compress = false;
		decompress_state.bufsize = BUFSIZE;
		if (!impack_compress_init(&decompress_state)) {
			ret = ERROR_MALLOC;
			goto cleanup;
		}
	}
#endif
	
	if (!state->legacy) {
		impack_crc_init();
	}
	uint64_t crc = 0;
#ifdef IMPACK_WITH_CRYPTO
	struct sha512_ctx legacy_checksum;
	if (state->legacy) {
		sha512_init(&legacy_checksum);
	}
#endif
	bool loop_running = true;
	while (loop_running) {
		uint64_t remaining;
#ifdef IMPACK_WITH_COMPRESSION
		if (state->compression != COMPRESSION_NONE) {
			while (true) {
				uint64_t lenout;
				impack_compression_result_t res = impack_compress_read(&decompress_state, buf, &lenout);
				if (res == COMPRESSION_RES_ERROR) {
					goto cleanup;
				} else if (res == COMPRESSION_RES_AGAIN) {
					remaining = BUFSIZE;
					if (state->data_length < remaining) {
						remaining = state->data_length;
					}
					uint32_t padding = 0;
#ifdef IMPACK_WITH_CRYPTO
					if (state->encryption != ENCRYPTION_NONE) {
						if (remaining % IMPACK_CRYPT_BLOCK_SIZE != 0) {
							padding = IMPACK_CRYPT_BLOCK_SIZE - (remaining % IMPACK_CRYPT_BLOCK_SIZE);
							remaining += padding;
						}
					}
#endif
					if (!pixelbuf_read(state, buf, remaining)) {
						goto cleanup;
					}
#ifdef IMPACK_WITH_CRYPTO
					if (state->encryption != ENCRYPTION_NONE) {
						impack_decrypt(&decrypt_ctx, buf, remaining, state->encryption);
						if (state->legacy && remaining == state->data_length) {
							padding = buf[remaining - 1];
						}
						remaining -= padding;
					}
#endif
					impack_compress_write(&decompress_state, buf, remaining);
					if (!state->legacy) {
						impack_crc(&crc, buf, remaining);
					}
					state->data_length -= remaining;
					if (state->legacy) {
						state->data_length -= padding;
					}
				} else {
					if (res == COMPRESSION_RES_FINAL) {
						if (state->data_length != 0) {
							goto cleanup;
						}
						loop_running = false;
					}
					remaining = lenout;
					break;
				}
			}
		} else {
#endif
			remaining = BUFSIZE;
			if (state->data_length < remaining) {
				remaining = state->data_length;
			}
			uint32_t padding = 0;
#ifdef IMPACK_WITH_CRYPTO
			if (state->encryption != ENCRYPTION_NONE) {
				if (remaining % IMPACK_CRYPT_BLOCK_SIZE != 0) {
					padding = IMPACK_CRYPT_BLOCK_SIZE - (remaining % IMPACK_CRYPT_BLOCK_SIZE);
					remaining += padding;
				}
			}
#endif
			if (!pixelbuf_read(state, buf, remaining)) {
				goto cleanup;
			}
#ifdef IMPACK_WITH_CRYPTO
			if (state->encryption != ENCRYPTION_NONE) {
				impack_decrypt(&decrypt_ctx, buf, remaining, state->encryption);
				if (state->legacy && remaining == state->data_length) {
					padding = buf[remaining - 1];
				}
				remaining -= padding; // Don't write padding into the output file
			}
#endif
			if (!state->legacy) {
				impack_crc(&crc, buf, remaining);
			}
			state->data_length -= remaining;
			if (state->legacy) {
				state->data_length -= padding;
			}
			if (state->data_length == 0) {
				loop_running = false;
			}
#ifdef IMPACK_WITH_COMPRESSION
		}
#endif
		
#ifdef IMPACK_WITH_CRYPTO
		if (state->legacy) {
			sha512_update(&legacy_checksum, remaining, buf);
		}
#endif
		if (fwrite(buf, 1, remaining, output_file) != remaining) {
			ret = ERROR_OUTPUT_IO;
			goto cleanup;
		}
	}

#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != ENCRYPTION_NONE) {
		impack_secure_erase((uint8_t*) &decrypt_ctx, sizeof(impack_crypt_ctx_t));
	}
#endif
#ifdef IMPACK_WITH_COMPRESSION
	if (state->compression != COMPRESSION_NONE) {
		impack_compress_free(&decompress_state);
	}
#endif
	fclose(output_file);
	free(buf);
	free(state->pixeldata);
	if (!state->legacy) {
		if (crc != state->crc) {
			return ERROR_CRC;
		}
	} else {
#ifdef IMPACK_WITH_CRYPTO
		uint8_t legacy_checksum_res[SHA512_DIGEST_SIZE];
		sha512_digest(&legacy_checksum, SHA512_DIGEST_SIZE, legacy_checksum_res);
		if (memcmp(legacy_checksum_res, state->checksum_legacy, SHA512_DIGEST_SIZE) != 0) {
			return ERROR_CRC;
		}
#endif
	}
	return ERROR_OK;
	
cleanup:
	free(state->pixeldata);
	if (state->filename != NULL) {
		free(state->filename);
	}
	if (buf != NULL) {
		free(buf);
	}
	if (output_file != NULL) {
		fclose(output_file);
	}
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != ENCRYPTION_NONE) {
		impack_secure_erase(state->crypt_key, IMPACK_CRYPT_KEY_SIZE);
		impack_secure_erase((uint8_t*) &decrypt_ctx, sizeof(impack_crypt_ctx_t));
	}
#endif
#ifdef IMPACK_WITH_COMPRESSION
	if (decompress_state.bufsize != 0) {
		impack_compress_free(&decompress_state);
	}
#endif
	return ret;
	
}
