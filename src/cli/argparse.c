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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

bool impack_argparse(impack_argparse_t *options, size_t options_count, char **argv, int argc) {
	
	for (int i = 1; i < argc; i++) {
		if (strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == '-') {
			bool valid = false;
			for (int j = 0; j < options_count; j++) {
				if (options[j].name_long != NULL) {
					if (strcmp(options[j].name_long, argv[i] + 2) == 0) {
						options[j].found = true;
						if (options[j].needs_arg) {
							if (argc == i + 1) {
								fprintf(stderr, "Argument required for option: %s\n", argv[i]);
								return false;
							}
							options[j].arg_out = argv[i + 1];
							i++;
						}
						valid = true;
						break;
					}
				}
			}
			if (!valid) {
				fprintf(stderr, "Unknown argument: %s\n", argv[i]);
				return false;
			}
		} else if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
			for (int j = 1; j < strlen(argv[i]); j++) {
				bool valid = false;
				for (int k = 0; k < options_count; k++) {
					if (options[k].name_short == argv[i][j]) {
						options[k].found = true;
						if (options[k].needs_arg) {
							if (argc == i + 1 || j != strlen(argv[i]) - 1) {
								fprintf(stderr, "Argument required for option: -%c\n", argv[i][j]);
								return false;
							}
							options[k].arg_out = argv[i + 1];
							i++;
						}
						valid = true;
						break;
					}
				}
				if (!valid) {
					fprintf(stderr, "Unknown argument: -%c\n", argv[i][j]);
					return false;
				} else {
					break;
				}
			}
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			return false;
		}
	}
	return true;
	
}

int impack_find_option(impack_argparse_t *options, size_t options_count, bool name_long, char *name) {
	
	for (int i = 0; i < options_count; i++) {
		if (name_long) {
			if (options[i].name_long != NULL) {
				if (strcmp(options[i].name_long, name) == 0) {
					return i;
				}
			}
		} else {
			if (options[i].name_short == name[0]) {
				return i;
			}
		}
	}
	abort(); // Searching for a non-existing option
	
}
