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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <read_conf.h>
#include <set_ambient_cap.h>
#include <capset_from_namelist.h>

#ifndef CONFDIR
#define CONFDIR "/etc"
#endif

#define CADO_CONF CONFDIR "/cado.conf"

static int groupmatch (char *group, char **grouplist) {
	for (;*grouplist; grouplist++) {
		//printf("%s %s\n",group, *grouplist);
		if (strcmp(group, *grouplist) == 0)
			return 1;
	}
	return 0;
}

uint64_t get_authorized_caps(char **user_groups) {
	uint64_t ok_caps=0;
	FILE *f;
	if (user_groups) raise_cap_dac_read_search();
	f=fopen(CADO_CONF, "r");
	if (f) {
		char *line=NULL;
		ssize_t len,n=0;
		while ((len=getline(&line, &n, f)) > 0) {
			//printf("%s",line);
			char *scan=line;
			char *tok;
			uint64_t capset;
			char *tmptok;
			while (isspace(*scan)) scan++;
			if (*scan == 0 || *scan == '#') //comment
				continue;
			tok=strtok_r(scan, ":", &tmptok);
			//printf("%s\n",tok);
			capset=0;
			if (capset_from_namelist(tok, &capset) < 0)
				continue;
			if (user_groups == NULL) {
				ok_caps |= capset;
				continue;
			} 
			//printf("CAP %s %d\n",tok,thiscap);
			while ((tok=strtok_r(NULL, ",\n ",&tmptok)) != NULL) {
				//printf("XX %s\n",tok);
				if (*tok=='@') {
					if (groupmatch(tok+1, user_groups+1)) {
						ok_caps |= capset;
						break;
					}
				} else if (strcmp(tok, user_groups[0]) == 0) {
					ok_caps |= capset;
					break;
				}
			}
		}
		fclose(f);
		if (line)
			free(line);
	}
	if (user_groups) lower_cap_dac_read_search();
	return ok_caps;
}

int set_self_capability(uint64_t capset) {
	cap_value_t cap;
	cap_t caps=cap_init();
	int f,rv=-1;
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (capset & (1ULL << cap)) {
			/*if (cap_set_flag(caps, CAP_PERMITTED, 1, &cap, CAP_SET) ||
					cap_set_flag(caps, CAP_EFFECTIVE, 1, &cap, CAP_SET)) {*/
			if (cap_set_flag(caps, CAP_PERMITTED, 1, &cap, CAP_SET)) {
				fprintf(stderr, "Cannot set permitted cap %s\n",cap_to_name(cap));
				exit(2);
			}
		}
	}
	if ((f=open("/proc/self/exe",O_RDONLY)) >= 0) {
		if (cap_set_fd(f,caps) >= 0)
			rv=0;
		close(f);
	}
	cap_free(caps);
	return rv;
}
