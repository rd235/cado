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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#undef __linux__
#ifdef __linux__
#include <sys/sendfile.h>
#define MAXSENDFILE 0x7ffff000
#else
#define BUFSIZE 1024
#endif

/* copy a file: when possible use a fast system call */
ssize_t copyfile(int infd, int outfd) {
	ssize_t rv=0;
	ssize_t n;
#ifdef __linux__
	do
		rv += n = sendfile(outfd, infd, NULL, MAXSENDFILE);
	while (n == MAXSENDFILE);
#else
	static int pagesize;
	if (__builtin_expect((pagesize == 0),0)) pagesize=sysconf(_SC_PAGESIZE);
	char buf[BUFSIZE];
	while ((n = read(infd, buf, BUFSIZE)) > 0) 
		rv += write(outfd, buf, n);
#endif
	return (n < 0) ? n : rv;
}

/* create a temporary copy of a file (using copyfile) */
/* outtemplate is a template for temporary files as explained in mkstemp(3) */
ssize_t copytemp(char *inpath, char *outtemplate) {
	int infd=open(inpath, O_RDONLY);
	int outfd;
	mode_t oldmask;
	ssize_t rv;
	if (infd < 0)
		return -1;
	oldmask = umask(077);
	outfd = mkstemp(outtemplate);
	umask(oldmask);
	if (outfd < 0) {
		close(infd);
		return -1;
	}
	rv = copyfile(infd, outfd);
	close(infd);
	close(outfd);
	return rv;
}
