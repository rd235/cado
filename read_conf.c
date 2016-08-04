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
#include <sys/prctl.h>
#include <read_conf.h>
#include <set_ambient_cap.h>
#include <capset_from_namelist.h>
#include <s2argv.h>

#ifndef CONFDIR
#define CONFDIR "/etc"
#endif

#define CADO_CONF CONFDIR "/cado.conf"

/* groupmatch returns 1 if group belongs to grouplist */
static int groupmatch (char *group, char **grouplist) {
	for (;*grouplist; grouplist++) {
		//printf("%s %s\n",group, *grouplist);
		if (strcmp(group, *grouplist) == 0)
			return 1;
	}
	return 0;
}

/* s2argv security, children must drop their capabilities */
static int drop_capabilities(void *useless) {
	return prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0);
}

/* get_authorized_caps returns the set of authorized capabilities
	 for the user user_groups[0] belonging to the groups user_groups[1:] */
/* if user_groups==NULL, get_authorized_caps computes the maximum set
	 of capabilities that cado itself must own to be able to assign */
uint64_t get_authorized_caps(char **user_groups, uint64_t reqset) {
	uint64_t ok_caps=0;
	FILE *f;
	/* cado.conf is not readble by users. Add the capability to do it */
	if (user_groups) raise_cap_dac_read_search();
	f=fopen(CADO_CONF, "r");
	if (f) {
		char *line=NULL;
		ssize_t len,n=0;
		/* set s2argv security, children must drop their capabilities */
		s2_fork_security=drop_capabilities;
		while ((len=getline(&line, &n, f)) > 0 && (reqset & ~ok_caps)) {
			//printf("%s",line);
			char *scan=line;
			char *tokencap;
			char *tokenusergroup;
			char *tokencondition;
			char *tok;
			uint64_t capset;
			char *tmptok;
			int usermatch=0;
			/* skip leading spaces */
			while (isspace(*scan)) scan++;
			if (*scan == 0 || *scan == '#') //comment
				continue;
			tokencap=strtok_r(scan, ":", &tmptok);
			//printf("CAP %s\n",tokencap);
			tokenusergroup=strtok_r(NULL, ":\n", &tmptok);
			//printf("UG %s\n",tokenusergroup);
			tokencondition=strtok_r(NULL, ":\n", &tmptok);
			//printf("COND %s\n",tokencondition);
			capset=0;
			if (capset_from_namelist(tokencap, &capset) < 0)
				continue;
			if (user_groups == NULL) {
				ok_caps |= capset;
				continue;
			} 
			//printf("CAP %s %d\n",tok,thiscap);
			while ((tok=strtok_r(tokenusergroup, ",\n ",&tmptok)) != NULL) {
				//printf("XX %s\n",tok);
				if (*tok=='@') {
					if (groupmatch(tok+1, user_groups+1)) {
						usermatch = 1;
						break;
					}
				} else if (strcmp(tok, user_groups[0]) == 0) {
					usermatch = 1;
					break;
				}
				tokenusergroup=NULL;
			}
			if (usermatch) {
				if (tokencondition) {
					if (system_execsa(tokencondition) == 0)
						ok_caps |= capset;
				} else
					ok_caps |= capset;
			}
		}
		fclose(f);
		if (line)
			free(line);
	}
	/* the capability to read cado.conf is no longer needed */
	if (user_groups) lower_cap_dac_read_search();
	return ok_caps;
}

/* set_self_capability sets the capability set needed by cado itself */
int set_self_capability(uint64_t capset) {
	cap_value_t cap;
	cap_t caps=cap_init();
	int f,rv=-1;
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (capset & (1ULL << cap)) {
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
