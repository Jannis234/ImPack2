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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "impack.h"
#include "config.h"

extern void impack_build_info();

#define REF_LENGTH 256
#define PASSPHRASE_CORRECT "123456"
#define PASSPHRASE_INCORRECT "abcdef"
#define PASSPHRASE_LEN 6
uint8_t ref_file[REF_LENGTH];
char namebuf[100];

void print_error(impack_error_t error) {
	
	switch (error) {
		case ERROR_INPUT_NOT_FOUND:
			printf("Input not found\n");
			return;
		case ERROR_INPUT_PERMISSION:
			printf("Input permission denied\n");
			return;
		case ERROR_INPUT_DIRECTORY:
			printf("Input is directory\n");
			return;
		case ERROR_INPUT_IO:
			printf("Input I/O error\n");
			return;
		case ERROR_OUTPUT_NOT_FOUND:
			printf("Output not found\n");
			return;
		case ERROR_OUTPUT_PERMISSION:
			printf("Output permission denied\n");
			return;
		case ERROR_OUTPUT_DIRECTORY:
			printf("Output is directory\n");
			return;
		case ERROR_OUTPUT_IO:
			printf("Output I/O error\n");
			return;
		case ERROR_MALLOC:
			printf("Out of memory\n");
			return;
		case ERROR_RANDOM:
			printf("Random read failed\n");
			return;
		case ERROR_IMG_SIZE:
			printf("Invalid size for format\n");
			return;
		case ERROR_IMG_TOO_SMALL:
			printf("Size too small\n");
			return;
		case ERROR_IMG_FORMAT_UNSUPPORTED:
			printf("Format not compiled in\n");
			return;
		case ERROR_IMG_FORMAT_UNKNOWN:
			printf("Format unknown\n");
			return;
		case ERROR_INPUT_IMG_INVALID:
			printf("Parse error\n");
			return;
		case ERROR_INPUT_IMG_VERSION:
			printf("Incompatible version\n");
			return;
		case ERROR_CRC:
			printf("CRC error\n");
			return;
		case ERROR_ENCRYPTION_UNAVAILABLE:
			printf("Encryption not compiled in\n");
			return;
		case ERROR_ENCRYPTION_UNSUPPORTED:
			printf("Encryption type not compiled in\n");
			return;
		case ERROR_ENCRYPTION_UNKNOWN:
			printf("Encryption unknown\n");
			return;
		case ERROR_COMPRESSION_UNAVAILABLE:
			printf("Compression not compiled in\n");
			return;
		case ERROR_COMPRESSION_UNSUPPORTED:
			printf("Compression type not compiled in\n");
			return;
		case ERROR_COMPRESSION_UNKNOWN:
			printf("Compression unknown\n");
			return;
		case ERROR_OK:
			break;
	}
	abort(); // Should never get here
	
}

bool decode_run(char *input_file, char *passphrase, bool shouldfail) {
	
	impack_decode_state_t state;
	impack_error_t res = impack_decode_stage1(&state, input_file);
	if (res != ERROR_OK) {
		if (shouldfail) {
			printf("OK\n");
			return true;
		} else {
			printf("Error\n");
			printf("  Unexpected error after decode stage 1: ");
			print_error(res);
			return false;
		}
	}
	res = impack_decode_stage2(&state, passphrase);
	if (res != ERROR_OK) {
		if (shouldfail) {
			printf("OK\n");
			return true;
		} else {
			printf("Error\n");
			printf("  Unexpected error after decode stage 2: ");
			print_error(res);
			return false;
		}
	}
	bool nameok = (strncmp(state.filename, "input.bin", state.filename_length) == 0);
	res = impack_decode_stage3(&state, "testout_decode.tmp");
	if (res != ERROR_OK) {
		if (shouldfail) {
			printf("OK\n");
			return true;
		} else {
			printf("Error\n");
			printf("  Unexpected error after decode stage 3: ");
			print_error(res);
			return false;
		}
	}
	
	bool printerror = true;
	bool haserror = false;
	if (shouldfail) {
		printf("Error\n");
		printerror = false;
		printf("  Decode ok, expected failure\n");
		haserror = true;
	}
	if (!nameok) {
		if (printerror) {
			printf("Error\n");
			printerror = false;
		}
		printf("  Filename incorrect\n");
		haserror = true;
	}
	char buf[REF_LENGTH];
	FILE *f = fopen("testout_decode.tmp", "rb");
	if (f == NULL) {
		if (printerror) {
			printf("Error\n");
			printerror = false;
		}
		printf("  Unable to open testout_decode.tmp\n");
		return false;
	}
	int32_t bytes_read = fread(buf, 1, REF_LENGTH, f);
	fclose(f);
	if (bytes_read != REF_LENGTH) {
		if (printerror) {
			printf("Error\n");
			printerror = false;
		}
		printf("  Unable to read testout_decode.tmp, got %d bytes\n", bytes_read);
		return false;
	}
	if (memcmp(buf, ref_file, REF_LENGTH) != 0) {
		if (printerror) {
			printf("Error\n");
		}
		printf("  Data incorrect\n");
		haserror = true;
	}
	
	if (haserror) {
		return false;
	} else {
		printf("OK\n");
		return true;
	}
	
}

