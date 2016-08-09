/* 
 * cado: execute a command in a capability ambient
 * Copyright (C) 2016  Renzo Davoli, University of Bologna
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
#include <string.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <capset_from_namelist.h>

static int addcap(char *name, uint64_t *capset) {
	char *tail;
	uint64_t exacaps=strtoull(name, &tail, 16);
	if (*tail == 0) {
		*capset |= exacaps;
		return 0;
	} else {
		int rv;
		cap_value_t thiscap;
		if (strncmp(name,"cap_",4) == 0) 
			rv=cap_from_name(name, &thiscap);
		else {
			int xnamelen=strlen(name)+5;
			char xname[xnamelen];
			snprintf(xname,xnamelen,"cap_%s",name);
			rv=cap_from_name(xname, &thiscap);
		}
		if (rv >= 0) 
			*capset |= 1ULL << thiscap;
		return rv;
	}
}

/* convert a list of comma separated capability tags to a bitmask of capabilities */
/* capset_from_namelist allows capability names with or without the "cap_" prefix. */
int capset_from_namelist(char *namelist, uint64_t *capset) {
	int rv=0;
	char *onecap;
	char *tmptok;
	char *spacetok;
	*capset = 0;
	for (; (onecap = strtok_r(namelist,",",&tmptok)) != NULL; namelist = NULL)
		rv |= addcap(strtok_r(onecap," \t",&spacetok), capset);
	return rv;
}

