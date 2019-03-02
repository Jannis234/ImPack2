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
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "config.h"
#include "impack.h"

GtkBuilder *builder;
bool encode_color_checkbox_state[4];
bool encode_color_checkbox_enabled;

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
	GtkRevealer *reveal = GTK_REVEALER(gtk_builder_get_object(builder, "EncodeAdvancedReveal"));
	gtk_revealer_set_reveal_child(reveal, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	GtkRevealer *encrypt_reveal = GTK_REVEALER(gtk_builder_get_object(builder, "EncodeEncryptAdvancedReveal"));
	gtk_revealer_set_reveal_child(encrypt_reveal, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	build_encode_compression_type_box(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	encode_compress_checkbox_toggle();
	
}

void encode_color_checkbox_toggle() {
	
	encode_color_checkbox_enabled = false;
	GtkCheckButton *color_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorBox"));
	GtkCheckButton *grayscale_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorGrayscaleBox"));
	GtkCheckButton *red_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorRedBox"));
	GtkCheckButton *green_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorGreenBox"));
	GtkCheckButton *blue_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorBlueBox"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_box))) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(grayscale_box), encode_color_checkbox_state[3]);
		gtk_widget_set_sensitive(GTK_WIDGET(grayscale_box), true);
		if (encode_color_checkbox_state[3]) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(red_box), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(green_box), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blue_box), false);
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(red_box), encode_color_checkbox_state[0]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(green_box), encode_color_checkbox_state[1]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blue_box), encode_color_checkbox_state[2]);
			gtk_widget_set_sensitive(GTK_WIDGET(red_box), true);
			gtk_widget_set_sensitive(GTK_WIDGET(green_box), true);
			gtk_widget_set_sensitive(GTK_WIDGET(blue_box), true);
		}
	} else {
		encode_color_checkbox_state[3] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(grayscale_box));
		if (!encode_color_checkbox_state[3]) {
			encode_color_checkbox_state[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(red_box));
			encode_color_checkbox_state[1] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(green_box));
			encode_color_checkbox_state[2] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(blue_box));
		}
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(red_box), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(green_box), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blue_box), true);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(grayscale_box), false);
		gtk_widget_set_sensitive(GTK_WIDGET(red_box), false);
		gtk_widget_set_sensitive(GTK_WIDGET(green_box), false);
		gtk_widget_set_sensitive(GTK_WIDGET(blue_box), false);
		gtk_widget_set_sensitive(GTK_WIDGET(grayscale_box), false);
	}
	encode_color_checkbox_enabled = true;
	
}

void encode_grayscale_checkbox_toggle() {
	
	if (encode_color_checkbox_enabled) { // Prevent this from running when encode_color_checkbox_toggle() changes the grayscale checkbox
		GtkCheckButton *grayscale_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorGrayscaleBox"));
		GtkCheckButton *red_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorRedBox"));
		GtkCheckButton *green_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorGreenBox"));
		GtkCheckButton *blue_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorBlueBox"));
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(grayscale_box))) {
			encode_color_checkbox_state[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(red_box));
			encode_color_checkbox_state[1] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(green_box));
			encode_color_checkbox_state[2] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(blue_box));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(red_box), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(green_box), false);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blue_box), false);
			gtk_widget_set_sensitive(GTK_WIDGET(red_box), false);
			gtk_widget_set_sensitive(GTK_WIDGET(green_box), false);
			gtk_widget_set_sensitive(GTK_WIDGET(blue_box), false);
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(red_box), encode_color_checkbox_state[0]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(green_box), encode_color_checkbox_state[1]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blue_box), encode_color_checkbox_state[2]);
			gtk_widget_set_sensitive(GTK_WIDGET(red_box), true);
			gtk_widget_set_sensitive(GTK_WIDGET(green_box), true);
			gtk_widget_set_sensitive(GTK_WIDGET(blue_box), true);
		}
	}
	
}

void encode_width_checkbox_toggled() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeWidthBox"));
	GtkSpinButton *number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeWidthNumber"));
	gtk_widget_set_sensitive(GTK_WIDGET(number), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	
}

void encode_height_checkbox_toggled() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeHeightBox"));
	GtkSpinButton *number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeHeightNumber"));
	gtk_widget_set_sensitive(GTK_WIDGET(number), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	
}

GtkFileChooserDialog* create_open_file_dialog() {
	
	GtkFileChooserDialog *dialog = GTK_FILE_CHOOSER_DIALOG(gtk_file_chooser_dialog_new("Select input file", GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL));
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
	GtkFileFilter *img_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(img_filter, "Images");
#ifdef IMPACK_WITH_PNG
	gtk_file_filter_add_pattern(img_filter, "*.png");
#endif
#ifdef IMPACK_WITH_WEBP
	gtk_file_filter_add_pattern(img_filter, "*.webp");
#endif
#ifdef IMPACK_WITH_TIFF
	gtk_file_filter_add_pattern(img_filter, "*.tiff");
	gtk_file_filter_add_pattern(img_filter, "*.tif");
#endif
#ifdef IMPACK_WITH_BMP
	gtk_file_filter_add_pattern(img_filter, "*.bmp");
#endif
#ifdef IMPACK_WITH_JP2K
	gtk_file_filter_add_pattern(img_filter, "*.jp2");
	gtk_file_filter_add_pattern(img_filter, "*.j2k");
#endif
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), img_filter);
	GtkFileFilter *all_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(all_filter, "All files");
	gtk_file_filter_add_pattern(all_filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);
	return dialog;
	
}