bool test_decode_format_run(char *msg, char *filename, char *passphrase, bool shouldfail, char *format_name, char *fileextension) {
	
	printf("%s, %s: ", msg, format_name);
	strcpy(namebuf + strlen(filename), fileextension);
	char *passarg = NULL;
	char passbuf[PASSPHRASE_LEN + 1];
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	return decode_run(namebuf, passarg, shouldfail);
	
}

bool test_decode_format(char *msg, char *filename, char *passphrase, bool shouldfail) {
	
	bool res = true;
	strcpy(namebuf, filename);
	int i = 0;
	while (impack_img_formats[i] != NULL) {
		const impack_img_format_desc_t *current = impack_img_formats[i];
		res &= test_decode_format_run(msg, filename, passphrase, shouldfail, current->name, current->extension + 1);
		i++;
	}
	return res;
	
}

bool encode_run(impack_img_format_t format, impack_encryption_type_t encrypt, char *passphrase, impack_compression_type_t compress, uint64_t width, uint64_t height, uint8_t channels) {
	
	impack_error_t res = impack_encode("testdata/input.bin", "testout_encode.tmp", encrypt, passphrase, compress, 0, channels, width, height, format, "testdata/input.bin");
	if (res != ERROR_OK) {
		printf("Error\n");
		printf("  Unexpected error after encode: ");
		print_error(res);
		return false;
	}
	return true;
	
}

bool test_cycle_format_run(char *msg, impack_encryption_type_t encrypt, char *passphrase, impack_compression_type_t compress, uint64_t width, uint64_t height, uint8_t channels, impack_img_format_t format, char *format_name) {
	
	printf("%s, %s: ", msg, format_name);
	char *passarg = NULL;
	char passbuf[PASSPHRASE_LEN + 1];
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	if (!encode_run(format, encrypt, passarg, compress, width, height, channels)) {
		return false;
	} else {
		if (passphrase != NULL) {
			strcpy(passbuf, passphrase);
		}
		return decode_run("testout_encode.tmp", passarg, false);
	}
	
}

bool test_cycle_format(char *msg, impack_encryption_type_t encrypt, char *passphrase, impack_compression_type_t compress, uint64_t width, uint64_t height, uint8_t channels) {
	
	bool res = true;
	int i = 0;
	while (impack_img_formats[i] != NULL) {
		const impack_img_format_desc_t *current = impack_img_formats[i];
		res &= test_cycle_format_run(msg, encrypt, passphrase, compress, width, height, channels, current->id, current->name);
		i++;
	}
	return res;
	
}

