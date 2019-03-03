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
#include <gtk/gtk.h>
#include "impack.h"
#include "gui.h"

bool encode_thread_running;
bool decode_thread_running;

void* encode_thread_main(void *data) {
	
	encode_thread_data_t *params = (encode_thread_data_t*) data;
	params->res = impack_encode(params->input_path, params->output_path, params->encrypt, params->passphrase, params->compress, params->compress_level, params->channels, params->img_width, params->img_height, FORMAT_AUTO, params->filename_include);
	encode_thread_running = false;
	return NULL;
	
}

bool encode_thread_run(encode_thread_data_t *params) {
	
	encode_thread_running = true;
	GError *error;
	GThread *encode_thread = g_thread_try_new("", encode_thread_main, params, &error);
	if (encode_thread == NULL) {
		return false;
	}
	while (encode_thread_running) {
		gtk_main_iteration();
	}
	g_thread_unref(encode_thread);
	return true;
	
}

void* decode_thread_main(void *data) {
	
	decode_thread_data_t *params = (decode_thread_data_t*) data;
	if (params->stage == 0) {
		params->res = impack_decode_stage1(&params->state, params->input_path);
	} else if (params->stage == 1) {
		params->res = impack_decode_stage2(&params->state, params->passphrase);
	} else {
		params->res = impack_decode_stage3(&params->state, params->output_path);
	}
	decode_thread_running = false;
	return NULL;
	
}

bool decode_thread_run(decode_thread_data_t *params) {
	
	decode_thread_running = true;
	GError *error;
	GThread *decode_thread = g_thread_try_new("", decode_thread_main, params, &error);
	if (decode_thread == NULL) {
		return false;
	}
	while (decode_thread_running) {
		gtk_main_iteration();
	}
	g_thread_unref(decode_thread);
	return true;
	
}
