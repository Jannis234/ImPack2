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

#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef IMPACK_WINDOWS
#include <termios.h>
#endif
#include "impack.h"

#define BUFSTEP 25

char* impack_readpass() {
	
	// TODO: ncurses
#ifndef IMPACK_WINDOWS
	struct termios term_old, term_new;
	bool silence_success = false;
	if (tcgetattr(STDIN_FILENO, &term_old) == 0) {
		silence_success = true;
		term_new = term_old;
		term_new.c_lflag &= ~ECHO;
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_new);
	}
#endif
	char *res = malloc(BUFSTEP);
	if (res == NULL) {
		return NULL;
	}
	uint64_t res_pos = 0;
	uint64_t res_size = BUFSTEP;
	char c;
	do {
		if (fread(&c, 1, 1, stdin) != 1) {
			return NULL;
		}
		res[res_pos] = c;
		res_pos++;
		if (res_pos == res_size) {
			char *res_new = realloc(res, res_size + BUFSTEP);
			if (res_new == NULL) {
				free(res);
				return NULL;
			}
			res = res_new;
			res_size += BUFSTEP;
		}
	} while (c != '\n' && c != '\r');
	res[res_pos - 1] = 0; // Replaces the newline
#ifndef IMPACK_WINDOWS
	if (silence_success) {
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_old);
		fprintf(stderr, "\n");
	}
#endif
	return res;
	
}
