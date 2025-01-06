/*
 * cado: execute a command in a capability ambient
 * Copyright (C) 2024-2025 Renzo Davoli, University of Bologna
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <cado_paths.h>
#include <set_ambient_cap.h>
#include <capset_from_namelist.h>
#include <file_utils.h>
#include <sys/fsuid.h>

#ifndef CONFDIR
#define CONFDIR "/etc"
#endif

#define CADO_CONF CONFDIR "/cado.conf"

/* Get the user cado file */
static inline int get_lp_file(const char *username, char *path) {
  return snprintf(path, PATH_MAX, "%s/cado_%s", SPOOL_DIR,  username);
}

/* cado.conf management */
struct usercap {
	struct usercap *next;
	uint64_t capset;
	char user[];
};

struct usercap *uc_root;

static void uc_add(char *user, uint64_t capset) {
	struct usercap **scan;
	for (scan = &uc_root; *scan != NULL; scan = &((*scan)->next))
		if (strcmp(user, (*scan)->user) == 0)
			break;
	if (*scan == NULL) {
		struct usercap *this = malloc(sizeof(*this) + strlen(user) + 1);
		if (this) {
			this->next = 0;
			this->capset = 0;
			strcpy(this->user, user);
			*scan = this;
		}
	}
	if (*scan != NULL)
		(*scan)->capset |= capset;
}

static void uc_ugadd(char *ug, uint64_t capset) {
	if (ug[0] == '@') {
		  struct group *grp = getgrnam(ug + 1);
			if (grp) {
				for(char **member = grp->gr_mem; *member != NULL; member++)
					uc_add(*member, capset);
			}
	} else
		uc_add(ug, capset);
}

#if 0
static void listcap(void) {
	for (struct usercap *scan = uc_root; scan != NULL; scan = scan->next)
		printf("%s %lx\n", scan->user, scan->capset);
}
#endif

int set_fd_capability(int fd, uint64_t capset) {
  cap_value_t cap;
  cap_t caps=cap_init();
  int rv=-1;
  for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
    if (capset & (1ULL << cap)) {
      if (cap_set_flag(caps, CAP_PERMITTED, 1, &cap, CAP_SET)) {
        fprintf(stderr, "Cannot set permitted cap %s\n",cap_to_name(cap));
        exit(2);
      }
    }
  }
	if (cap_set_fd(fd,caps) >= 0)
      rv=0;
  cap_free(caps);
  return rv;
}

static void rm_oldcap(void) {
	int dirfd = open(SPOOL_DIR, O_RDONLY | O_DIRECTORY);
	if (dirfd >= 0) {
		DIR *d = fdopendir(dirfd);
		if (d) {
			struct dirent *de;
			while ((de = readdir(d)) != NULL) {
				if (strncmp(de->d_name, "cado_", 5) == 0)
					unlinkat(dirfd,  de->d_name, 0);
			}
		}
		close(dirfd);
	}
}

static void setlpcap(void) {
	for (struct usercap *scan = uc_root; scan != NULL; scan = scan->next) {
		struct passwd *pwd = getpwnam(scan->user);
		if (pwd) {
			char lp_path[PATH_MAX];
			get_lp_file(pwd->pw_name, lp_path);
			// printf("lp %s\n", lp_path);
			int fdin = open("/proc/self/exe", O_RDONLY);
			int fdout = open(lp_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
			if (fdin >= 0 && fdout >=0) {
				if (copyfile(fdin, fdout) < 0) {
					fprintf(stderr, "Cannot copy exe to %s\n", lp_path);
					exit(2);
				}
				if (fchown(fdout, pwd->pw_uid, -1) < 0) {
					fprintf(stderr, "Cannot chown of %s\n", lp_path);
					exit(2);
				}
				fchmod(fdout, 00500);
				set_fd_capability(fdout, scan->capset);
			}
			close(fdin);
			close(fdout);
		}
	}
}

static void usercap(void) {
	FILE *f;
	f=fopen(CADO_CONF, "r");
	if (f) {
		char *line=NULL;
		size_t n=0;
		while (getline(&line, &n, f) > 0) {
			//printf("%s",line);
			char *scan=line;
			char *tokencap;
			char *tokenusergroup;
			//char *tokencondition;
			char *tok;
			uint64_t capset;
			char *tmptok;
			/* skip leading spaces */
			while (isspace(*scan)) scan++;
			if (*scan == 0 || *scan == '#') //comment
				continue;
			tokencap=strtok_r(scan, ":", &tmptok);
			//printf("CAP %s\n",tokencap);
			tokenusergroup=strtok_r(NULL, ":\n", &tmptok);
			//printf("UG %s\n",tokenusergroup);
			//tokencondition=strtok_r(NULL, ":\n", &tmptok);
			//printf("COND %s\n",tokencondition);
			if (capset_from_namelist(tokencap, &capset) < 0)
				continue;
			//printf("CAP %s %d\n",tok,thiscap);
			while ((tok=strtok_r(tokenusergroup, ",\n ",&tmptok)) != NULL) {
				//printf("XX %s\n",tok);
				uc_ugadd(tok, capset);
				tokenusergroup=NULL;
			}
		}
		fclose(f);
		if (line)
			free(line);
	}
}

/* setup least privilege copies of cado, one for each user */
int leastpriv_setup(int onoff) {
	if (onoff) {
		usercap();
		rm_oldcap();
		setlpcap();
	} else
		rm_oldcap();
	return 0;
}

/* Reload the user specific copy of cado */
int leastpriv_run(int argc, char *argv[]) {
	if (argv[0][0] != '-') {
		size_t newarg0len = strlen(argv[0]) + 2;
		char newarg0[newarg0len];
		snprintf(newarg0,newarg0len,"-%s",argv[0]);
		char *newargv[argc + 1];
		char lp_path[PATH_MAX];
		struct passwd *pwd = getpwuid(getuid());
		newargv[0] = newarg0;
		for (int i = 1; i <= argc; i++)
			newargv[i] = argv[i];
		if (pwd) {
			get_lp_file(pwd->pw_name, lp_path);

			setfsuid(getuid());
			int rv =  execv(lp_path, newargv);
			setfsuid(geteuid());
			return (errno == ENOENT) ? 0 : rv;
		}
	}
	return 0;
}
