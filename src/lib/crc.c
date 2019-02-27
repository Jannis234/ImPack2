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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define CRC_POLY 0xC96C5795D7870F42ULL

uint64_t crc_table[256];

void impack_crc_init() {
	
	for (int i = 0; i < 256; i++) {
		uint64_t crc = i;
		for (int j = 0; j < 8; j++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ CRC_POLY;
			} else {
				crc >>= 1;
			}
		}
		crc_table[i] = crc;
	}
	
}

void impack_crc(uint64_t *crc, uint8_t *buf, size_t buflen) {
	
	*crc = ~(*crc);
	for (size_t i = 0; i < buflen; i++) {
		*crc = crc_table[buf[i] ^ (*crc & 255)] ^ (*crc >> 8);
	}
	*crc = ~(*crc);
	
}