bool test_cycle() {
	
	bool res = true;
	uint8_t allchannels = CHANNEL_RED | CHANNEL_GREEN | CHANNEL_BLUE;
	res &= test_cycle_format("Default settings", false, NULL, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Red color channel", false, NULL, COMPRESSION_NONE, 0, 0, CHANNEL_RED);
	res &= test_cycle_format("Green color channel", false, NULL, COMPRESSION_NONE, 0, 0, CHANNEL_GREEN);
	res &= test_cycle_format("Blue color channel", false, NULL, COMPRESSION_NONE, 0, 0, CHANNEL_BLUE);
	res &= test_cycle_format("Red + green color channels", false, NULL, COMPRESSION_NONE, 0, 0, CHANNEL_RED | CHANNEL_GREEN);
	res &= test_cycle_format("Red + blue color channels", false, NULL, COMPRESSION_NONE, 0, 0, CHANNEL_RED | CHANNEL_BLUE);
	res &= test_cycle_format("Green + blue color channels", false, NULL, COMPRESSION_NONE, 0, 0, CHANNEL_GREEN | CHANNEL_BLUE);
	res &= test_cycle_format("Grayscale mode", false, NULL, COMPRESSION_NONE, 0, 0, 0);
	res &= test_cycle_format("Custom width", false, NULL, COMPRESSION_NONE, 2, 0, allchannels);
	res &= test_cycle_format("Custom height", false, NULL, COMPRESSION_NONE, 0, 2, allchannels);
	res &= test_cycle_format("Custom width + height", false, NULL, COMPRESSION_NONE, 50, 50, allchannels);
	
	int baselen = strlen("Encrypted and compressed data, Camellia encryption, PBKDF2,  compression"); // Maximum length encryption name
	int namelen = 0;
	int current = 0;
#ifdef IMPACK_WITH_COMPRESSION
	while (impack_compression_types[current] != NULL) {
		if (strlen(impack_compression_types[current]->name) > namelen) {
			namelen = strlen(impack_compression_types[current]->name);
		}
		current++;
	}
#endif
	char namebuf[baselen + namelen + 1];
#ifdef IMPACK_WITH_COMPRESSION
	current = 0;
	while (impack_compression_types[current] != NULL) {
		sprintf(namebuf, "Compressed data, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, false, NULL, impack_compression_types[current]->id, 0, 0, allchannels);
		current++;
	}
#endif
#ifdef IMPACK_WITH_CRYPTO
	res &= test_cycle_format("Encrypted data, AES encryption, PBKDF2", ENCRYPTION_AES, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Encrypted data, Camellia encryption, PBKDF2", ENCRYPTION_CAMELLIA, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Encrypted data, Serpent encryption, PBKDF2", ENCRYPTION_SERPENT, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Encrypted data, Twofish encryption, PBKDF2", ENCRYPTION_TWOFISH, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
#ifdef IMPACK_WITH_ARGON2
	res &= test_cycle_format("Encrypted data, AES encryption, Argon2", ENCRYPTION_AES_ARGON2, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Encrypted data, Camellia encryption, Argon2", ENCRYPTION_CAMELLIA_ARGON2, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Encrypted data, Serpent encryption, Argon2", ENCRYPTION_SERPENT_ARGON2, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
	res &= test_cycle_format("Encrypted data, Twofish encryption, Argon2", ENCRYPTION_TWOFISH_ARGON2, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
#endif
#ifdef IMPACK_WITH_COMPRESSION
	current = 0;
	while (impack_compression_types[current] != NULL) {
		sprintf(namebuf, "Encrypted and compressed data, AES encryption, PBKDF2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_AES, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
		sprintf(namebuf, "Encrypted and compressed data, Camellia encryption, PBKDF2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_CAMELLIA, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
		sprintf(namebuf, "Encrypted and compressed data, Serpent encryption, PBKDF2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_SERPENT, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
		sprintf(namebuf, "Encrypted and compressed data, Twofish encryption, PBKDF2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_TWOFISH, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
#ifdef IMPACK_WITH_ARGON2
		sprintf(namebuf, "Encrypted and compressed data, AES encryption, Argon2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_AES_ARGON2, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
		sprintf(namebuf, "Encrypted and compressed data, Camellia encryption, Argon2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_CAMELLIA_ARGON2, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
		sprintf(namebuf, "Encrypted and compressed data, Serpent encryption, Argon2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_SERPENT_ARGON2, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
		sprintf(namebuf, "Encrypted and compressed data, Twofish encryption, Argon2, %s compression", impack_compression_types[current]->name);
		res &= test_cycle_format(namebuf, ENCRYPTION_TWOFISH_ARGON2, PASSPHRASE_CORRECT, impack_compression_types[current]->id, 0, 0, allchannels);
#endif
		current++;
	}
#endif
#endif
	return res;
	
}

bool test_decode() {
	
	bool res = true;
	
	res &= test_decode_format("Default settings", "testdata/valid", NULL, false);
	
	// Color channels
	res &= test_decode_format("Red color channel", "testdata/valid_channel_red", NULL, false);
	res &= test_decode_format("Green color channel", "testdata/valid_channel_green", NULL, false);
	res &= test_decode_format("Blue color channel", "testdata/valid_channel_blue", NULL, false);
	res &= test_decode_format("Red + green color channels", "testdata/valid_channel_red_green", NULL, false);
	res &= test_decode_format("Red + blue color channels", "testdata/valid_channel_red_blue", NULL, false);
	res &= test_decode_format("Green + blue color channels", "testdata/valid_channel_green_blue", NULL, false);
	res &= test_decode_format("Grayscale mode", "testdata/valid_grayscale", NULL, false);
	
	// Encryption
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Encrypted data, AES encryption, PBKDF2", "testdata/valid_encrypted_aes_pbkdf2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, Camellia encryption, PBKDF2", "testdata/valid_encrypted_camellia_pbkdf2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, Serpent encryption, PBKDF2", "testdata/valid_encrypted_serpent_pbkdf2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, Twofish encryption, PBKDF2", "testdata/valid_encrypted_twofish_pbkdf2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, AES encryption, PBKDF2, incorrect passphrase", "testdata/valid_encrypted_aes_pbkdf2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted data, Camellia encryption, PBKDF2, incorrect passphrase", "testdata/valid_encrypted_camellia_pbkdf2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted data, Serpent encryption, PBKDF2, incorrect passphrase", "testdata/valid_encrypted_serpent_pbkdf2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted data, Twofish encryption, PBKDF2, incorrect passphrase", "testdata/valid_encrypted_twofish_pbkdf2", PASSPHRASE_INCORRECT, true);
#ifdef IMPACK_WITH_ARGON2
	res &= test_decode_format("Encrypted data, AES encryption, Argon2", "testdata/valid_encrypted_aes_argon2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, Camellia encryption, Argon2", "testdata/valid_encrypted_camellia_argon2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, Serpent encryption, Argon2", "testdata/valid_encrypted_serpent_argon2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, Twofish encryption, Argon2", "testdata/valid_encrypted_twofish_argon2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, AES encryption, Argon2, incorrect passphrase", "testdata/valid_encrypted_aes_argon2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted data, Camellia encryption, Argon2, incorrect passphrase", "testdata/valid_encrypted_camellia_argon2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted data, Serpent encryption, Argon2, incorrect passphrase", "testdata/valid_encrypted_serpent_argon2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted data, Twofish encryption, Argon2, incorrect passphrase", "testdata/valid_encrypted_twofish_argon2", PASSPHRASE_INCORRECT, true);
#endif
#endif
	
	// Compression
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Compressed data, deflate compression", "testdata/valid_compressed_deflate", NULL, false);
	res &= test_decode_format("Legacy image, compressed data", "testdata/valid_legacy_compressed", NULL, false);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Compressed data, ZSTD compression", "testdata/valid_compressed_zstd", NULL, false);
	//res &= test_decode_format("Corrupted compressed data, ZSTD compression", "testdata/invalid_compressed_zstd", NULL, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Compressed data, LZMA2 compression", "testdata/valid_compressed_lzma2", NULL, false);
	//res &= test_decode_format("Corrupted compressed data, LZMA2 compression", "testdata/invalid_compressed_lzma2", NULL, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Compressed data, Bzip2 compression", "testdata/valid_compressed_bzip2", NULL, false);
	//res &= test_decode_format("Corrupted compressed data, Bzip2 compression", "testdata/invalid_compressed_bzip2", NULL, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Compressed data, Brotli compression", "testdata/valid_compressed_brotli", NULL, false);
	//res &= test_decode_format("Corrupted compressed data, Brotli compression", "testdata/invalid_compressed_brotli", NULL, true);
#endif
	
	// Encryption + compression
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Encrypted and compressed data, AES encryption, deflate compression", "testdata/valid_encrypted_aes_compressed_deflate", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, AES encryption, deflate compression, incorrect passphrase", "testdata/valid_encrypted_aes_compressed_deflate", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, deflate compression", "testdata/valid_encrypted_camellia_compressed_deflate", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, deflate compression, incorrect passphrase", "testdata/valid_encrypted_camellia_compressed_deflate", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, deflate compression", "testdata/valid_encrypted_serpent_compressed_deflate", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, deflate compression, incorrect passphrase", "testdata/valid_encrypted_serpent_compressed_deflate", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, deflate compression", "testdata/valid_encrypted_twofish_compressed_deflate", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, deflate compression, incorrect passphrase", "testdata/valid_encrypted_twofish_compressed_deflate", PASSPHRASE_INCORRECT, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Encrypted and compressed data, AES encryption, ZSTD compression", "testdata/valid_encrypted_aes_compressed_zstd", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, AES encryption, ZSTD compression, incorrect passphrase", "testdata/valid_encrypted_aes_compressed_zstd", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, ZSTD compression", "testdata/valid_encrypted_camellia_compressed_zstd", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, ZSTD compression, incorrect passphrase", "testdata/valid_encrypted_camellia_compressed_zstd", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, ZSTD compression", "testdata/valid_encrypted_serpent_compressed_zstd", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, ZSTD compression, incorrect passphrase", "testdata/valid_encrypted_serpent_compressed_zstd", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, ZSTD compression", "testdata/valid_encrypted_twofish_compressed_zstd", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, ZSTD compression, incorrect passphrase", "testdata/valid_encrypted_twofish_compressed_zstd", PASSPHRASE_INCORRECT, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Encrypted and compressed data, AES encryption, LZMA2 compression", "testdata/valid_encrypted_aes_compressed_lzma2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, AES encryption, LZMA2 compression, incorrect passphrase", "testdata/valid_encrypted_aes_compressed_lzma2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, LZMA2 compression", "testdata/valid_encrypted_camellia_compressed_lzma2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, LZMA2 compression, incorrect passphrase", "testdata/valid_encrypted_camellia_compressed_lzma2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, LZMA2 compression", "testdata/valid_encrypted_serpent_compressed_lzma2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, LZMA2 compression, incorrect passphrase", "testdata/valid_encrypted_serpent_compressed_lzma2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, LZMA2 compression", "testdata/valid_encrypted_twofish_compressed_lzma2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, LZMA2 compression, incorrect passphrase", "testdata/valid_encrypted_twofish_compressed_lzma2", PASSPHRASE_INCORRECT, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Encrypted and compressed data, AES encryption, Bzip2 compression", "testdata/valid_encrypted_aes_compressed_bzip2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, AES encryption, Bzip2 compression, incorrect passphrase", "testdata/valid_encrypted_aes_compressed_bzip2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, Bzip2 compression", "testdata/valid_encrypted_camellia_compressed_bzip2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, Bzip2 compression, incorrect passphrase", "testdata/valid_encrypted_camellia_compressed_bzip2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, Bzip2 compression", "testdata/valid_encrypted_serpent_compressed_bzip2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, Bzip2 compression, incorrect passphrase", "testdata/valid_encrypted_serpent_compressed_bzip2", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, Bzip2 compression", "testdata/valid_encrypted_twofish_compressed_bzip2", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, Bzip2 compression, incorrect passphrase", "testdata/valid_encrypted_twofish_compressed_bzip2", PASSPHRASE_INCORRECT, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Encrypted and compressed data, AES encryption, Brotli compression", "testdata/valid_encrypted_aes_compressed_brotli", PASSPHRASE_CORRECT, false);
	//res &= test_decode_format("Encrypted and compressed data, AES encryption, Brotli compression, incorrect passphrase", "testdata/valid_encrypted_aes_compressed_brotli", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Camellia encryption, Brotli compression", "testdata/valid_encrypted_camellia_compressed_brotli", PASSPHRASE_CORRECT, false);
	//res &= test_decode_format("Encrypted and compressed data, Camellia encryption, Brotli compression, incorrect passphrase", "testdata/valid_encrypted_camellia_compressed_brotli", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Serpent encryption, Brotli compression", "testdata/valid_encrypted_serpent_compressed_brotli", PASSPHRASE_CORRECT, false);
	//res &= test_decode_format("Encrypted and compressed data, Serpent encryption, Brotli compression, incorrect passphrase", "testdata/valid_encrypted_serpent_compressed_brotli", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Encrypted and compressed data, Twofish encryption, Brotli compression", "testdata/valid_encrypted_twofish_compressed_brotli", PASSPHRASE_CORRECT, false);
	//res &= test_decode_format("Encrypted and compressed data, Twofish encryption, Brotli compression, incorrect passphrase", "testdata/valid_encrypted_twofish_compressed_brotli", PASSPHRASE_INCORRECT, true);
#endif
#endif
	
	// Legacy
	res &= test_decode_format("Legacy image", "testdata/valid_legacy", NULL, false);
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Legacy image, encrypted data", "testdata/valid_legacy_encrypted", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Legacy image, encrypted data, incorrect passphrase", "testdata/valid_legacy_encrypted", PASSPHRASE_INCORRECT, true);
#endif
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Legacy image, compressed data", "testdata/valid_legacy_compressed", NULL, false);
#endif
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Legacy image, encrypted and compressed data", "testdata/valid_legacy_encrypted_compressed", PASSPHRASE_CORRECT, false);
#endif
#endif
	
	// Truncated images
	res &= test_decode_format("Truncated image", "testdata/invalid_truncated_1", NULL, true);
	res &= test_decode_format("Truncated image", "testdata/invalid_truncated_2", NULL, true);
	res &= test_decode_format("Truncated image", "testdata/invalid_truncated_3", NULL, true);
	res &= test_decode_format("Truncated image", "testdata/invalid_truncated_4", NULL, true);
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Truncated encrypted image, AES encryption", "testdata/invalid_encrypted_aes_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted image, Camellia encryption", "testdata/invalid_encrypted_camellia_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted image, Serpent encryption", "testdata/invalid_encrypted_serpent_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted image, Twofish encryption", "testdata/invalid_encrypted_twofish_truncated", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Truncated compressed image, deflate compression", "testdata/invalid_compressed_deflate_truncated", NULL, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Truncated compressed image, ZSTD compression", "testdata/invalid_compressed_zstd_truncated", NULL, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Truncated compressed image, LZMA2 compression", "testdata/invalid_compressed_lzma2_truncated", NULL, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Truncated compressed image, BZIP2 compression", "testdata/invalid_compressed_bzip2_truncated", NULL, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Truncated compressed image, Brotli compression", "testdata/invalid_compressed_brotli_truncated", NULL, true);
#endif
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Truncated encrypted and compressed image, AES encryption, deflate compression", "testdata/invalid_encrypted_aes_compressed_deflate_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Camellia encryption, deflate compression", "testdata/invalid_encrypted_camellia_compressed_deflate_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Serpent encryption, deflate compression", "testdata/invalid_encrypted_serpent_compressed_deflate_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Twofish encryption, deflate compression", "testdata/invalid_encrypted_twofish_compressed_deflate_truncated", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Truncated encrypted and compressed image, AES encryption ZSTD compression", "testdata/invalid_encrypted_aes_compressed_zstd_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Camellia encryption ZSTD compression", "testdata/invalid_encrypted_camellia_compressed_zstd_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Serpent encryption ZSTD compression", "testdata/invalid_encrypted_serpent_compressed_zstd_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Twofish encryption ZSTD compression", "testdata/invalid_encrypted_twofish_compressed_zstd_truncated", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Truncated encrypted and compressed image, AES encryption, LZMA2 compression", "testdata/invalid_encrypted_aes_compressed_lzma2_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Camellia encryption, LZMA2 compression", "testdata/invalid_encrypted_camellia_compressed_lzma2_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Serpent encryption, LZMA2 compression", "testdata/invalid_encrypted_serpent_compressed_lzma2_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Twofish encryption, LZMA2 compression", "testdata/invalid_encrypted_twofish_compressed_lzma2_truncated", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Truncated encrypted and compressed image, AES encryption, BZIP2 compression", "testdata/invalid_encrypted_aes_compressed_bzip2_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Camellia encryption, BZIP2 compression", "testdata/invalid_encrypted_camellia_compressed_bzip2_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Serpent encryption, BZIP2 compression", "testdata/invalid_encrypted_serpent_compressed_bzip2_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Twofish encryption, BZIP2 compression", "testdata/invalid_encrypted_twofish_compressed_bzip2_truncated", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Truncated encrypted and compressed image, AES encryption, Brotli compression", "testdata/invalid_encrypted_aes_compressed_brotli_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Camellia encryption, Brotli compression", "testdata/invalid_encrypted_camellia_compressed_brotli_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Serpent encryption, Brotli compression", "testdata/invalid_encrypted_serpent_compressed_brotli_truncated", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Truncated encrypted and compressed image, Twofish encryption, Brotli compression", "testdata/invalid_encrypted_twofish_compressed_brotli_truncated", PASSPHRASE_CORRECT, true);
#endif
#endif
	res &= test_decode_format("Truncated legacy image", "testdata/invalid_legacy_truncated", NULL, true);
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Truncated encrypted legacy image", "testdata/invalid_legacy_encrypted_truncated", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Truncated compressed legacy image", "testdata/invalid_legacy_compressed_truncated", NULL, true);
#endif
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Truncated encrypted and compressed legacy image", "testdata/invalid_legacy_encrypted_compressed_truncated", PASSPHRASE_CORRECT, true);
#endif
#endif
	
	// Invalid CRC
	res &= test_decode_format("Invalid checksum", "testdata/invalid_crc", NULL, true);
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Invalid checksum, AES encryption", "testdata/invalid_encrypted_aes_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Camellia encryption", "testdata/invalid_encrypted_camellia_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Serpent encryption", "testdata/invalid_encrypted_serpent_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Twofish encryption", "testdata/invalid_encrypted_twofish_crc", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Invalid checksum, deflate compression", "testdata/invalid_compressed_deflate_crc", NULL, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Invalid checksum, ZSTD compression", "testdata/invalid_compressed_zstd_crc", NULL, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Invalid checksum, LZMA2 compression", "testdata/invalid_compressed_lzma2_crc", NULL, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Invalid checksum, BZIP2 compression", "testdata/invalid_compressed_bzip2_crc", NULL, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Invalid checksum, Brotli compression", "testdata/invalid_compressed_brotli_crc", NULL, true);
#endif
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Invalid checksum, AES encryption, deflate compression", "testdata/invalid_encrypted_aes_compressed_deflate_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Camellia encryption, deflate compression", "testdata/invalid_encrypted_camellia_compressed_deflate_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Serpent encryption, deflate compression", "testdata/invalid_encrypted_serpent_compressed_deflate_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Twofish encryption, deflate compression", "testdata/invalid_encrypted_twofish_compressed_deflate_crc", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Invalid checksum, AES encryption ZSTD compression", "testdata/invalid_encrypted_aes_compressed_zstd_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Camellia encryption ZSTD compression", "testdata/invalid_encrypted_camellia_compressed_zstd_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Serpent encryption ZSTD compression", "testdata/invalid_encrypted_serpent_compressed_zstd_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Twofish encryption ZSTD compression", "testdata/invalid_encrypted_twofish_compressed_zstd_crc", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Invalid checksum, AES encryption, LZMA2 compression", "testdata/invalid_encrypted_aes_compressed_lzma2_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Camellia encryption, LZMA2 compression", "testdata/invalid_encrypted_camellia_compressed_lzma2_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Serpent encryption, LZMA2 compression", "testdata/invalid_encrypted_serpent_compressed_lzma2_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Twofish encryption, LZMA2 compression", "testdata/invalid_encrypted_twofish_compressed_lzma2_crc", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Invalid checksum, AES encryption, BZIP2 compression", "testdata/invalid_encrypted_aes_compressed_bzip2_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Camellia encryption, BZIP2 compression", "testdata/invalid_encrypted_camellia_compressed_bzip2_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Serpent encryption, BZIP2 compression", "testdata/invalid_encrypted_serpent_compressed_bzip2_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Twofish encryption, BZIP2 compression", "testdata/invalid_encrypted_twofish_compressed_bzip2_crc", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Invalid checksum, AES encryption, Brotli compression", "testdata/invalid_encrypted_aes_compressed_brotli_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Camellia encryption, Brotli compression", "testdata/invalid_encrypted_camellia_compressed_brotli_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Serpent encryption, Brotli compression", "testdata/invalid_encrypted_serpent_compressed_brotli_crc", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid checksum, Twofish encryption, Brotli compression", "testdata/invalid_encrypted_twofish_compressed_brotli_crc", PASSPHRASE_CORRECT, true);
#endif
#endif
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Legacy image, invalid checksum", "testdata/invalid_legacy_crc", NULL, true); // Legacy images use SHA-512
#endif
	
	// Invalid data length
	res &= test_decode_format("Invalid data length", "testdata/invalid_crc", NULL, true);
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Invalid data length, AES encryption", "testdata/invalid_encrypted_aes_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Camellia encryption", "testdata/invalid_encrypted_camellia_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Serpent encryption", "testdata/invalid_encrypted_serpent_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Twofish encryption", "testdata/invalid_encrypted_twofish_length", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Invalid data length, deflate compression", "testdata/invalid_compressed_deflate_length", NULL, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Invalid data length, ZSTD compression", "testdata/invalid_compressed_zstd_length", NULL, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Invalid data length, LZMA2 compression", "testdata/invalid_compressed_lzma2_length", NULL, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Invalid data length, BZIP2 compression", "testdata/invalid_compressed_bzip2_length", NULL, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Invalid data length, Brotli compression", "testdata/invalid_compressed_brotli_length", NULL, true);
#endif
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Invalid data length, AES encryption, deflate compression", "testdata/invalid_encrypted_aes_compressed_deflate_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Camellia encryption, deflate compression", "testdata/invalid_encrypted_camellia_compressed_deflate_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Serpent encryption, deflate compression", "testdata/invalid_encrypted_serpent_compressed_deflate_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Twofish encryption, deflate compression", "testdata/invalid_encrypted_twofish_compressed_deflate_length", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Invalid data length, AES encryption ZSTD compression", "testdata/invalid_encrypted_aes_compressed_zstd_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Camellia encryption ZSTD compression", "testdata/invalid_encrypted_camellia_compressed_zstd_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Serpent encryption ZSTD compression", "testdata/invalid_encrypted_serpent_compressed_zstd_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Twofish encryption ZSTD compression", "testdata/invalid_encrypted_twofish_compressed_zstd_length", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Invalid data length, AES encryption, LZMA2 compression", "testdata/invalid_encrypted_aes_compressed_lzma2_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Camellia encryption, LZMA2 compression", "testdata/invalid_encrypted_camellia_compressed_lzma2_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Serpent encryption, LZMA2 compression", "testdata/invalid_encrypted_serpent_compressed_lzma2_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Twofish encryption, LZMA2 compression", "testdata/invalid_encrypted_twofish_compressed_lzma2_length", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Invalid data length, AES encryption, BZIP2 compression", "testdata/invalid_encrypted_aes_compressed_bzip2_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Camellia encryption, BZIP2 compression", "testdata/invalid_encrypted_camellia_compressed_bzip2_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Serpent encryption, BZIP2 compression", "testdata/invalid_encrypted_serpent_compressed_bzip2_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Twofish encryption, BZIP2 compression", "testdata/invalid_encrypted_twofish_compressed_bzip2_length", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Invalid data length, AES encryption, Brotli compression", "testdata/invalid_encrypted_aes_compressed_brotli_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Camellia encryption, Brotli compression", "testdata/invalid_encrypted_camellia_compressed_brotli_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Serpent encryption, Brotli compression", "testdata/invalid_encrypted_serpent_compressed_brotli_length", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Invalid data length, Twofish encryption, Brotli compression", "testdata/invalid_encrypted_twofish_compressed_brotli_length", PASSPHRASE_CORRECT, true);
#endif
#endif
	res &= test_decode_format("Legacy image, invalid data length", "testdata/invalid_legacy_length", NULL, true);
	
	// Corrupt data
	res &= test_decode_format("Corrupt data", "testdata/invalid_data", NULL, true);
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Corrupt data, AES encryption", "testdata/invalid_encrypted_aes_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Camellia encryption", "testdata/invalid_encrypted_camellia_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Serpent encryption", "testdata/invalid_encrypted_serpent_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Twofish encryption", "testdata/invalid_encrypted_twofish_data", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Corrupt data, deflate compression", "testdata/invalid_compressed_deflate_data", NULL, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Corrupt data, ZSTD compression", "testdata/invalid_compressed_zstd_data", NULL, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Corrupt data, LZMA2 compression", "testdata/invalid_compressed_lzma2_data", NULL, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Corrupt data, BZIP2 compression", "testdata/invalid_compressed_bzip2_data", NULL, true);
#endif
#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Corrupt data, Brotli compression", "testdata/invalid_compressed_brotli_data", NULL, true);
#endif
#ifdef IMPACK_WITH_CRYPTO
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Corrupt data, AES encryption, deflate compression", "testdata/invalid_encrypted_aes_compressed_deflate_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Camellia encryption, deflate compression", "testdata/invalid_encrypted_camellia_compressed_deflate_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Serpent encryption, deflate compression", "testdata/invalid_encrypted_serpent_compressed_deflate_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Twofish encryption, deflate compression", "testdata/invalid_encrypted_twofish_compressed_deflate_data", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Corrupt data, AES encryption ZSTD compression", "testdata/invalid_encrypted_aes_compressed_zstd_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Camellia encryption ZSTD compression", "testdata/invalid_encrypted_camellia_compressed_zstd_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Serpent encryption ZSTD compression", "testdata/invalid_encrypted_serpent_compressed_zstd_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Twofish encryption ZSTD compression", "testdata/invalid_encrypted_twofish_compressed_zstd_data", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Corrupt data, AES encryption, LZMA2 compression", "testdata/invalid_encrypted_aes_compressed_lzma2_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Camellia encryption, LZMA2 compression", "testdata/invalid_encrypted_camellia_compressed_lzma2_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Serpent encryption, LZMA2 compression", "testdata/invalid_encrypted_serpent_compressed_lzma2_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Twofish encryption, LZMA2 compression", "testdata/invalid_encrypted_twofish_compressed_lzma2_data", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_BZIP2
	res &= test_decode_format("Corrupt data, AES encryption, BZIP2 compression", "testdata/invalid_encrypted_aes_compressed_bzip2_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Camellia encryption, BZIP2 compression", "testdata/invalid_encrypted_camellia_compressed_bzip2_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Serpent encryption, BZIP2 compression", "testdata/invalid_encrypted_serpent_compressed_bzip2_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Twofish encryption, BZIP2 compression", "testdata/invalid_encrypted_twofish_compressed_bzip2_data", PASSPHRASE_CORRECT, true);
#endif
	// Brotli seems to sometimes lock up when presented with invalid data
	// TODO: Figure out if this is a bug with brotli or impack's implementation
/*#ifdef IMPACK_WITH_BROTLI
	res &= test_decode_format("Corrupt data, AES encryption, Brotli compression", "testdata/invalid_encrypted_aes_compressed_brotli_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Camellia encryption, Brotli compression", "testdata/invalid_encrypted_camellia_compressed_brotli_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Serpent encryption, Brotli compression", "testdata/invalid_encrypted_serpent_compressed_brotli_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Corrupt data, Twofish encryption, Brotli compression", "testdata/invalid_encrypted_twofish_compressed_brotli_data", PASSPHRASE_CORRECT, true);
#endif*/
#endif
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Legacy image, corrupt data", "testdata/invalid_legacy_data", NULL, true);
	res &= test_decode_format("Legacy encrypted image, corrupt data", "testdata/invalid_legacy_encrypted_data", PASSPHRASE_CORRECT, true);
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Legacy compressed image, corrupt data", "testdata/invalid_legacy_compressed_data", NULL, true);
	res &= test_decode_format("Legacy encrypted and compressed image, corrupt data", "testdata/invalid_legacy_encrypted_compressed_data", PASSPHRASE_CORRECT, true);
#endif
#endif
	
	// Misc. invalid
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Encrypted image, AES encryption, incorrect IV", "testdata/invalid_encrypted_aes_iv", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Encrypted image, Camellia encryption, incorrect IV", "testdata/invalid_encrypted_camellia_iv", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Encrypted image, Serpent encryption, incorrect IV", "testdata/invalid_encrypted_serpent_iv", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Encrypted image, Twofish encryption, incorrect IV", "testdata/invalid_encrypted_twofish_iv", PASSPHRASE_CORRECT, true);
#endif
	res &= test_decode_format("Invalid magic number", "testdata/invalid_magic", NULL, true);
	res &= test_decode_format("Legacy image, invalid magic number", "testdata/invalid_legacy_magic", NULL, true);
	res &= test_decode_format("Invalid channel selection 1", "testdata/invalid_channel_1", NULL, true);
	res &= test_decode_format("Invalid channel selection 2", "testdata/invalid_channel_2", NULL, true);
	
	return res;
	
}

int main(int argc, char **argv) {
	
	int res = 0;
	printf("ImPack2 testsuite\n");
	printf("=================\n\n");
	impack_build_info();
	printf("\n");
	printf("Preparing... ");
	FILE *f = fopen("testdata/input.bin", "rb");
	if (f == NULL) {
		printf("\nError: Unable to open testdata/input.bin");
		return 1;
	}
	if (fread(ref_file, 1, REF_LENGTH, f) != REF_LENGTH) {
		printf("\nError: Unable to read testdata/input.bin");
		fclose(f);
		return 1;
	}
	fclose(f);
	printf("OK\n\n");
	printf("Testing encode + decode cycles...\n");
	if (!test_cycle()) {
		res = 1;
	}
	printf("\n");
	printf("Testing decode against reference files...\n");
	if (!test_decode()) {
		res = 1;
	}
	printf("\n");
	if (res == 0) {
		printf("All tests PASSED\n");
	} else {
		printf("Some tests FAILED\n");
		printf("Please report a bug\n");
	}
	
	return res;
	
}
