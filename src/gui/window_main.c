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
#include "impack_internal.h"
#include "gui.h"

GtkBuilder *builder;
bool encode_color_checkbox_state[4];
bool encode_color_checkbox_enabled;
bool encode_compress_box_enabled;

int enabled_compression_types() {
	
#ifdef IMPACK_WITH_COMPRESSION
	int res = 0;
	while (impack_compression_types[res] != NULL) {
		res++;
	}
	return res;
#else
	return 0;
#endif
	
}

void build_encode_compression_type_box(bool advanced) {
	
#ifdef IMPACK_WITH_COMPRESSION
	encode_compress_box_enabled = false;
	GtkComboBoxText *box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "EncodeCompressTypeBox"));
	const gchar *last_active = gtk_combo_box_get_active_id(GTK_COMBO_BOX(box));
	char *next_active = NULL;
	if (advanced) {
		if (strcmp(last_active, "strong") == 0) { // Keep LZMA2 if selected, otherwise select the default
			next_active = "LZMA2";
		}
	} else {
		if (strcmp(last_active, "LZMA2") == 0) {
			next_active = "strong";
		} else {
			next_active = "normal";
		}
	}
	if (next_active == NULL) {
		int current = 0;
		while (impack_compression_types[current] != NULL) {
			if (impack_compression_types[current]->id == impack_default_compression()) {
				next_active = impack_compression_types[current]->name;
			}
			current++;
		}
	}
	gtk_combo_box_text_remove_all(box);
	if (advanced) {
		int current = 0;
		while (impack_compression_types[current] != NULL) {
			gtk_combo_box_text_append(box, impack_compression_types[current]->name, impack_compression_types[current]->name);
			current++;
		}
	} else {
		gtk_combo_box_text_append(box, "normal", "Normal (Fast)");
#ifdef IMPACK_WITH_LZMA
		gtk_combo_box_text_append(box, "strong", "Strong (Slow)");
#endif
	}
	encode_compress_box_enabled = true;
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(box), next_active);
#endif
	
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
	GtkRevealer *compress_reveal = GTK_REVEALER(gtk_builder_get_object(builder, "EncodeCompressAdvancedReveal"));
	gtk_revealer_set_reveal_child(compress_reveal, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
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

void encode_compress_level_checkbox_toggled() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCompressLevelCheckbox"));
	GtkSpinButton *number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeCompressLevelNumber"));
	gtk_widget_set_sensitive(GTK_WIDGET(number), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	
}

void encode_custom_filename_toggled() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCustomFilenameCheckbox"));
	GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeCustomFilenameText"));
	gtk_widget_set_sensitive(GTK_WIDGET(entry), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box)));
	
}

void encode_no_filename_toggled() {
	
	GtkCheckButton *box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeNoFilenameCheckbox"));
	GtkCheckButton *box_custom = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCustomFilenameCheckbox"));
	GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeCustomFilenameText"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(box))) {
		gtk_widget_set_sensitive(GTK_WIDGET(box_custom), false);
		gtk_widget_set_sensitive(GTK_WIDGET(entry), false);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(box_custom), true);
		encode_custom_filename_toggled();
	}
	
}

void encode_compress_type_change() {
	
#ifdef IMPACK_WITH_COMPRESSION
	if (encode_compress_box_enabled) { // Prevent this from running during build_encode_compression_type_box()
		GtkSpinButton *number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeCompressLevelNumber"));
		GtkAdjustment *adj = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "EncodeCompressLevelAdjust"));
		GtkComboBoxText *box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "EncodeCompressTypeBox"));
		const gchar *active = gtk_combo_box_get_active_id(GTK_COMBO_BOX(box));
		impack_compression_type_t compress_type = COMPRESSION_NONE;
		if (strcmp(active, "normal") == 0) {
			compress_type = impack_default_compression();
		} else if (strcmp(active, "strong") == 0) {
			compress_type = COMPRESSION_LZMA;
		} else {
			compress_type = impack_select_compression((char*) active);
		}
		int level = 1;
		while (impack_compress_level_valid(compress_type, level)) {
			level++;
		}
		level--;
		gtk_adjustment_set_upper(adj, level);
		gtk_spin_button_set_value(number, level);
	}
