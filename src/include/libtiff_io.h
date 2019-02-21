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

#ifndef __IMPACK_LIBTIFF_IO_H__
#define __IMPACK_LIBTIFF_IO_H__

#include "config.h"

#ifdef IMPACK_WITH_TIFF

#include <stdbool.h>
#include <stdio.h>
#include <tiffio.h>

/* Libtiff requires seeking on it's input/output file
 * Since ImPack2 might be working with stdin/stdout, this code emulates
 * file I/O on a buffer in memory */

bool impack_tiff_init_read(FILE *input_file, bool le);
bool impack_tiff_init_write();
void impack_tiff_finish_read();
bool impack_tiff_finish_write(FILE *output_file);
tsize_t impack_tiff_read(thandle_t data, tdata_t buf, tsize_t len);
tsize_t impack_tiff_write(thandle_t data, tdata_t buf, tsize_t len);
toff_t impack_tiff_seek(thandle_t data, toff_t offset, int whence);
int impack_tiff_close(thandle_t data);
toff_t impack_tiff_size(thandle_t data);
int impack_tiff_map(thandle_t data, tdata_t *buf, toff_t *len);
void impack_tiff_unmap(thandle_t data, tdata_t buf, toff_t len);

#endif
#endif
