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
#include <getopt.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <inttypes.h>

#include <pam_check.h>
#include <get_user_groups.h>
#include <capset_from_namelist.h>
#include <read_conf.h>
#include <set_ambient_cap.h>
#include <cado_scado_check.h>

/* print a capset (in case of -v, verbose mode). */
static void printcapset(uint64_t capset, char *indent) {
	if (capset) {
	cap_value_t cap;
	int count=0;
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (capset & (1ULL << cap)) {
			count ++;
			printf("%s%2d %016" PRIx64 " %s\n", indent, cap, UINT64_C(1) << cap, cap_to_name(cap));
		}
	}
	if (count > 1)
		printf("%s   %016" PRIx64 "\n", indent, capset);
	} else
		printf("%s   %016" PRIx64 " NONE\n",indent, UINT64_C(0));
}

/* command line args management */
#define OPTSTRING "hfvsS"
struct option long_options[]={
	{"help", no_argument, NULL, 'h'},
	{"force", no_argument, NULL, 'f'},
	{"verbose", no_argument, NULL, 'v'},
	{"setcap", no_argument, NULL, 's'},
	{"scado", no_argument, NULL, 'S'},
	{0,0,0,0}
};

void usage(char *progname) {
	fprintf(stderr,"%s - execute a command in a different capability ambient\n\n",progname);
	fprintf(stderr,"usage: %s OPTIONS capability_list [command [args]]\n\n",progname);
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  -h, --help         display help message and exit\n");
	fprintf(stderr,"  -f, --force        do not display warnings, do what is allowed\n");
	fprintf(stderr,"  -v, --verbose      generate extra output\n");
	fprintf(stderr,"  -S, --scado        check scado pre-authorization for scripts\n");
	fprintf(stderr,"  -s, --setcap       set the minimun caps for %s (root access)\n",progname);
	exit(1);
}

int main(int argc, char*argv[])
{
	char *progname=basename(argv[0]);
	char **user_groups=get_user_groups();
	uint64_t okcaps;
	uint64_t reqcaps;
	uint64_t grantcap=0;
	int verbose=0;
	int force=0;
	int setcap=0;
	int scado=0;
	int pam_check_required = 1;
	char copy_path[PATH_MAX] = "";
	char *argvsh[]={getenv("SHELL"),NULL};
	char **cmdargv;

	while (1) {
		int c=getopt_long(argc, argv, OPTSTRING, long_options, NULL);
		if (c < 0) 
			break;
		switch (c) {
			case 'h': usage(progname);
								break;
			case 'v': verbose=1;
								break;
			case 'f': force=1;
								break;
			case 's': setcap=1;
								break;
			case 'S': scado=1;
								break;
		}
	}

	/* setcap mode: cado sets the minimal required set of capability required by itself */

	if (setcap) {
		if (setuid(0) != 0 || geteuid() != 0) {
			fprintf(stderr, "setcap requires root access %d\n",geteuid());
			exit(2);
		}
		okcaps = get_authorized_caps(NULL, -1LL);
		okcaps |= 1ULL << CAP_DAC_READ_SEARCH;
		if (verbose) {
			printf("Capability needed by %s:\n", progname);
			printcapset(okcaps, "  ");
		}
		if (set_self_capability(okcaps) < 0) {
			fprintf(stderr, "Cannot set %s capabilities\n", progname);
			exit(2);
		}
		exit(0);
	}
		
	if (user_groups == NULL) {
		fprintf(stderr, "No passwd entry for user '%d'\n",getuid());
		exit(2);
	}

	/* -v without any other parameter: cado shows the set of ambient capabilities allowed for the current user/group */
	if (verbose && (argc == optind)) {
		okcaps=get_authorized_caps(user_groups, -1LL);
		printf("Allowed ambient capabilities:\n");
		printcapset(okcaps, "  ");
		exit(0);
	}

	if (argc - optind < 1)
		usage(progname);

	/* parse the set of requested capabilities */
	if (capset_from_namelist(argv[optind], &reqcaps)) {
		fprintf(stderr, "List of capabilities: syntax error\n");
		exit(2);
	}

	if (verbose) {
		printf("Requested ambient capabilities:\n");
		printcapset(reqcaps, "  ");
	}

	/* check if the capability requested are also allowed */
	okcaps=get_authorized_caps(user_groups, reqcaps);

	optind++;

	if (optind < argc)
		cmdargv = argv + optind;
	else {
		cmdargv = argvsh;
		if (cmdargv[0] == NULL) {
			fprintf(stderr, "Error: $SHELL env variable not set.\n");
			exit(1);
		}
	}


	/* scado mode, check if there is a pre-authorization for the command */
	if (scado) {
		uint64_t scado_caps = cado_scado_check(user_groups[0], cmdargv[0], copy_path);
		if (verbose) {
			printf("Scado permitted capabilities for %s:\n", cmdargv[0]);
			printcapset(scado_caps, "  ");
		}
		okcaps &= scado_caps;
		pam_check_required = 0;
	} 

	/* the user requested capabilities which are not allowed */
	if (reqcaps & ~okcaps) {
		if (verbose) {
			printf("Unavailable ambient capabilities:\n");
			printcapset(reqcaps & ~okcaps, "  ");
		}
		/* if not in "force" mode, do not complaint */
		if (!force) {
			fprintf(stderr,"%s: Permission denied\n",progname);
			exit(2);
		}
	}

	grantcap = reqcaps & okcaps;

	/* revert setgid mode */
	if (setuid(getuid()) < 0) {
		fprintf(stderr,"%s: setuid failure\n",progname);
		exit(2);
	}

	/* ask for pam authorization (usually password) if required */
	if (pam_check_required && pam_check(user_groups[0]) != PAM_SUCCESS) {
		fprintf(stderr,"%s: Authentication failure\n",progname);
		exit(2);
	}

	/* okay: grantcap can be granted, do it! */
	if (grantcap)
		set_ambient_cap(grantcap);

	if (verbose && (reqcaps & ~okcaps)) {
			printf("Granted ambient capabilities:\n");
			printcapset(grantcap, "  ");
	}

	/* exec the command in the new ambient capability environment */
	execvp(copy_path[0] == 0 ? cmdargv[0] : copy_path, cmdargv);
	exit(2);
}
