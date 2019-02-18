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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef IMPACK_WITH_CRYPTO
#include <nettle/aes.h>
#include <nettle/cbc.h>
#endif
#include "impack.h"
#include "impack_internal.h"

#define BUFSIZE 16384 // 16 KiB
#define PIXELBUF_STEP 1048576 // 1 MiB

bool pixelbuf_add(uint8_t **pixeldata, uint64_t *pixeldata_size, uint64_t *pixeldata_pos, uint8_t channels, uint8_t *data, uint64_t len) {
	
	while (len > 0) {
		if (*pixeldata_pos >= *pixeldata_size - 3) { // Keep space for one full pixel
			uint8_t *newbuf = realloc(*pixeldata, *pixeldata_size + PIXELBUF_STEP);
			if (newbuf == NULL) {
				return false;
			}
			*pixeldata = newbuf;
			*pixeldata_size += PIXELBUF_STEP;
		}
		
		if (channels != 0) {
			if (((*pixeldata_pos % 3 == 0) && (channels & CHANNEL_RED)) || \
				((*pixeldata_pos % 3 == 1) && (channels & CHANNEL_GREEN)) || \
				((*pixeldata_pos % 3 == 2) && (channels & CHANNEL_BLUE))) { // Current channel enabled
				(*pixeldata)[*pixeldata_pos] = *data;
				data++;
				len--;
			}
			(*pixeldata_pos)++;
		} else { // Grayscale mode
			(*pixeldata)[*pixeldata_pos] = *data;
			(*pixeldata)[*pixeldata_pos + 1] = *data;
			(*pixeldata)[*pixeldata_pos + 2] = *data;
			data++;
			len--;
			(*pixeldata_pos) += 3;
		}
	}
	return true;
	
}

impack_error_t impack_encode(char *input_path, char *output_path, bool encrypt, char *passphrase, impack_compression_type_t compress, uint8_t channels, uint64_t img_width, uint64_t img_height, impack_img_format_t format) {
	
	FILE *input_file, *output_file;
	if (strlen(input_path) == 1 && input_path[0] == '-') {
		input_file = stdin;
	} else {
		input_file = fopen(input_path, "rb");
		if (input_file == NULL) {
#ifdef IMPACK_WITH_CRYPTO
			if (encrypt) {
				impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
			}
#endif
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
	if (strlen(output_path) == 1 && output_path[0] == '-') {
		output_file = stdout;
	} else {
		output_file = fopen(output_path, "wb");
		if (output_file == NULL) {
			fclose(input_file);
#ifdef IMPACK_WITH_CRYPTO
			if (encrypt) {
				impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
			}
#endif
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

	uint8_t *input_buf = malloc(BUFSIZE);
	if (input_buf == NULL) {
#ifdef IMPACK_WITH_CRYPTO
		if (encrypt) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		fclose(input_file);
		fclose(output_file);
		return ERROR_MALLOC;
	}
	uint8_t *pixeldata = malloc(PIXELBUF_STEP);
	if (pixeldata == NULL) {
#ifdef IMPACK_WITH_CRYPTO
		if (encrypt) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		}
#endif
		free(input_buf);
		fclose(input_file);
		fclose(output_file);
		return ERROR_MALLOC;
	}
	uint64_t pixeldata_size = PIXELBUF_STEP;
	uint64_t pixeldata_pos = 3;
	
	pixeldata[0] = ((channels & CHANNEL_RED) != 0) ? 255 : 0;
	pixeldata[1] = ((channels & CHANNEL_GREEN) != 0) ? 255 : 0;
	pixeldata[2] = ((channels & CHANNEL_BLUE) != 0) ? 255 : 0;
	
	uint8_t magic[] = IMPACK_MAGIC_NUMBER;
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, magic, 4); // These will not fail, the buffer is large enough
	uint8_t format_version = IMPACK_FORMAT_VERSION;
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &format_version, 1);
	uint8_t encryption_flag = (encrypt ? 1 : 0);
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &encryption_flag, 1);
	uint8_t compression_flag = compress;
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &compression_flag, 1);
	uint64_t length_offset = pixeldata_pos;
	for (int i = 0; i < 8; i++) { // Add a dummy value that will be replaced when the length is known
		uint8_t dummy = 0;
		pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &dummy, 1);
	}
	char *input_filename = impack_filename(input_path);
	uint32_t input_filename_length = strlen(input_filename);
	uint32_t input_filename_length_endian = impack_endian32(input_filename_length);
	pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, (uint8_t*) &input_filename_length_endian, 4);
	uint64_t crc_offset = pixeldata_pos;
	for (int i = 0; i < 8; i++) { // More dummy bytes for the CRC
		uint8_t dummy = 0;
		pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, &dummy, 1);
	}
	
