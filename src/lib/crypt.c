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
#include <stdlib.h>
#include <string.h>
#include <nettle/aes.h>
#include <nettle/camellia.h>
#include <nettle/cbc.h>
#include <nettle/hmac.h>
#include <nettle/pbkdf2.h>
#include <nettle/serpent.h>
#include <nettle/twofish.h>
#include "impack.h"
#include "impack_internal.h"

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

void impack_set_encrypt_key(impack_crypt_ctx_t *ctx, uint8_t *key, impack_encryption_type_t type) {
	
	switch (type) {
		case ENCRYPTION_AES:
			aes256_set_encrypt_key(&ctx->aes, key);
			break;
		case ENCRYPTION_CAMELLIA:
			camellia256_set_encrypt_key(&ctx->camellia, key);
			break;
		case ENCRYPTION_SERPENT:
			serpent_set_key(&ctx->serpent, IMPACK_CRYPT_KEY_SIZE, key);
			break;
		case ENCRYPTION_TWOFISH:
			twofish_set_key(&ctx->twofish, IMPACK_CRYPT_KEY_SIZE, key);
			break;
		default:
			abort();
	}
	
}

void impack_set_decrypt_key(impack_crypt_ctx_t *ctx, uint8_t *key, impack_encryption_type_t type) {
	
	switch (type) {
		case ENCRYPTION_AES:
			aes256_set_decrypt_key(&ctx->aes, key);
			break;
		case ENCRYPTION_CAMELLIA:
			camellia256_set_decrypt_key(&ctx->camellia, key);
			break;
		case ENCRYPTION_SERPENT:
			serpent_set_key(&ctx->serpent, IMPACK_CRYPT_KEY_SIZE, key);
			break;
		case ENCRYPTION_TWOFISH:
			twofish_set_key(&ctx->twofish, IMPACK_CRYPT_KEY_SIZE, key);
			break;
		default:
			abort();
	}
	
}

void impack_encrypt(impack_crypt_ctx_t *ctx, uint8_t *data, uint64_t length, impack_encryption_type_t type) {
	
	void *nettle_ctx;
	nettle_cipher_func *f;
	switch (type) {
		case ENCRYPTION_AES:
			nettle_ctx = &ctx->aes;
			f = (nettle_cipher_func*) aes256_encrypt;
			break;
		case ENCRYPTION_CAMELLIA:
			nettle_ctx = &ctx->camellia;
			f = (nettle_cipher_func*) camellia256_crypt;
			break;
		case ENCRYPTION_SERPENT:
			nettle_ctx = &ctx->serpent;
			f = (nettle_cipher_func*) serpent_encrypt;
			break;
		case ENCRYPTION_TWOFISH:
			nettle_ctx = &ctx->twofish;
			f = (nettle_cipher_func*) twofish_encrypt;
			break;
		default:
			abort();
	}
	cbc_encrypt(nettle_ctx, f, IMPACK_CRYPT_BLOCK_SIZE, ctx->iv, length, data, data);
	
}

void impack_decrypt(impack_crypt_ctx_t *ctx, uint8_t *data, uint64_t length, impack_encryption_type_t type) {
	
	void *nettle_ctx;
	nettle_cipher_func *f;
	switch (type) {
		case ENCRYPTION_AES:
			nettle_ctx = &ctx->aes;
			f = (nettle_cipher_func*) aes256_decrypt;
			break;
		case ENCRYPTION_CAMELLIA:
			nettle_ctx = &ctx->camellia;
			f = (nettle_cipher_func*) camellia256_crypt;
			break;
		case ENCRYPTION_SERPENT:
			nettle_ctx = &ctx->serpent;
			f = (nettle_cipher_func*) serpent_decrypt;
			break;
		case ENCRYPTION_TWOFISH:
			nettle_ctx = &ctx->twofish;
			f = (nettle_cipher_func*) twofish_decrypt;
			break;
		default:
			abort();
	}
	cbc_decrypt(nettle_ctx, f, IMPACK_CRYPT_BLOCK_SIZE, ctx->iv, length, data, data);
	
}

#endif