#endif
	
}

GtkFileChooserDialog* create_open_file_dialog(bool filter) {
	
	GtkFileChooserDialog *dialog = GTK_FILE_CHOOSER_DIALOG(gtk_file_chooser_dialog_new("Select input file", GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL));
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
	if (filter) {
		GtkFileFilter *img_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(img_filter, "Images");
		int i = 0;
		while (impack_img_formats[i] != NULL) {
			const impack_img_format_desc_t *current = impack_img_formats[i];
			gtk_file_filter_add_pattern(img_filter, current->extension);
			if (current->extension_alt != NULL) {
				int j = 0;
				while (current->extension_alt[j] != NULL) {
					gtk_file_filter_add_pattern(img_filter, current->extension_alt[j]);
					j++;
				}
			}
			i++;
		}
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
	int current = 0;
	char *default_format = NULL;
	while (impack_img_formats[current] != NULL) {
		gtk_combo_box_text_append(cbox, impack_img_formats[current]->extension, impack_img_formats[current]->name);
		if (impack_default_img_format() == impack_img_formats[current]->id) {
			default_format = impack_img_formats[current]->extension;
		}
		current++;
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

void show_impack_error(impack_error_t error, bool maybe_passphrase) {
	
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
			if (maybe_passphrase) {
				msg = "The image contains invalid data\nNote: This may be caused by an incorrect passphrase";
			}
			break;
		case ERROR_INPUT_IMG_VERSION:
			msg = "The image was created by an incompatible newer version of ImPack2";
			break;
		case ERROR_CRC:
			msg = "The data contained inside the image seems to be corrupted";
			if (maybe_passphrase) {
				msg = "The data contained inside the image seems to be corrupted\nNote: This may be caused by an incorrect passphrase";
			}
			break;
		case ERROR_ENCRYPTION_UNAVAILABLE:
			msg = "The image contains encrypted data, but encryption is unsupported by this build of ImPack2";
			break;
		case ERROR_ENCRYPTION_UNSUPPORTED:
			msg = "The image was encrypted with an algorithm that is unsupported by this build of ImPack2";
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
#ifdef IMPACK_WITH_ARGON2
			bool want_pbkdf2 = false;
#else
			bool want_pbkdf2 = true;
#endif
			encode_params.encrypt = impack_select_encryption((char*) gtk_combo_box_get_active_id(GTK_COMBO_BOX(encrypt_type_box)), want_pbkdf2);
		} else {
			encode_params.encrypt = impack_default_encryption(false);
		}
	}
	encode_params.compress = COMPRESSION_NONE;
	encode_params.compress_level = 0;
#ifdef IMPACK_WITH_COMPRESSION
	GtkCheckButton *compress_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCompressCheckbox"));
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(compress_box))) {
		GtkComboBoxText *compress_type_box = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "EncodeCompressTypeBox"));
		const gchar *compress_type = gtk_combo_box_get_active_id(GTK_COMBO_BOX(compress_type_box));
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box))) {
			encode_params.compress = impack_select_compression((char*) compress_type);
			GtkCheckButton *compress_level_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeCompressLevelCheckbox"));
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(compress_level_box))) {
				GtkSpinButton *compress_level_number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeCompressLevelNumber"));
				encode_params.compress_level = gtk_spin_button_get_value_as_int(compress_level_number);
			}
		} else {
			if (strcmp(compress_type, "normal") == 0) {
				encode_params.compress = impack_default_compression();
			} else {
				encode_params.compress = COMPRESSION_LZMA;
			}
		}
	}
