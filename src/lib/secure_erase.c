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
#include <stddef.h>
#include <stdint.h>

void impack_secure_erase(uint8_t *buf, size_t len) {
	
	volatile uint8_t *pointer = (volatile uint8_t*) buf;
	while (len > 0) {
		*pointer = 0;
		pointer++;
		len--;
	}
	
}