#ifdef IMPACK_WITH_CRYPTO
	struct CBC_CTX(struct aes256_ctx, AES_BLOCK_SIZE) encrypt_ctx;
	if (encrypt) {
		if (!impack_random(encrypt_ctx.iv, AES_BLOCK_SIZE)) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
			free(pixeldata);
			free(input_buf);
			fclose(input_file);
			fclose(output_file);
			return ERROR_RANDOM;
		}
		if (!pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, encrypt_ctx.iv, AES_BLOCK_SIZE)) {
			impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
			free(pixeldata);
			free(input_buf);
			fclose(input_file);
			fclose(output_file);
			return ERROR_MALLOC;
		}
		uint8_t key[AES256_KEY_SIZE];
		impack_derive_key(passphrase, key, AES256_KEY_SIZE, encrypt_ctx.iv, AES_BLOCK_SIZE);
		aes256_set_encrypt_key(&encrypt_ctx.ctx, key);
		impack_secure_erase((uint8_t*) passphrase, strlen(passphrase));
		impack_secure_erase(key, AES256_KEY_SIZE);
	}
#endif

	char *input_filename_add = input_filename;
	uint64_t input_filename_add_length = input_filename_length;
#ifdef IMPACK_WITH_CRYPTO
	if (encrypt) {
		if (input_filename_length % AES_BLOCK_SIZE != 0) {
			uint32_t padding = AES_BLOCK_SIZE - (input_filename_length % AES_BLOCK_SIZE);
			char *input_filename_padded = malloc(input_filename_length + padding);
			if (input_filename_padded == NULL) {
				impack_secure_erase((uint8_t*) &encrypt_ctx.ctx, sizeof(struct aes256_ctx));
				free(pixeldata);
				free(input_buf);
				fclose(input_file);
				fclose(output_file);
				return ERROR_MALLOC;
			}
			strncpy(input_filename_padded, input_filename, input_filename_length);
			memset(input_filename_padded + input_filename_length, 0, padding);
			input_filename_add = input_filename_padded;
			input_filename_add_length += padding;
		}
		CBC_ENCRYPT(&encrypt_ctx, aes256_encrypt, input_filename_add_length, (uint8_t*) input_filename_add, (uint8_t*) input_filename_add);
	}
#endif
	if (!pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, (uint8_t*) input_filename_add, input_filename_add_length)) {
#ifdef IMPACK_WITH_CRYPTO
		if (encrypt) {
			impack_secure_erase((uint8_t*) &encrypt_ctx.ctx, sizeof(struct aes256_ctx));
		}
#endif
		free(pixeldata);
		free(input_buf);
		if (input_filename_add != input_filename) {
			free(input_filename_add);
		}
		fclose(input_file);
		fclose(output_file);
		return ERROR_MALLOC;
	}
	if (input_filename_add != input_filename) {
		free(input_filename_add);
	}
	
