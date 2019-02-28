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
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "config.h"
#include "impack.h"

GtkBuilder *builder;

int enabled_compression_types() {
	
	int res = 0;
#ifdef IMPACK_WITH_ZLIB
	res++;
#endif
#ifdef IMPACK_WITH_ZSTD
	res++;
#endif
#ifdef IMPACK_WITH_LZMA
	res++;
#endif
#ifdef IMPACK_WITH_BROTLI
	res++;
#endif
#ifdef IMPACK_WITH_BZIP2
	res++;
#endif
	return res;
	
}

void build_encode_compression_type_box(bool advanced) {
	
	GtkComboBoxText *box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "EncodeCompressTypeBox"));
	const gchar *last_active = gtk_combo_box_get_active_id(GTK_COMBO_BOX(box));
	char *next_active = NULL;
	if (advanced) {
		if (strcmp(last_active, "strong") == 0) { // Keep LZMA2 if selected, otherwise select the default
			next_active = "lzma2";
		}
	} else {
		if (strcmp(last_active, "lzma2") == 0) {
			next_active = "strong";
		} else {
			next_active = "normal";
		}
	}
	if (next_active == NULL) {
		switch (impack_default_compression()) {
			case COMPRESSION_BROTLI:
				next_active = "brotli";
				break;
			case COMPRESSION_BZIP2:
				next_active = "bzip2";
				break;
			case COMPRESSION_LZMA:
				next_active = "lzma2";
				break;
			case COMPRESSION_ZLIB:
				next_active = "deflate";
				break;
			case COMPRESSION_ZSTD:
				next_active = "zstd";
				break;
			default:
				abort();
		}
	}
	gtk_combo_box_text_remove_all(box);
	if (advanced) {
#ifdef IMPACK_WITH_BROTLI
		gtk_combo_box_text_append(box, "brotli", "Brotli");
#endif
#ifdef IMPACK_WITH_BZIP2
		gtk_combo_box_text_append(box, "bzip2", "Bzip2");
#endif
#ifdef IMPACK_WITH_ZLIB
		gtk_combo_box_text_append(box, "deflate", "Deflate");
#endif
#ifdef IMPACK_WITH_LZMA
		gtk_combo_box_text_append(box, "lzma2", "LZMA2");
#endif
#ifdef IMPACK_WITH_ZSTD
		gtk_combo_box_text_append(box, "zstd", "ZSTD");
#endif
	} else {
		gtk_combo_box_text_append(box, "normal", "Normal (Fast)");
		gtk_combo_box_text_append(box, "strong", "Strong (Slow)");
	}
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(box), next_active);
	
}

void encode_encrypt_checkbox_toggle() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeEncryptCheckbox"));
	GtkRevealer *reveal = GTK_REVEALER(gtk_builder_get_object(builder, "EncodeEncryptReveal"));
	gtk_revealer_set_reveal_child(reveal, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	
}

void encode_compress_checkbox_toggle() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCompressCheckbox"));
	GtkRevealer *reveal = GTK_REVEALER(gtk_builder_get_object(builder, "EncodeCompressReveal"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box))) {
		GtkCheckButton *adv_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeAdvancedCheckbox"));
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box))) {
			gtk_revealer_set_reveal_child(reveal, enabled_compression_types() > 1);
		} else {
#ifdef IMPACK_WITH_LZMA
			gtk_revealer_set_reveal_child(reveal, enabled_compression_types() > 1);
#else
			gtk_revealer_set_reveal_child(reveal, false);
#endif
		}
	} else {
		gtk_revealer_set_reveal_child(reveal, false);
	}
	
}

void encode_advanced_checkbox_toggle() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeAdvancedCheckbox"));
	GtkRevealer *encrypt_reveal = GTK_REVEALER(gtk_builder_get_object(builder, "EncodeEncryptAdvancedReveal"));
	gtk_revealer_set_reveal_child(encrypt_reveal, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	build_encode_compression_type_box(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	encode_compress_checkbox_toggle();
	
}

void window_main_add_callbacks(GtkBuilder *b) {
	
	gtk_builder_add_callback_symbol(b, "encode_encrypt_checkbox_toggle", encode_encrypt_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_compress_checkbox_toggle", encode_compress_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_advanced_checkbox_toggle", encode_advanced_checkbox_toggle);
	
}

void window_main_setup(GtkBuilder *b) {
	
	builder = b;
#ifdef IMPACK_WITH_CRYPTO
	GtkComboBoxText *encrypt_box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(b, "EncodeEncryptTypeBox"));
	gtk_combo_box_text_append(encrypt_box, "aes", "AES");
	gtk_combo_box_text_append(encrypt_box, "camellia", "Camellia");
	gtk_combo_box_text_append(encrypt_box, "serpent", "Serpent");
	gtk_combo_box_text_append(encrypt_box, "twofish", "Twofish");
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(encrypt_box), "aes");
#else
	GtkCheckButton *encrypt_box = GTK_CHECK_BUTTON(gtk_builder_get_object(b, "EncodeEncryptCheckbox"));
	gtk_widget_set_visible(GTK_WIDGET(encrypt_box), false);
#endif
#ifdef IMPACK_WITH_COMPRESSION
	GtkComboBoxText *compress_box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(b, "EncodeCompressTypeBox"));
	gtk_combo_box_text_append(compress_box, "normal", "Normal (Fast)");
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(compress_box), "normal");
#ifdef IMPACK_WITH_LZMA
	gtk_combo_box_text_append(compress_box, "strong", "Strong (Slow)");
#endif
#else
	GtkCheckButton *compress_box = GTK_CHECK_BUTTON(gtk_builder_get_object(b, "EncodeCompressCheckbox"));
	gtk_widget_set_visible(GTK_WIDGET(compress_box), false);
#endif
	
}
