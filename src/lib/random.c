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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#ifdef IMPACK_WINDOWS
#include <windows.h>
#include <wincrypt.h>
#endif

bool impack_random(uint8_t *dst, size_t count) {

#ifdef IMPACK_WINDOWS
	HCRYPTPROV provider;
	if (CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0) {
		return false;
	}
	if (CryptGenRandom(provider, count, dst) == 0) {
		CryptReleaseContext(provider, 0);
		return false;
	}
	CryptReleaseContext(provider, 0);
	return true;
#else
	FILE *devurandom = fopen("/dev/urandom", "rb");
	if (devurandom == NULL) {
		return false;
	}
	if (fread(dst, 1, count, devurandom) != count) {
		return false;
	}
	fclose(devurandom);
	return true;
#endif

}

#endif

