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
#include <stdint.h>

bool host_big_endian() {
	
	uint32_t test = 1;
	uint8_t *test2 = (uint8_t*) &test;
	return (test2[0] == 0);
	
}

void swap(uint8_t *src, uint8_t *dst, int len) {
	
	for (int i = 0; i < len; i++) {
		dst[i] = src[len - 1 - i];
	}
	
}

uint64_t impack_endian64(uint64_t val) {
	
	if (host_big_endian()) {
		return val;
	} else {
		uint64_t res = 0;
		swap((uint8_t*) &val, (uint8_t*) &res, 8);
		return res;
	}
	
}

uint32_t impack_endian32(uint32_t val) {
	
	if (host_big_endian()) {
		return val;
	} else {
		uint32_t res = 0;
		swap((uint8_t*) &val, (uint8_t*) &res, 4);
		return res;
	}
	
}

