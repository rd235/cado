/* 
 * cado: execute a command in a capability ambient
 * Copyright (C) 2016  Renzo Davoli, Davide Berardi, University of Bologna
 * 
 * This file is part of cado.
 *
 * Cado is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <set_ambient_cap.h>
#include <capset_from_namelist.h>

static int argv1_all(char *s) {
	if (s == NULL)
		return 1;
	if (strcmp(s,"") == 0)
		return 1;
	if (strcmp(s,"-") == 0)
		return 1;
	if (strcmp(s,"all") == 0)
		return 1;
	return 0;
}

int main(int argc, char *argv[]) {
	uint64_t capset = -1;
	char *argvsh[]={getenv("SHELL"),NULL};
	switch (argc) {
		case 1:
			capset = -1;
			argv += 1;
			break;
		default:
			if (argv1_all(argv[1]))
				capset = -1; 
			else if (capset_from_namelist(argv[1], &capset) < 0) {
				fprintf(stderr, "error parsing capabilities\n");
				exit(2);
			}
			argv+=2;
			break;
	}
	if (*argv == NULL) argv = argvsh;
	drop_ambient_cap(capset);
	execvp(argv[0],argv);
	perror("exec");
	return 2; 
}   

