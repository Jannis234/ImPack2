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

#ifdef IMPACK_WITH_JXL

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

impack_error_t impack_read_img_jxl(FILE *input_file, uint8_t *magic, uint8_t **pixeldata, uint64_t *pixeldata_size) {
	
	impack_error_t ret = ERROR_MALLOC;
	return ERROR_OK;
	
cleanup:
	
	return ret;
	
}

#endif