#endif
	GtkCheckButton *width_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeWidthBox"));
	encode_params.img_width = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(width_box))) {
		GtkSpinButton *width_number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeWidthNumber"));
		encode_params.img_width = gtk_spin_button_get_value_as_int(width_number);
	}
	GtkCheckButton *height_box = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "EncodeHeightBox"));
	encode_params.img_height = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(height_box))) {
		GtkSpinButton *height_number = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "EncodeHeightNumber"));
		encode_params.img_height = gtk_spin_button_get_value_as_int(height_number);
	}
	encode_params.filename_include = encode_params.input_path;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_box))) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "EncodeNoFilenameCheckbox")))) {
			encode_params.filename_include = "out"; // Use a placeholder instead
		} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "EncodeCustomFilenameCheckbox")))) {
			GtkEntry *custom_filename_entry = GTK_ENTRY(gtk_builder_get_object(builder, "EncodeCustomFilenameText"));
			encode_params.filename_include = (char*) gtk_entry_get_text(custom_filename_entry);
		}
	}
	
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
	} else {
		if (encode_params.res == ERROR_OK) {
			GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Encoding successful"));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(GTK_WIDGET(dialog));
		} else {
			show_impack_error(encode_params.res, false);
		}
	}
	
	free(encode_params.passphrase);
	GtkLabel *encode_button_label = GTK_LABEL(gtk_builder_get_object(builder, "EncodeButtonLabel"));
	gtk_widget_set_sensitive(GTK_WIDGET(main_stack), true);
	gtk_widget_set_sensitive(GTK_WIDGET(main_switcher), true);
	gtk_window_set_deletable(window, true);
	gtk_stack_set_visible_child(encode_button_stack, GTK_WIDGET(encode_button_label));
	gtk_spinner_stop(encode_button_spinner);
	
}

void decode_button_click() {
	
	GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(builder, "DecodeInputText"));
	if (gtk_entry_get_text_length(entry) == 0) {
		show_error("Please select an input file");
		return;
	}
	
	GtkStack *main_stack = GTK_STACK(gtk_builder_get_object(builder, "MainStack"));
	GtkStackSwitcher *main_switcher = GTK_STACK_SWITCHER(gtk_builder_get_object(builder, "MainStackSwitcher"));
	GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow"));
	GtkStack *decode_button_stack = GTK_STACK(gtk_builder_get_object(builder, "DecodeButtonStack"));
	GtkSpinner *decode_button_spinner = GTK_SPINNER(gtk_builder_get_object(builder, "DecodeButtonSpinner"));
	gtk_widget_set_sensitive(GTK_WIDGET(main_stack), false);
	gtk_widget_set_sensitive(GTK_WIDGET(main_switcher), false);
	gtk_window_set_deletable(window, false);
	gtk_stack_set_visible_child(decode_button_stack, GTK_WIDGET(decode_button_spinner));
	gtk_spinner_start(decode_button_spinner);
	
	bool running = true;
	decode_thread_data_t decode_params;
	decode_params.stage = 0;
	decode_params.input_path = (char*) gtk_entry_get_text(entry);
	if (!decode_thread_run(&decode_params)) {
		show_error("Out of memory");
		running = false;
	} else {
		if (decode_params.res != ERROR_OK) {
			show_impack_error(decode_params.res, false);
			running = false;
		}
	}
	decode_params.stage = 1;
	decode_params.passphrase = NULL;
	if (running && decode_params.state.encryption != ENCRYPTION_NONE) {
		GtkDialog *dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Enter passphrase", GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_DIALOG_DESTROY_WITH_PARENT, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_ACCEPT, NULL));
		GtkBox *dialog_box = GTK_BOX(gtk_dialog_get_content_area(dialog));
		GtkLabel *dialog_label = GTK_LABEL(gtk_label_new("The image contains encrypted data, please enter the passphrase"));
		gtk_widget_set_margin_start(GTK_WIDGET(dialog_label), 10);
		gtk_widget_set_margin_end(GTK_WIDGET(dialog_label), 10);
		gtk_widget_set_margin_top(GTK_WIDGET(dialog_label), 10);
		gtk_box_pack_start(dialog_box, GTK_WIDGET(dialog_label), false, false, 0);
		GtkEntry *dialog_entry = GTK_ENTRY(gtk_entry_new());
		gtk_entry_set_visibility(dialog_entry, false);
		gtk_widget_set_margin_top(GTK_WIDGET(dialog_entry), 10);
		gtk_widget_set_margin_bottom(GTK_WIDGET(dialog_entry), 10);
		gtk_box_pack_start(dialog_box, GTK_WIDGET(dialog_entry), false, false, 0);
		gtk_widget_show(GTK_WIDGET(dialog_label));
		gtk_widget_show(GTK_WIDGET(dialog_entry));
		if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT) {
			const gchar *passphrase = gtk_entry_get_text(dialog_entry);
			decode_params.passphrase = malloc(strlen(passphrase) + 1);
			if (decode_params.passphrase == NULL) {
				show_error("Out of memory");
				free(decode_params.state.pixeldata);
				running = false;
			} else {
				strcpy(decode_params.passphrase, passphrase);
			}
		} else {
			free(decode_params.state.pixeldata);
			running = false;
		}
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}
	if (running) {
		if (!decode_thread_run(&decode_params)) {
			show_error("Out of memory");
			running = false;
		} else {
			if (decode_params.res != ERROR_OK) {
				show_impack_error(decode_params.res, true);
				running = false;
			}
		}
	}
	if (running) {
		GtkFileChooserDialog *dialog = GTK_FILE_CHOOSER_DIALOG(gtk_file_chooser_dialog_new("Select output file", GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL));
		gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), false);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), decode_params.state.filename);
		if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
			decode_params.output_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		} else {
#ifdef IMPACK_WITH_CRYPTO
			if (decode_params.state.encryption != ENCRYPTION_NONE) {
				impack_secure_erase(decode_params.state.crypt_key, IMPACK_CRYPT_KEY_SIZE);
			}
#endif
			free(decode_params.state.pixeldata);
			free(decode_params.state.filename);
			running = false;
		}
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}
	decode_params.stage = 2;
	if (running) {
		if (!decode_thread_run(&decode_params)) {
			g_free(decode_params.output_path);
			show_error("Out of memory");
		} else {
			g_free(decode_params.output_path);
			if (decode_params.res != ERROR_OK) {
				show_impack_error(decode_params.res, true);
			} else {
				GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG(gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(builder, "MainWindow")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Decoding successful"));
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(GTK_WIDGET(dialog));
			}
		}
	}
	
	GtkLabel *decode_button_label = GTK_LABEL(gtk_builder_get_object(builder, "DecodeButtonLabel"));
	gtk_widget_set_sensitive(GTK_WIDGET(main_stack), true);
	gtk_widget_set_sensitive(GTK_WIDGET(main_switcher), true);
	gtk_window_set_deletable(window, true);
	gtk_stack_set_visible_child(decode_button_stack, GTK_WIDGET(decode_button_label));
	gtk_spinner_stop(decode_button_spinner);
	
}

