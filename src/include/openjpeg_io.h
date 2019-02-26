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

#ifndef __IMPACK_OPENJPEG_IO_H__
#define __IMPACK_OPENJPEG_IO_H__

#include "config.h"

#ifdef IMPACK_WITH_JP2K

#include <stdbool.h>
#include <stdio.h>
#include <openjpeg.h>
#include "impack.h"

/* Same problem why libtiff_io.c exists:
   Openjpeg wants a seekable stream and ImPack2 might be working on stdin/stdout */

extern impack_error_t impack_opj_stream_write_errno;
opj_stream_t *impack_create_opj_stream(FILE *f, bool is_input);

#endif
#endif
