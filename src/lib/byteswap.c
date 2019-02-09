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

#include <stdint.h>

uint64_t impack_endian(uint64_t val) {
	
	uint32_t test = 1;
	uint8_t *test2 = (uint8_t*) &test;

	if (test2[0] == 0) { // Host is big-endian
		return val;
	} else { // Host is little endian
		uint64_t res = 0;
		uint8_t *val_bytes = (uint8_t*) &val;
		uint8_t *res_bytes = (uint8_t*) &res;
		for (int i = 0; i < 8; i++) {
			res_bytes[i] = val_bytes[7 - i];
		}
		return res;
	}
	
}

