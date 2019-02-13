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

#include <stdio.h>
#include "impack.h"

int main(int argc, char **argv) {

	// TODO: Dummy code for testing
	impack_error_t res = impack_encode(argv[1], argv[2], false, NULL, COMPRESSION_ZLIB);
	fprintf(stderr, "%d\n", res);
	impack_decode_state_t state;
	res = impack_decode_stage1(&state, argv[2]);
	fprintf(stderr, "%d\n", res);
	res = impack_decode_stage2(&state, NULL);
	fprintf(stderr, "%d\n", res);
	res = impack_decode_stage3(&state, argv[3]);
	fprintf(stderr, "%d\n", res);

	return 0;
	
}
