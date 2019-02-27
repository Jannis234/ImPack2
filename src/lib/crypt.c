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

#ifdef IMPACK_WITH_CRYPTO

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <nettle/aes.h>
#include <nettle/hmac.h>
#include <nettle/pbkdf2.h>

#define PBKDF2_ITERATIONS 100000

void impack_derive_key(char *passphrase, uint8_t *keyout, size_t keysize, uint8_t *salt, size_t saltsize) {
	
	struct hmac_sha512_ctx ctx;
	hmac_sha512_set_key(&ctx, strlen(passphrase), (uint8_t*) passphrase);
	PBKDF2(&ctx, hmac_sha512_update, hmac_sha512_digest, SHA512_DIGEST_SIZE, PBKDF2_ITERATIONS, saltsize, salt, keysize, keyout);
	
}

void impack_derive_key_legacy(char *passphrase, uint8_t *keyout, size_t keysize, uint8_t *salt, size_t saltsize) {
	
	struct hmac_sha1_ctx ctx;
	hmac_sha1_set_key(&ctx, strlen(passphrase), (uint8_t*) passphrase);
	PBKDF2(&ctx, hmac_sha1_update, hmac_sha1_digest, SHA1_DIGEST_SIZE, 1000, saltsize, salt, keysize, keyout);
	
}

#endif
