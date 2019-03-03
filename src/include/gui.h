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

#ifndef __IMPACK_GUI_H__
#define __IMPACK_GUI_H__

#include <stdbool.h>
#include <stdint.h>
#include <gtk/gtk.h>
#include "impack.h"

typedef struct {
	char *input_path;
	char *output_path;
	uint8_t channels;
	impack_encryption_type_t encrypt;
	char *passphrase;
	impack_compression_type_t compress;
	int32_t compress_level;
	uint64_t img_width;
	uint64_t img_height;
	char *filename_include;
	impack_error_t res;
} encode_thread_data_t;

void window_main_add_callbacks(GtkBuilder *b);
void window_main_setup(GtkBuilder *b);

bool encode_thread_run(encode_thread_data_t *params);

#endif