#ifdef IMPACK_WITH_COMPRESSION
	impack_compress_state_t compress_state;
	if (compress != COMPRESSION_NONE) {
		compress_state.type = compress;
		compress_state.is_compress = true;
		compress_state.bufsize = BUFSIZE;
		if (!impack_compress_init(&compress_state)) {
#ifdef IMPACK_WITH_CRYPTO
			if (encrypt) {
				impack_secure_erase((uint8_t*) &encrypt_ctx.ctx, sizeof(struct aes256_ctx));
			}
#endif
			free(pixeldata);
			free(input_buf);
			fclose(input_file);
			fclose(output_file);
			return ERROR_MALLOC;
		}
	}
	bool file_read_done = false;
#endif
	
	impack_crc_init();
	uint64_t crc = 0;
	uint64_t data_length = 0;
	size_t bytes_read;
	bool loop_running = true;
	do {
#ifdef IMPACK_WITH_COMPRESSION
		if (compress != COMPRESSION_NONE) {
			while (true) {
				if (file_read_done) {
					uint64_t flushlen;
					if (impack_compress_flush(&compress_state, input_buf, &flushlen) == COMPRESSION_RES_FINAL) {
						bytes_read = flushlen;
						loop_running = false;
						break;
					} else {
						bytes_read = BUFSIZE;
						break;
					}
				} else {
					uint64_t dummy;
					if (impack_compress_read(&compress_state, input_buf, &dummy) == COMPRESSION_RES_AGAIN) {
						bytes_read = fread(input_buf, 1, BUFSIZE, input_file);
						impack_compress_write(&compress_state, input_buf, bytes_read);
						if (bytes_read != BUFSIZE) {
							file_read_done = true;
						}
					} else {
						bytes_read = BUFSIZE;
						break;
					}
				}
			}
		} else {
#endif
			bytes_read = fread(input_buf, 1, BUFSIZE, input_file);
#ifdef IMPACK_WITH_COMPRESSION
		}
#endif
		data_length += bytes_read;
		impack_crc(&crc, input_buf, bytes_read);
#ifdef IMPACK_WITH_CRYPTO
		if (encrypt) {
			if (bytes_read % AES_BLOCK_SIZE != 0) {
				uint32_t padding = AES_BLOCK_SIZE - (bytes_read % AES_BLOCK_SIZE);
				memset(input_buf + bytes_read, 0, padding); // The buffer size is a multiple of the block size, there is always enough space for padding when it's needed
				bytes_read += padding;
			}
			CBC_ENCRYPT(&encrypt_ctx, aes256_encrypt, bytes_read, input_buf, input_buf);
		}
#endif
		if (!pixelbuf_add(&pixeldata, &pixeldata_size, &pixeldata_pos, channels, input_buf, bytes_read)) {
#ifdef IMPACK_WITH_CRYPTO
			if (encrypt) {
				impack_secure_erase((uint8_t*) &encrypt_ctx.ctx, sizeof(struct aes256_ctx));
			}
#endif
			free(pixeldata);
			free(input_buf);
			fclose(input_file);
			fclose(output_file);
			return ERROR_MALLOC;
		}
	} while (bytes_read == BUFSIZE && loop_running);
	free(input_buf);
#ifdef IMPACK_WITH_CRYPTO
	if (encrypt) {
		impack_secure_erase((uint8_t*) &encrypt_ctx.ctx, sizeof(struct aes256_ctx));
	}
#endif
	if (!feof(input_file)) {
		free(pixeldata);
		fclose(input_file);
		fclose(output_file);
		return ERROR_INPUT_IO;
	}
	fclose(input_file);

	data_length = impack_endian64(data_length);
	pixelbuf_add(&pixeldata, &pixeldata_size, &length_offset, channels, (uint8_t*) &data_length, 8);
	crc = impack_endian64(crc);
	pixelbuf_add(&pixeldata, &pixeldata_size, &crc_offset, channels, (uint8_t*) &crc, 8);

	impack_error_t res = impack_write_img(output_path, output_file, &pixeldata, pixeldata_size, pixeldata_pos, img_width, img_height, format);
	free(pixeldata);
	fclose(output_file);
	return res;
	
}
