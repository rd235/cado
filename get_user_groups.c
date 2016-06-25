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

#include <get_user_groups.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>

char **get_user_groups(void) {
	uid_t uid=getuid();
	struct passwd *pwd=getpwuid(uid);
	int ngroups=0;
	char **user_groups=NULL;
	getgrouplist(pwd->pw_name, pwd->pw_gid, NULL, &ngroups);
	if (ngroups > 0) {
		gid_t gids[ngroups];
		struct group *grp;
		user_groups = calloc(ngroups+2, sizeof(char *));
		if (user_groups) {
			int i=0;
			user_groups[i++] = strdup(pwd->pw_name);
			if (getgrouplist(pwd->pw_name, pwd->pw_gid, gids, &ngroups) == ngroups) {
				while ((grp=getgrent()) != NULL) {
					int j;
					for (j=0; j<ngroups; j++) {
						if (grp->gr_gid == gids[j]) {
							gids[j] = -1;
							user_groups[i++] = strdup(grp->gr_name);
						}
					}
				}
				user_groups[i] = NULL;
				endgrent();
			}
		}
	}
	return user_groups;
}

