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
#include <nettle/aes.h>
#include <nettle/cbc.h>
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
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->pixeldata_pos = 3;

	uint8_t magic_buf[4];
	if (!pixelbuf_read(state, magic_buf, 4)) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	uint8_t magic[] = IMPACK_MAGIC_NUMBER;
	if (memcmp(magic_buf, magic, 3) != 0) {
		free(state->pixeldata);
		return ERROR_INPUT_IMG_INVALID;
	}
	state->legacy = false;
	if (magic_buf[3] != magic[3]) {
		if (magic_buf[3] == 97) { // 'a', could be a legacy file
			if (!pixelbuf_read(state, magic_buf, 2)) {
				free(state->pixeldata);
				return ERROR_INPUT_IMG_INVALID;
			}
			if (magic_buf[0] != 99) { // 'c'
				free(state->pixeldata);
				return ERROR_INPUT_IMG_INVALID;
			}
			state->legacy = true;
			if (magic_buf[1] == 107) { // 'k'
				state->compression = COMPRESSION_NONE;
			} else if (magic_buf[1] == 90) { // 'Z'
				state->compression = COMPRESSION_ZLIB;
			} else {
				free(state->pixeldata);
				return ERROR_INPUT_IMG_INVALID;
			}
		} else {
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}
	}
	
	if (!state->legacy) {
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
	} else {
		uint32_t data_length;
		if (!pixelbuf_read(state, (uint8_t*) &data_length, 4)) {
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}
		if (!pixelbuf_read(state, (uint8_t*) &state->filename_length, 4)) {
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}
		state->data_length = impack_endian32_le(data_length);
		state->filename_length = impack_endian32_le(state->filename_length);
		if (!pixelbuf_read(state, state->checksum_legacy, 64)) {
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}
		uint8_t flag;
		if (!pixelbuf_read(state, &flag, 1)) {
			free(state->pixeldata);
			return ERROR_INPUT_IMG_INVALID;
		}
		state->encryption = (flag == 255) ? 1 : 0;
	}
	
#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption > 1) {
		free(state->pixeldata);
		return ERROR_ENCRYPTION_UNKNOWN;
	}
#else
	if (state->encryption > 0) {
		free(state->pixeldata);
		return ERROR_ENCRYPTION_UNAVAILABLE;
	}
#endif
	
	if (state->compression > COMPRESSION_NONE) {
#ifdef IMPACK_WITH_COMPRESSION
		switch (state->compression) {
			case COMPRESSION_ZLIB:
#ifdef IMPACK_WITH_ZLIB
				break;
#else
				free(state->pixeldata);
				return ERROR_COMPRESSION_UNSUPPORTED;
#endif
			case COMPRESSION_ZSTD:
#ifdef IMPACK_WITH_ZSTD
				break;
#else
				free(state->pixeldata);
				return ERROR_COMPRESSION_UNSUPPORTED;
#endif
			case COMPRESSION_LZMA:
#ifdef IMPACK_WITH_LZMA
				break;
#else
				free(state->pixeldata);
				return ERROR_COMPRESSION_UNSUPPORTED;
#endif
			default:
				free(state->pixeldata);
				return ERROR_COMPRESSION_UNKNOWN;
		}
#else
		free(state->pixeldata);
		return ERROR_COMPRESSION_UNAVAILABLE;
#endif
	}
	
	return ERROR_OK;
	
}

impack_error_t impack_decode_stage2(impack_decode_state_t *state, char *passphrase) {
	
	if (!state->legacy) {
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
	}
	
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
	
	uint64_t filename_length;
#ifdef IMPACK_WITH_CRYPTO
	struct CBC_CTX(struct aes256_ctx, AES_BLOCK_SIZE) decrypt_ctx;
#endif
	if (!state->legacy) {
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
		filename_length = state->filename_length;
#ifdef IMPACK_WITH_CRYPTO
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
	} else {
		filename_length = state->filename_length;
#ifdef IMPACK_WITH_CRYPTO
		if (state->encryption != 0) {
			if (!pixelbuf_read(state, state->aes_iv, AES_BLOCK_SIZE)) {
				impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
				free(state->pixeldata);
				return ERROR_INPUT_IMG_INVALID;
			}
			uint8_t salt[32];
			if (!pixelbuf_read(state, salt, 32)) {
				impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
				free(state->pixeldata);
				return ERROR_INPUT_IMG_INVALID;
			}
			impack_derive_key_legacy(passphrase, state->aes_key, AES256_KEY_SIZE, salt, 32);
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
	}
	
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
	if (state->encryption != 0 && !state->legacy) {
		CBC_DECRYPT(&decrypt_ctx, aes256_decrypt, filename_length, (uint8_t*) state->filename, (uint8_t*) state->filename);
		memcpy(state->aes_iv, decrypt_ctx.iv, AES_BLOCK_SIZE);
		impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
	}
#endif
	
	for (uint32_t i = 0; i < state->filename_length; i++) {
		if (!isprint(state->filename[i])) { // Search for non-printable characters in the filename
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
	}
	
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
				newname[output_path_length] = '/'; // This should also work on windows
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
	
#ifdef IMPACK_WITH_COMPRESSION
	impack_compress_state_t decompress_state;
	if (state->compression != COMPRESSION_NONE) {
		decompress_state.type = state->compression;
		decompress_state.is_compress = false;
		decompress_state.bufsize = BUFSIZE;
		if (!impack_compress_init(&decompress_state)) {
#ifdef IMPACK_WITH_CRYPTO
			if (state->encryption != 0) {
				impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
			}
#endif
			free(state->pixeldata);
			fclose(output_file);
			return ERROR_MALLOC;
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
#ifdef IMPACK_WITH_CRYPTO
					if (state->encryption != 0) {
						impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
					}
#endif
					impack_compress_free(&decompress_state);
					free(state->pixeldata);
					free(buf);
					fclose(output_file);
					return ERROR_INPUT_IMG_INVALID;
				} else if (res == COMPRESSION_RES_AGAIN) {
					remaining = BUFSIZE;
					if (state->data_length < remaining) {
						remaining = state->data_length;
					}
					uint32_t padding = 0;
#ifdef IMPACK_WITH_CRYPTO
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
						impack_compress_free(&decompress_state);
						free(state->pixeldata);
						free(buf);
						fclose(output_file);
						return ERROR_INPUT_IMG_INVALID;
					}
#ifdef IMPACK_WITH_CRYPTO
					if (state->encryption != 0) {
						CBC_DECRYPT(&decrypt_ctx, aes256_decrypt, remaining, buf, buf);
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
#ifdef IMPACK_WITH_CRYPTO
							if (state->encryption != 0) {
								impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
							}
#endif
							impack_compress_free(&decompress_state);
							free(state->pixeldata);
							free(buf);
							fclose(output_file);
							return ERROR_INPUT_IMG_INVALID;
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
#ifdef IMPACK_WITH_CRYPTO
			if (state->encryption != 0) {
				impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
			}
#endif
#ifdef IMPACK_WITH_COMPRESSION
			if (state->compression != COMPRESSION_NONE) {
				impack_compress_free(&decompress_state);
			}
#endif
			free(state->pixeldata);
			free(buf);
			fclose(output_file);
			return ERROR_OUTPUT_IO;
		}
	}

#ifdef IMPACK_WITH_CRYPTO
	if (state->encryption != 0) {
		impack_secure_erase((uint8_t*) &decrypt_ctx.ctx, sizeof(struct aes256_ctx));
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
	
}
