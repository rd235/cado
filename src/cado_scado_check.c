/* 
 * cado: execute a command in a capability ambient
 * Copyright (C) 2016  Davide Berardi and Renzo Davoli, University of Bologna
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
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cado_const.h>
#include <get_scado_file.h>
#include <set_ambient_cap.h>
#include <scado_parse.h>
#include <compute_digest.h>
#include <cado_scado_check.h>

static void print_digest_warning(void) {
	  fprintf(stderr, "%s", DIGEST_WARNING);
}

/* Avoiding TOCTOU attacks:
	 The executable file is copied in a safe place where the user cannot modify it.
	 (cado is setuid cado).
	 The digest of the copy is compared against the digest reported in the scado configuration file.
	 If the digests are the same, cado runs the copy */
/* A watchdog process unlinks the copy as soon as either cado terminates or has started the command (execve system call) */

/*  copy_and_check_digest returns 0 if it succeeds, i.e.
	 	- a file at path exists and it is readable
		- the hash digest of the file is equal to digest
	and in this case newpath returns the newpath of the tmpfile to run */
/* if scado does not require to check the digest, copy_and_check_digest
	 returns an empty string in newpath, i.e  *newpath == 0 */
/* if copy_and_check_digest fails, it returns -1 */
static int copy_and_check_digest(const char *path, char *digest, char *newpath) {
	size_t newpathlen;
	/* grandchild -> grandparent pipe:
		 write the path of the tmp copy and close
		 in case of error: close (without writing anything)
		 in case of hash mismatch write a NULL byte and close */
	int gc_pipe[2];
	pid_t childpid;

	/* grandparent -> grandchild pipe: used to check when the exec occours. */
	/* this pipe has the FD_CLOEXEC bit set. 
		 it is unvoluntarily closed either when an exec succeeds or when the grandparent terminates
		 (end or abend, it does not matter) */
	/* when the grandchild gets the EOF it unlinks the temporary file */
	int gp_pipe[2];

	if (!path || !digest || !newpath) {
		return -1;
	}

	if (snprintf(newpath, PATH_MAX, "%s/%s", COPY_DIR, COPY_TEMPLATE) < 0) {
		return -1;
	}
	newpathlen = strlen(newpath);

	if (pipe(gc_pipe)) {
		perror("pipe");
		return -1;
	}

	if (pipe(gp_pipe)) {
		perror("pipe");
		return -1;
	}

	if (fcntl(gp_pipe[0], F_SETFD, FD_CLOEXEC) ||
			fcntl(gp_pipe[1], F_SETFD, FD_CLOEXEC)) {
		perror("fdcntl cloexec");
		return -1;
	}

	/* Start garbage collector
	 * Double fork to avoid zombie processes and to remove the temporary file when it is not needed any more */
	if(!(childpid=fork())) {
		/* Child */
		if(!fork()) {
			/* Grandchild */
			char newdigest[DIGESTSTRLEN + 1];
			char c;
			int exit_status = 1;

			/* if the grandparent terminates before reading the newpath, 
				 write returns EPIPE. So even in this situation the garbage gets collected i.e. unlink(newpath)*/
			/* setsid to avoid the propagation of signals to the process group (e.g. ^C) */
			if (close(gc_pipe[0]) || close(gp_pipe[1]) || setsid() < 0 || signal(SIGPIPE, SIG_IGN) == SIG_ERR)
				exit(-1);

			gid_t savegid = getegid();
			if (setegid(getgid()) < 0) {
				perror("copytemp setegid to real gid");
				exit(-1);
			}
			if (copytemp_digest(path, newpath, newdigest) < 0) {
				perror("copytemp_hash");
				exit(-1);
			}
			if (setegid(savegid) < 0) {
				perror("copytemp restore setegid");
				exit(-1);
			}

			//debug only
			//printf("GS %s\n%s\n%s\n", newpath,digest,newdigest);
			if (chmod(newpath, 00610) < 0)
				goto end;

			if (strcmp(digest,newdigest) == 0) {
				if (write(gc_pipe[1], newpath, newpathlen + 1) < 1) 
					goto end;
			} else  {
				char err = 0;
				if (write(gc_pipe[1], &err, 1) < 1) 
					goto end;
			}

			if (close(gc_pipe[1])) 
				goto end;

			/* No data should be written on the pipe from the other end.  This is only
			 * to check when the parent calls exec() (or terminates)*/
			while (read(gp_pipe[0], &c, sizeof(char)) > 0)
				;

			close(gp_pipe[0]);
			exit_status = 0;
end:
			unlink(newpath);
			exit(exit_status);
		} else {
			exit(0);
		}
	}
	waitpid(childpid, NULL, 0);

	/* (grand) Parent */
	if (close(gc_pipe[1]) || close(gp_pipe[0])) {
		perror("close pipe");
		return -1;
	}

	/* it should read a string of the same length of the original newpath which is the 
		 template for the tmp file (as described in mkstemp(3) */
	int n;
	if ((n=read(gc_pipe[0], newpath, newpathlen + 1)) <= 0) {
		return -1;
	}

	if (close(gc_pipe[0])) {
		perror("close");
	}

	if (newpath[0] == 0) {
		print_digest_warning();
		return -1;
	}

	return 0;
}

/* given the username and the command path, cado_scado_check returns the set of
	 permitted capabilities, as defined by the scado(1) command */
uint64_t cado_scado_check(const char *username, const char *exe_path, char *new_path) {
	char scado_file[PATH_MAX];
	char digest[DIGESTSTRLEN + 1];
	int rv;
	uint64_t capset = 0;

	if (!username || !exe_path || !new_path) 
		return 0;

	if (get_scado_file(username, scado_file) <= 0){
		perror("get scado file");
		return 0;
	}

	uid_t saveuid = geteuid();
	if (seteuid(getuid()))
		return 0;
	rv = scado_path_getinfo(scado_file, exe_path, &capset, digest);
	if (seteuid(saveuid))
		return 0;

	/* default value: do not run a copy, directly run the command */
	*new_path = 0;
	if (rv <= 0) {
		/* error: no capabilities can be granted */
		if (rv < 0)
			perror("error opening scado file");
		return 0;
	} else {
		/* if no digest was specified in the scado configuration line for the current command:
			 	the capabilities in capset can be granted.
			otherwise copy the executable file to avoid TOCTOU attacks */
		if (*digest == 0 || copy_and_check_digest(exe_path, digest, new_path) == 0)
			return capset;
		else
			return 0;
	}
}
