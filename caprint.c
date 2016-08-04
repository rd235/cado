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
#include <stdint.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/capability.h>
#include <inttypes.h>

char *tag="CapAmb:\t";
uint64_t get_capamb(pid_t pid) {
	FILE *f;
	char filename[32];
	int status=0;
	int target=strlen(tag);
	uint64_t capamb=0;
	int c;
	snprintf(filename,32,"/proc/%d/status",pid);

	f=fopen(filename,"r");
	if (f==NULL)
		exit(2);

	while ((c = getc(f)) != EOF) {
		if (c == tag[status]) {
			status++;
			if (status == target) {
				int fields = 0;
				if ((fields = fscanf(f,"%" PRIx64 "",&capamb)) != 1)
					fprintf(stderr, "WARNING: fscanf on %s return %d fields.\n", filename, fields);
				break;
			}
		} else
			status=0;
	}
	fclose(f);
	return capamb;
}

#define OPTSTRING "hpcl"
struct option long_options[]={
	{"help", no_argument, NULL, 'h'},
	{"prompt", no_argument, NULL, 'p'},
	{"compact", no_argument, NULL, 'c'},
	{"long", no_argument, NULL, 'l'},
};

void usage(char *progname) {
	fprintf(stderr,"%s - show the current capability ambient\n\n",progname);
	fprintf(stderr,"usage: %s OPTIONS [pid]\n\n",progname);
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  -h, --help         display help message and exit\n");
	fprintf(stderr,"  -p, --prompt       single line output for the command prompt\n");
	fprintf(stderr,"  -c, --compact      single line without cap_ prefix\n");
	fprintf(stderr,"  -l, --long         display cap# and mask\n");
	exit(1);
}


int main(int argc, char *argv[]) {
	uint64_t capamb;
	char *progname=basename(argv[0]);
	int prompt=0;
	int compact=0;
	int longlist=0;
	while (1) {
		int c=getopt_long(argc, argv, OPTSTRING, long_options, NULL);
		if (c < 0)
			break;
		switch (c) {
			case 'h': usage(progname);
								break;
			case 'p': prompt=compact=1;
								break;
			case 'c': compact=1;
								break;
			case 'l': longlist=1;
								break;
			default:
								usage(progname);
		}
	}

	if (argc - optind > 1)
		usage(progname);

	if (optind < argc)
		capamb=get_capamb(atoi(argv[optind]));
	else
		capamb=get_capamb(getpid());
	if (capamb) {
		cap_value_t cap;
		char *sep="";
		int count=0;
		for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
			if (capamb & (1ULL << cap)) {
				count++;
				if (longlist)
					printf("%2d %016llx %s\n",cap,1ULL<<cap,cap_to_name(cap));
				else if (compact)
					printf("%s%s",sep,cap_to_name(cap)+4);
				else
					printf("%s\n",cap_to_name(cap));
				sep=",";
			}
		}
		if (prompt) printf("#");
		if (compact) printf("\n");
		if (longlist && count > 1)
			printf("   %016" PRIx64 "\n",capamb);
	}

	return 0;
}
