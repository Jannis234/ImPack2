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

#include <gtk/gtk.h>
#include "window_main.h"

void main_window_close() {
	
	gtk_main_quit();
	
}

int main(int argc, char **argv) {
	
	gtk_init(&argc, &argv);
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_callback_symbol(builder, "main_window_close", main_window_close);
	window_main_add_callbacks(builder);
	GError *error = NULL;
	if (!gtk_builder_add_from_resource(builder, "/impack2/window_main.ui", &error)) {
		g_warning("%s", error->message);
		g_free(error);
		return 1;
	}
	GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "MainWindow"));
	gtk_builder_connect_signals(builder, NULL);
	gtk_widget_show(window);
	window_main_setup(builder);
	gtk_main();
	g_object_unref(G_OBJECT(builder));
	return 0;
	
}
