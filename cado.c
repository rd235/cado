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
#include <pam_check.h>
#include <get_user_groups.h>
#include <capset_from_namelist.h>
#include <read_conf.h>
#include <set_ambient_cap.h>

static void printcapset(uint64_t capset, char *indent) {
	cap_value_t cap;
	int count=0;
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (capset & (1ULL << cap)) {
			count ++;
			printf("%s%2d %016llx %s\n",indent,cap,1ULL<<cap,cap_to_name(cap));
		}
	}
	if (count > 1)
		printf("%s   %016llx\n",indent,capset);
}

#define OPTSTRING "hqvs"
struct option long_options[]={
	{"help", no_argument, NULL, 'h'},
	{"quiet", no_argument, NULL, 'q'},
	{"verbose", no_argument, NULL, 'v'},
	{"setcap", no_argument, NULL, 'v'},
};

void usage(char *progname) {
	fprintf(stderr,"%s - execute a command in a different capability ambient\n\n",progname);
	fprintf(stderr,"usage: %s OPTIONS capability_list command [args]\n\n",progname);
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  -h, --help         display help message and exit\n");
	fprintf(stderr,"  -q, --quiet        do not display warnings, do what it is allowed\n");
	fprintf(stderr,"  -v, --verbose      generate extra output\n");
	fprintf(stderr,"  -s, --setcap       set the minimun caps for %s (root access)\n",progname);
	exit(1);
}

int main(int argc, char*argv[])
{
	char *progname=basename(argv[0]);
	char **user_groups=get_user_groups();
	uint64_t okcaps=get_authorized_caps(user_groups);
	uint64_t reqcaps=0;
	uint64_t grantcap=0;
	int verbose=0;
	int quiet=0;
	int setcap=0;

	while (1) {
		int c=getopt_long(argc, argv, OPTSTRING, long_options, NULL);
		if (c < 0) 
			break;
		switch (c) {
			case 'h': usage(progname);
								break;
			case 'v': verbose=1;
								break;
			case 'q': quiet=1;
								break;
			case 's': setcap=1;
								break;
		}
	}

	if (setcap) {
		if (geteuid() != 0) {
			fprintf(stderr, "setcap requires root access\n");
			exit(2);
		}
		okcaps = get_authorized_caps(NULL);
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
		
	if (verbose) {
		printf("Allowed ambient capabilities:\n");
		printcapset(okcaps, "  ");
	}

	if (verbose && (argc == optind))
		exit(0);

	if (argc - optind < 2)
		usage(progname);

	if (capset_from_namelist(argv[optind], &reqcaps)) 
		exit(2);

	if (verbose) {
		printf("Requested ambient capabilities:\n");
		printcapset(reqcaps, "  ");
	}

	if (reqcaps & ~okcaps) {
		if (verbose) {
			printf("Unavailable ambient capabilities:\n");
			printcapset(reqcaps & ~okcaps, "  ");
		}
		if (!quiet) {
			fprintf(stderr,"%s: Permission denied\n",progname);
			exit(2);
		}
	}

	grantcap = reqcaps & okcaps;

	optind++;

	if (pam_check(user_groups[0]) != 0) {
		fprintf(stderr,"%s: Authentication failure\n",progname);
		exit(2);
	}
	set_ambient_cap(grantcap);
	if (verbose && (reqcaps & ~okcaps)) {
			printf("Granted ambient capabilities:\n");
			printcapset(grantcap, "  ");
	}
	execvp(argv[optind],argv+optind);
	exit(2);
}