void encode_open_button_click() {
	
	GtkFileChooserDialog *dialog = create_open_file_dialog();
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeInputText"));
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(entry, filename);
		g_free(filename);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
}

void decode_open_button_click() {
	
	GtkFileChooserDialog *dialog = create_open_file_dialog();
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "DecodeInputText"));
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(entry, filename);
		g_free(filename);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
}

void encode_output_button_click() {
	
	GtkFileChooserDialog *dialog = GTK_FILE_CHOOSER_DIALOG(gtk_file_chooser_dialog_new("Select output file", GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Select", GTK_RESPONSE_ACCEPT, NULL));
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
	GtkLabel *label = GTK_LABEL(gtk_label_new("Image format:"));
	GtkComboBoxText *cbox = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
#ifdef IMPACK_WITH_BMP
	gtk_combo_box_text_append(cbox, "bmp", "BMP");
#endif
#ifdef IMPACK_WITH_JP2K
	gtk_combo_box_text_append(cbox, "jp2", "JPEG 2000");
#endif
#ifdef IMPACK_WITH_PNG
	gtk_combo_box_text_append(cbox, "png", "PNG");
#endif
#ifdef IMPACK_WITH_TIFF
	gtk_combo_box_text_append(cbox, "tiff", "TIFF");
#endif
#ifdef IMPACK_WITH_WEBP
	gtk_combo_box_text_append(cbox, "webp", "WebP");
#endif
	char *default_format = NULL;
	switch (impack_default_img_format()) {
		case FORMAT_BMP:
			default_format = "bmp";
			break;
		case FORMAT_JP2K:
			default_format = "jp2";
			break;
		case FORMAT_PNG:
			default_format = "png";
			break;
		case FORMAT_TIFF:
			default_format = "tiff";
			break;
		case FORMAT_WEBP:
			default_format = "webp";
			break;
		default:
			abort();
	}
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(cbox), default_format);
	gtk_box_pack_end(box, GTK_WIDGET(cbox), false, false, 0);
	gtk_box_pack_end(box, GTK_WIDGET(label), false, false, 0);
	gtk_widget_set_hexpand(GTK_WIDGET(box), true);
	gtk_widget_show(GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(label));
	gtk_widget_show(GTK_WIDGET(cbox));
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), GTK_WIDGET(box));
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeOutputText"));
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		char *fileext = filename + strlen(filename) - 1;
		while (filename != fileext) {
			if (*fileext == '.' || *fileext == '/' || *fileext == '\\') {
				break;
			}
			fileext--;
		}
		bool addext = true;
		const gchar *format_selected = gtk_combo_box_get_active_id(GTK_COMBO_BOX(cbox));
		if (fileext != filename && *fileext == '.') {
			if (strcmp(format_selected, fileext + 1) == 0) {
				addext = false; // Extension already typed in by the user
			}
		}
		if (addext) {
			char *newname = malloc(strlen(filename) + strlen(format_selected) + 2);
			if (newname != NULL) {
				sprintf(newname, "%s.%s", filename, format_selected);
				g_free(filename);
				filename = newname;
			}
		}
		gtk_entry_set_text(entry, filename);
		g_free(filename);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
}

void window_main_add_callbacks(GtkBuilder *b) {
	
	gtk_builder_add_callback_symbol(b, "encode_encrypt_checkbox_toggle", encode_encrypt_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_compress_checkbox_toggle", encode_compress_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_advanced_checkbox_toggle", encode_advanced_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_color_checkbox_toggle", encode_color_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_grayscale_checkbox_toggle", encode_grayscale_checkbox_toggle);
	gtk_builder_add_callback_symbol(b, "encode_width_checkbox_toggled", encode_width_checkbox_toggled);
	gtk_builder_add_callback_symbol(b, "encode_height_checkbox_toggled", encode_height_checkbox_toggled);
	gtk_builder_add_callback_symbol(b, "encode_open_button_click", encode_open_button_click);
	gtk_builder_add_callback_symbol(b, "decode_open_button_click", decode_open_button_click);
	gtk_builder_add_callback_symbol(b, "encode_output_button_click", encode_output_button_click);
	
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
	encode_color_checkbox_state[0] = true;
	encode_color_checkbox_state[1] = true;
	encode_color_checkbox_state[2] = true;
	encode_color_checkbox_state[3] = false;
	encode_color_checkbox_enabled = true;
	
}
