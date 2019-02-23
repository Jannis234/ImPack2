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
		haserror = true;
	}
	int32_t bytes_read = fread(buf, 1, REF_LENGTH, f);
	fclose(f);
	if (bytes_read != REF_LENGTH) {
		if (printerror) {
			printf("Error\n");
			printerror = false;
		}
		printf("  Unable to read testout_decode.tmp, got %d bytes\n", bytes_read);
		haserror = true;
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

bool test_decode_format(char *msg, char *filename, char *passphrase, bool shouldfail) {
	
	bool res = true;
	strcpy(namebuf, filename);
	size_t namelen = strlen(filename);
	char passbuf[PASSPHRASE_LEN + 1];
	char *passarg = NULL;
#ifdef IMPACK_WITH_PNG
	printf("%s, PNG: ", msg);
	strcpy(namebuf + namelen, ".png");
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	res &= decode_run(namebuf, passbuf, shouldfail);
#endif
#ifdef IMPACK_WITH_WEBP
	printf("%s, WebP: ", msg);
	strcpy(namebuf + namelen, ".webp");
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	res &= decode_run(namebuf, passarg, shouldfail);
#endif
#ifdef IMPACK_WITH_TIFF
	printf("%s, TIFF: ", msg);
	strcpy(namebuf + namelen, ".tiff");
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	res &= decode_run(namebuf, passarg, shouldfail);
#endif
#ifdef IMPACK_WITH_BMP
	printf("%s, BMP: ", msg);
	strcpy(namebuf + namelen, ".bmp");
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	res &= decode_run(namebuf, passarg, shouldfail);
#endif
	return res;
	
}

bool encode_run(impack_img_format_t format, bool encrypt, char *passphrase, impack_compression_type_t compress, uint64_t width, uint64_t height, uint8_t channels) {
	
	impack_error_t res = impack_encode("testdata/input.bin", "testout_encode.tmp", encrypt, passphrase, compress, 0, channels, width, height, format, "testdata/input.bin");
	if (res != ERROR_OK) {
		printf("Error\n");
		printf("  Unexpected error after encode: ");
		print_error(res);
		return false;
	}
	return true;
	
}

bool test_cycle_format(char *msg, bool encrypt, char *passphrase, impack_compression_type_t compress, uint64_t width, uint64_t height, uint8_t channels) {
	
	bool res = true;
	char passbuf[PASSPHRASE_LEN + 1];
	char *passarg = NULL;
#ifdef IMPACK_WITH_PNG
	printf("%s, PNG: ", msg);
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	if (!encode_run(FORMAT_PNG, encrypt, passarg, compress, width, height, channels)) {
		res = false;
	} else {
		if (passphrase != NULL) {
			strcpy(passbuf, passphrase);
		}
		res &= decode_run("testout_encode.tmp", passarg, false);
	}
#endif
#ifdef IMPACK_WITH_WEBP
	printf("%s, WebP: ", msg);
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	if (!encode_run(FORMAT_WEBP, encrypt, passarg, compress, width, height, channels)) {
		res = false;
	} else {
		if (passphrase != NULL) {
			strcpy(passbuf, passphrase);
		}
		res &= decode_run("testout_encode.tmp", passarg, false);
	}
#endif
#ifdef IMPACK_WITH_TIFF
	printf("%s, TIFF: ", msg);
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	if (!encode_run(FORMAT_TIFF, encrypt, passarg, compress, width, height, channels)) {
		res = false;
	} else {
		if (passphrase != NULL) {
			strcpy(passbuf, passphrase);
		}
		res &= decode_run("testout_encode.tmp", passarg, false);
	}
#endif
#ifdef IMPACK_WITH_BMP
	printf("%s, BMP: ", msg);
	passarg = NULL;
	if (passphrase != NULL) {
		strcpy(passbuf, passphrase);
		passarg = passbuf;
	}
	if (!encode_run(FORMAT_BMP, encrypt, passarg, compress, width, height, channels)) {
		res = false;
	} else {
		if (passphrase != NULL) {
			strcpy(passbuf, passphrase);
		}
		res &= decode_run("testout_encode.tmp", passarg, false);
	}
#endif
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
	res &= test_cycle_format("Custom width", false, NULL, COMPRESSION_NONE, 1, 0, allchannels);
	res &= test_cycle_format("Custom height", false, NULL, COMPRESSION_NONE, 0, 1, allchannels);
	res &= test_cycle_format("Custom width + height", false, NULL, COMPRESSION_NONE, 50, 50, allchannels);
#ifdef IMPACK_WITH_ZLIB
	res &= test_cycle_format("Compressed data, deflate compression", false, NULL, COMPRESSION_ZLIB, 0, 0, allchannels);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_cycle_format("Compressed data, ZSTD compression", false, NULL, COMPRESSION_ZSTD, 0, 0, allchannels);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_cycle_format("Compressed data, LZMA2 compression", false, NULL, COMPRESSION_LZMA, 0, 0, allchannels);
#endif
#ifdef IMPACK_WITH_CRYPTO
	res &= test_cycle_format("Encrypted data", true, PASSPHRASE_CORRECT, COMPRESSION_NONE, 0, 0, allchannels);
#ifdef IMPACK_WITH_ZLIB
	res &= test_cycle_format("Encrypted and compressed data, deflate compression", true, PASSPHRASE_CORRECT, COMPRESSION_ZLIB, 0, 0, allchannels);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_cycle_format("Encrypted and compressed data, ZSTD compression", true, PASSPHRASE_CORRECT, COMPRESSION_ZSTD, 0, 0, allchannels);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_cycle_format("Encrypted and compressed data, LZMA2 compression", true, PASSPHRASE_CORRECT, COMPRESSION_LZMA, 0, 0, allchannels);
#endif
#endif
	return res;
	
}

bool test_decode() {
	
	bool res = true;
	res &= test_decode_format("Default settings", "testdata/valid", NULL, false);
	res &= test_decode_format("Red color channel", "testdata/valid_channel_red", NULL, false);
	res &= test_decode_format("Green color channel", "testdata/valid_channel_green", NULL, false);
	res &= test_decode_format("Blue color channel", "testdata/valid_channel_blue", NULL, false);
	res &= test_decode_format("Red + green color channels", "testdata/valid_channel_red_green", NULL, false);
	res &= test_decode_format("Red + blue color channels", "testdata/valid_channel_red_blue", NULL, false);
	res &= test_decode_format("Green + blue color channels", "testdata/valid_channel_green_blue", NULL, false);
	res &= test_decode_format("Grayscale mode", "testdata/valid_grayscale", NULL, false);
	res &= test_decode_format("Legacy image", "testdata/valid_legacy", NULL, false);
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Compressed data, deflate compression", "testdata/valid_compressed_deflate", NULL, false);
	res &= test_decode_format("Corrupted compressed data, deflate compression", "testdata/invalid_compressed_deflate", NULL, true);
	res &= test_decode_format("Legacy image, compressed data", "testdata/valid_legacy_compressed", NULL, false);
	res &= test_decode_format("Legacy image, corrupted compressed data", "testdata/invalid_legacy_compressed", NULL, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Compressed data, ZSTD compression", "testdata/valid_compressed_zstd", NULL, false);
	res &= test_decode_format("Corrupted compressed data, ZSTD compression", "testdata/invalid_compressed_zstd", NULL, true);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Compressed data, LZMA2 compression", "testdata/valid_compressed_lzma2", NULL, false);
	res &= test_decode_format("Corrupted compressed data, LZMA2 compression", "testdata/invalid_compressed_lzma2", NULL, true);
#endif
#ifdef IMPACK_WITH_CRYPTO
	res &= test_decode_format("Encrypted data", "testdata/valid_encrypted", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Encrypted data, incorrect passphrase", "testdata/valid_encrypted", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Corrupted encrypted data", "tesdata/invalid_encrypted_data", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Encrypted data, incorrect IV", "testdata/invalid_encrypted_iv", PASSPHRASE_CORRECT, true);
	res &= test_decode_format("Legacy image, encrypted data", "testdata/valid_legacy_encrypted", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Legacy image, encrypted data, incorrect passphrase", "testdata/valid_legacy_encrypted", PASSPHRASE_INCORRECT, true);
	res &= test_decode_format("Legacy image, corrupted encrypted data", "testdata/invalid_legacy_encrypted", PASSPHRASE_CORRECT, true);
#ifdef IMPACK_WITH_ZLIB
	res &= test_decode_format("Encrypted and compressed data, deflate compression", "testdata/valid_encrypted_compressed_deflate", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Legacy image, encrypted and compressed data", "testdata/valid_legacy_encrypted_compressed", PASSPHRASE_CORRECT, false);
	res &= test_decode_format("Legacy image, corrupted compressed data", "testdata/invalid_legacy_compressed", PASSPHRASE_CORRECT, true);
#endif
#ifdef IMPACK_WITH_ZSTD
	res &= test_decode_format("Encrypted and compressed data, ZSTD compression", "testdata/valid_encrypted_compressed_zstd", PASSPHRASE_CORRECT, false);
#endif
#ifdef IMPACK_WITH_LZMA
	res &= test_decode_format("Encrypted and compressed data, LZMA2 compression", "testdata/valid_encrypted_compressed_lzma2", PASSPHRASE_CORRECT, false);
#endif
	res &= test_decode_format("Legacy image, corrupted data", "testdata/legacy_invalid", NULL, true); // Legacy images use SHA-512
#endif
	res &= test_decode_format("Invalid magic number", "testdata/invalid_magic", NULL, true);
	res &= test_decode_format("Incorrect data size", "testdata/invalid_size", NULL, true);
	res &= test_decode_format("Corrupted data", "testdata/invalid_data", NULL, true);
	res &= test_decode_format("Truncated image", "testdata/invalid_truncated", NULL, true);
	return res;
	
}

int main(int argc, char **argv) {
	
	int res = 0;
	printf("ImPack2 testsuite\n");
	printf("=================\n");
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