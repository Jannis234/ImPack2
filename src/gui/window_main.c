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
#include "gui.h"

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

GtkFileChooserDialog* create_open_file_dialog(bool filter) {
	
	GtkFileChooserDialog *dialog = GTK_FILE_CHOOSER_DIALOG(gtk_file_chooser_dialog_new("Select input file", GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL));
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
	if (filter) {
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
	}
	GtkFileFilter *all_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(all_filter, "All files");
	gtk_file_filter_add_pattern(all_filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);
	return dialog;
	
}

void encode_open_button_click() {
	
	GtkFileChooserDialog *dialog = create_open_file_dialog(false);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeInputText"));
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(entry, filename);
		g_free(filename);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
}

void decode_open_button_click() {
	
	GtkFileChooserDialog *dialog = create_open_file_dialog(true);
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

void show_error(char *msg) {
	
	GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, msg));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
}

void show_impack_error(impack_error_t error) {
	
	char *msg = NULL;
	switch (error) {
		case ERROR_INPUT_NOT_FOUND:
			msg = "Can not read input file: No such file or directory";
			break;
		case ERROR_INPUT_PERMISSION:
			msg = "Can not read input file: Permission denied";
			break;
		case ERROR_INPUT_DIRECTORY:
			msg = "Can not read input file: Path is a directory";
			break;
		case ERROR_INPUT_IO:
			msg = "Can not read input file: I/O error";
			break;
		case ERROR_OUTPUT_NOT_FOUND:
			msg = "Can not write output file: No such directory";
			break;
		case ERROR_OUTPUT_PERMISSION:
			msg = "Can not write output file: Permission denied";
			break;
		case ERROR_OUTPUT_DIRECTORY:
			msg = "Can not write output file: Path is a directory";
			break;
		case ERROR_OUTPUT_IO:
			msg = "Can not write output file: I/O error";
			break;
		case ERROR_MALLOC:
			msg = "Out of memory";
			break;
		case ERROR_RANDOM:
			msg = "Can not gather random data for encryption";
			break;
		case ERROR_IMG_SIZE:
			msg = "Invalid image size for the selected output format";
			break;
		case ERROR_IMG_TOO_SMALL:
			msg = "Selected image size is too small";
			break;
		case ERROR_IMG_FORMAT_UNSUPPORTED:
			msg = "The image file format is unsupported by this build of ImPack2";
			break;
		case ERROR_IMG_FORMAT_UNKNOWN:
			msg = "Unknown image file format";
			break;
		case ERROR_INPUT_IMG_INVALID:
			msg = "The image contains invalid data";
			break;
		case ERROR_INPUT_IMG_VERSION:
			msg = "The image was created by an incompatible newer version of ImPack2";
			break;
		case ERROR_CRC:
			msg = "The data contained inside the image seems to be corrupted";
			break;
		case ERROR_ENCRYPTION_UNAVAILABLE:
			msg = "The image contains encrypted data, but encryption is unsupported by this build of ImPack2";
			break;
		case ERROR_ENCRYPTION_UNKNOWN:
			msg = "The image was created by an incompatible newer version of ImPack2";
			break;
		case ERROR_COMPRESSION_UNAVAILABLE:
			msg = "The image contains compressed data, but compression is unsupported by this build of ImPack2";
			break;
		case ERROR_COMPRESSION_UNSUPPORTED:
			msg = "The image was compressed with an algorithm that is unsupported by this build of ImPack2";
			break;
		case ERROR_COMPRESSION_UNKNOWN:
			msg = "The image was created by an incompatible newer version of ImPack2";
			break;
		default:
			abort();
	}
	show_error(msg);
	
}

