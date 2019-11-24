/* 
 * scado: setuid in capability sauce.
 * Copyright (C) 2016 Davide Berardi and Renzo Davoli, University of Bologna
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
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <compute_digest.h>
#define BUFSIZE 1024

/* compute the hash digest for file whose descriptor is infd.
	 if outfd >= 0 copy the contents to the file whose descriptor is outfd.
	 the return value is the size of the file (or a negative value in case of error).
	 the size of ascii_digest must be at least DIGESTSTRLEN + 1*/
static ssize_t fcompute_digest(int infd, int outfd, char *ascii_digest) {
	char buf[BUFSIZE];
	unsigned char binary_digest[DIGESTLEN];
	MHASH td;

	ssize_t n;
	ssize_t rv = 0;
	td = mhash_init(DIGESTTYPE);

	while ((n=read(infd,buf,BUFSIZE)) > 0) {
		mhash(td, buf, n);
		if (outfd >= 0) write(outfd, buf, n);
		rv += n;
	}

	mhash_deinit(td, binary_digest);
	if (n >= 0) {
		int i;
		for (i = 0; i < DIGESTLEN; i++, ascii_digest += 2) 
			sprintf(ascii_digest, "%.2x", binary_digest[i]);
		*ascii_digest = 0;
	}
	return (n < 0) ? n : rv;
}

/* compute the hash digest of the file (the first arg is the pathname) */
ssize_t compute_digest(const char *path, char *digest) {
	int fd=open(path, O_RDONLY);
	if (fd >= 0) {
		ssize_t rv = fcompute_digest(fd, -1, digest);
		close(fd);
		return rv;
	} else
		return -1;
}

/* compute the hash digest of the file (the first arg is the pathname)
	 while copying it in a temporary file. The second arg is a template for tmp files
	 as explained in mkstemp(3)
 */
ssize_t copytemp_digest(const char *inpath, char *outtemplate, char *digest) {
	int infd=open(inpath, O_RDONLY);
	int outfd;
	mode_t oldmask;
	ssize_t rv;
	if (infd < 0)
		return -1;
	oldmask = umask(027);
	outfd = mkstemp(outtemplate);
	umask(oldmask);
	if (outfd < 0)
		return -1;
	rv = fcompute_digest(infd, outfd, digest);
	close(infd);
	close(outfd);
	return rv;
}