void about_button_click() {
	
	GtkAboutDialog *dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
	gtk_about_dialog_set_program_name(dialog, "ImPack2");
	gtk_about_dialog_set_version(dialog, IMPACK_VERSION_STRING);
	gtk_about_dialog_set_license_type(dialog, GTK_LICENSE_GPL_3_0);
	GError *error = NULL;
	GdkPixbuf *icon = gdk_pixbuf_new_from_resource("/impack2/icon.png", &error);
	if (icon == NULL) {
		g_free(error); // Ignore
	} else {
		gtk_about_dialog_set_logo(dialog, icon);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
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
	gtk_builder_add_callback_symbol(b, "encode_no_filename_toggled", encode_no_filename_toggled);
	gtk_builder_add_callback_symbol(b, "encode_custom_filename_toggled", encode_custom_filename_toggled);
	gtk_builder_add_callback_symbol(b, "encode_compress_level_checkbox_toggled", encode_compress_level_checkbox_toggled);
	gtk_builder_add_callback_symbol(b, "encode_compress_type_change", encode_compress_type_change);
	gtk_builder_add_callback_symbol(b, "encode_open_button_click", encode_open_button_click);
	gtk_builder_add_callback_symbol(b, "decode_open_button_click", decode_open_button_click);
	gtk_builder_add_callback_symbol(b, "encode_output_button_click", encode_output_button_click);
	gtk_builder_add_callback_symbol(b, "encode_button_click", encode_button_click);
	gtk_builder_add_callback_symbol(b, "decode_button_click", decode_button_click);
	gtk_builder_add_callback_symbol(b, "about_button_click", about_button_click);
	
}

void window_main_setup(GtkBuilder *b) {
	
	builder = b;
	GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(b, "MainWindow"));
	GError *error = NULL;
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