void encode_button_click() {
	
	GtkEntry *input_entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeInputText"));
	if (gtk_entry_get_text_length(input_entry) == 0) {
		show_error("Please select an input file");
		return;
	}
	GtkEntry *output_entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeOutputText"));
	if (gtk_entry_get_text_length(output_entry) == 0) {
		show_error("Please select an output file");
		return;
	}
	GtkCheckButton *encrypt_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeEncryptCheckbox"));
	GtkEntry *pass_entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodePassphraseText"));
	GtkEntry *pass_confirm_entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodePassphraseConfirmText"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(encrypt_box))) {
		if (strcmp(gtk_entry_get_text(pass_entry), gtk_entry_get_text(pass_confirm_entry)) != 0) {
			show_error("The passphrases do not match");
			return;
		}
	}
	GtkCheckButton *adv_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeAdvancedCheckbox"));
	GtkCheckButton *color_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorBox"));
	GtkCheckButton *grayscale_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorGrayscaleBox"));
	GtkCheckButton *color_red_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorRedBox"));
	GtkCheckButton *color_green_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorGreenBox"));
	GtkCheckButton *color_blue_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeColorBlueBox"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box))) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_box)) && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(grayscale_box))) {
			if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_red_box)) && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_green_box)) && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_blue_box))) {
				show_error("Please select at least one color channel");
				return;
			}
		}
	}
	
	encode_thread_data_t encode_params;
	encode_params.input_path = (char*) gtk_entry_get_text(input_entry);
	encode_params.output_path = (char*) gtk_entry_get_text(output_entry);
	encode_params.channels = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box)) && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_box))) {
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(grayscale_box))) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_red_box))) {
				encode_params.channels |= CHANNEL_RED;
			}
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_green_box))) {
				encode_params.channels |= CHANNEL_GREEN;
			}
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(color_blue_box))) {
				encode_params.channels |= CHANNEL_BLUE;
			}
		}
	} else {
		encode_params.channels = CHANNEL_RED | CHANNEL_GREEN | CHANNEL_BLUE;
	}
	encode_params.encrypt = ENCRYPTION_NONE;
	encode_params.passphrase = NULL;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(encrypt_box))) {
		GtkComboBoxText *encrypt_type_box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "EncodeEncryptTypeBox"));
		const gchar *passphrase = gtk_entry_get_text(pass_entry);
		encode_params.passphrase = malloc(strlen(passphrase) + 1);
		if (encode_params.passphrase == NULL) {
			show_error("Out of memory");
			return;
		}
		strcpy(encode_params.passphrase, passphrase);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box))) {
			encode_params.encrypt = impack_select_encryption((char*) gtk_combo_box_get_active_id(GTK_COMBO_BOX(encrypt_type_box)));
		} else {
			encode_params.encrypt = impack_default_encryption();
		}
	}
	GtkCheckButton *compress_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCompressCheckbox"));
	encode_params.compress = COMPRESSION_NONE;
	encode_params.compress_level = 0; // TODO
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(compress_box))) {
		GtkComboBoxText *compress_type_box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "EncodeCompressTypeBox"));
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box))) {
			encode_params.compress = impack_select_compression((char*) gtk_combo_box_get_active_id(GTK_COMBO_BOX(compress_type_box)));
		} else {
			encode_params.compress = impack_default_compression();
		}
	}
	GtkCheckButton *width_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeWidthBox"));
	encode_params.img_width = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(width_box))) {
		GtkSpinButton *width_number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeWidthNumber"));
		encode_params.img_width = gtk_spin_button_get_value(width_number);
	}
	GtkCheckButton *height_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeHeightBox"));
	encode_params.img_height = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(height_box))) {
		GtkSpinButton *height_number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeHeightNumber"));
		encode_params.img_height = gtk_spin_button_get_value(height_number);
	}
	encode_params.filename_include = encode_params.input_path; // TODO
	
	GtkStack *main_stack = GTK_STACK(gtk_builder_get_object(builder, "MainStack"));
	GtkStackSwitcher *main_switcher = GTK_STACK_SWITCHER(gtk_builder_get_object(builder, "MainStackSwitcher"));
	GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow"));
	GtkStack *encode_button_stack = GTK_STACK(gtk_builder_get_object(builder, "EncodeButtonStack"));
	GtkSpinner *encode_button_spinner = GTK_SPINNER(gtk_builder_get_object(builder, "EncodeButtonSpinner"));
	gtk_widget_set_sensitive(GTK_WIDGET(main_stack), false);
	gtk_widget_set_sensitive(GTK_WIDGET(main_switcher), false);
	gtk_window_set_deletable(window, false);
	gtk_stack_set_visible_child(encode_button_stack, GTK_WIDGET(encode_button_spinner));
	gtk_spinner_start(encode_button_spinner);
	if (!encode_thread_run(&encode_params)) {
		show_error("Out of memory");
		return;
	}
	free(encode_params.passphrase);
	GtkLabel *encode_button_label = GTK_LABEL(gtk_builder_get_object(builder, "EncodeButtonLabel"));
	gtk_widget_set_sensitive(GTK_WIDGET(main_stack), true);
	gtk_widget_set_sensitive(GTK_WIDGET(main_switcher), true);
	gtk_window_set_deletable(window, true);
	gtk_stack_set_visible_child(encode_button_stack, GTK_WIDGET(encode_button_label));
	gtk_spinner_stop(encode_button_spinner);
	
	if (encode_params.res == ERROR_OK) {
		GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Encoding successful"));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
	} else {
		show_impack_error(encode_params.res);
	}
	
}

void decode_button_click() {
	
	
	
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
	gtk_builder_add_callback_symbol(b, "encode_button_click", encode_button_click);
	gtk_builder_add_callback_symbol(b, "decode_button_click", decode_button_click);
	
}

void window_main_setup(GtkBuilder *b) {
	
	builder = b;
	GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(b, "MainWindow"));
	GError *error;
	GdkPixbuf *icon = gdk_pixbuf_new_from_resource("/impack2/icon.png", &error);
	if (icon == NULL) {
		g_free(error); // Ignore
	} else {
		gtk_window_set_icon(window, icon);
	}
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
