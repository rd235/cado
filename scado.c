/* 
 * scado: setuid in capability sauce.
 * Copyright (C) 2016  Davide Berardi, University of Bologna
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
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <fcntl.h>

#include <pwd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <file_utils.h>
#include <pam_check.h>
#include <get_scado_file.h>

#include <cado_paths.h>
#include <cado_const.h>
#include <scado_parse.h>
#include <compute_digest.h>
#include <config.h>

#include <s2argv.h>

#define EDIT_PAM_MAXTRIES 3

#define SUCCESS 0

/* Generic error */
#define ERROR_EXIT -1

/* Authentication error */
#define ERROR_AUTH -2

const char *tmpdir;

#define OPTSTRING "hu:UDle"
struct option long_options[] = {
	{"help",       no_argument,       NULL, 'h'},
	{"update",     required_argument, NULL, 'u'},
	{"update-all", no_argument,       NULL, 'U'},
	{"delete",     no_argument,       NULL, 'D'},
	{"list",       no_argument,       NULL, 'l'},
	{"edit",       no_argument,       NULL, 'e'},
};

enum scado_option {none, update, delete, list, edit, toomany};

void usage_n_exit(char *progname, char *message) {
	fprintf(stderr, "%s - script cado, setuid in capability sauce\n", progname);
	fprintf(stderr, "usage: %s OPTIONS\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -h --help               display help message and exit\n");
	fprintf(stderr, "   -u --update <command>   update the checksum of command\n");
	fprintf(stderr, "   -U --update-all         update the checksum of all commands\n");
	fprintf(stderr, "   -D --delete             delete the scado file\n");
	fprintf(stderr, "   -l --list               print the scado file\n");
	fprintf(stderr, "   -e --edit               edit the scado file\n");
	if (message)
		fprintf(stderr, "\n%s\n",message);
	exit(ERROR_EXIT);
}

/* watchdog for garbage collection.
	 the temporary file for scado editing is removed in case of abend */
static int editor_garbage_collect(char *path) {
	int checkpipe[2];
	pid_t childpid;
	if (pipe(checkpipe))
		return -1;
	/* Start garbage collector
	 * Double fork to avoid zombie processes and to remove the temporary file when it is not needed any more */
	if(!(childpid = fork())) {
		/* Child */
		if(!fork()) {
			char c = 0;
			/* Grandchild */
			if (close(checkpipe[1]) == 0 && setsid() > 0) 
				read(checkpipe[0], &c, 1);
			if (c == 0)
				unlink(path);
			exit(0);
		} else
			exit(0);
	} 
	waitpid(childpid, NULL, 0);
	/* (grand) parent */
	close(checkpipe[0]);
	return(checkpipe[1]);
}

static void editor_garbage_collect_do_not_unlink(int fd) {
	char c = 'K'; // keep it, any other non-null char would fit.
	write(fd, &c, 1);
}

/* command line selectable functions */

int scado_none_or_toomany(char *progname, char *username, char *program_path) {
	usage_n_exit(progname, "select exactly one option among: -u -U -D -l -e");
	return 0;
}

int scado_delete(char *progname, char *username, char *program_path) {
	char scado_file[PATH_MAX];

	if (get_scado_file(username, scado_file) < 0)
		return ERROR_EXIT;

	/* Get the authorization. */
	if (pam_check(username) != PAM_SUCCESS) {
		return ERROR_AUTH;
	}

	return unlink(scado_file);
}

int scado_update(char *progname, char *username, char *program_path) {
	char tmp_file[PATH_MAX];
	char scado_file[PATH_MAX];

	if (get_scado_file(username, scado_file) < 0)
		return ERROR_EXIT;

	/* Get the authorization. */
	if (pam_check(username) != PAM_SUCCESS) {
		return ERROR_AUTH;
	}

	if (snprintf(tmp_file, PATH_MAX, "%s/%s", tmpdir, TMP_TEMPLATE) < 0) 
		return ERROR_EXIT;

	if (copytemp(scado_file, tmp_file) < 0)
		return ERROR_EXIT;

	scado_copy_update(tmp_file, scado_file, program_path);

	if (unlink(tmp_file) < 0)
		return ERROR_EXIT;

	return SUCCESS;
}

int scado_list(char *progname, char *username, char *program_path) {
	char scado_file[PATH_MAX];
	int fd;
	int outfl = fcntl(STDOUT_FILENO, F_GETFL, 0);
	if (outfl & O_APPEND)
		fcntl(STDOUT_FILENO, F_SETFL, outfl & ~O_APPEND);

	if (get_scado_file(username, scado_file) < 0)
		return ERROR_EXIT;

	if ((fd = open(scado_file, O_RDONLY)) < 0)
		return fd;

	copyfile(fd, STDOUT_FILENO);

	close(fd);
	return SUCCESS;
}

int scado_edit(char *progname, char *username, char *program_path) {
	char tmp_file[PATH_MAX];
	char scado_file[PATH_MAX];
	char *editor;
	char *args = NULL;
	int status = 0;
	pid_t pid, xpid;
	char digest_before[DIGESTSTRLEN + 1];
	char digest_after[DIGESTSTRLEN + 1];
	int garbage_collect_fd;

	/* Ignore signals. */
	(void) signal(SIGHUP,  SIG_IGN);
	(void) signal(SIGINT,  SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);

	if (get_scado_file(username, scado_file) < 0)
		return ERROR_EXIT;

	if (snprintf(tmp_file, PATH_MAX, "%s/%s", tmpdir, TMP_TEMPLATE) < 0) 
		return ERROR_EXIT;

	if (copytemp_digest(scado_file, tmp_file, digest_before) < 0) {
		*digest_before = 0;
		if (errno == ENOENT) {
			int tmpfd=mkstemp(tmp_file);
			if (tmpfd < 0)
				return ERROR_EXIT;
			if (write(tmpfd, DEFAULT_MESSAGE, sizeof(DEFAULT_MESSAGE)-1) < 0)
				return ERROR_EXIT;
			close(tmpfd);
		} else
			return ERROR_EXIT;
	}

	garbage_collect_fd = editor_garbage_collect(tmp_file);

	/* Get the editor from the configuration. */
	if (((editor = getenv("VISUAL")) == NULL || *editor == '\0') &&
			((editor = getenv("EDITOR")) == NULL || *editor == '\0')) {
		editor = EDITOR;
	}

	switch (pid = fork()) {
		case -1:
			perror("fork");
			exit(ERROR_EXIT);
		case 0:
			/* XXX secure? */
			if (setgid(getgid()) < 0) {
				perror("setgid(getgid())");
				exit(ERROR_EXIT);
			}
			if (setuid(getuid()) < 0) {
				perror("setuid(getuid())");
				exit(ERROR_EXIT);
			}

			asprintf(&args, "%s %s", editor, tmp_file);

			if (args == NULL) {
				exit(ERROR_EXIT);
			}

			execsp(args);
			perror(editor);
			exit(ERROR_EXIT);
		default:
			/* parent */
			break;
	}
	/* parent */
	for (;;) {
		xpid = waitpid(pid, &status, 0);
		if (xpid == -1) {
			if(errno != EINTR) {
				fprintf(stderr, "wait pid: error waiting for PID %ld (%s): %s \n",
						(long) xpid, editor, strerror(errno));
				return ERROR_EXIT;
			}
		}
		else if (WIFEXITED(status) && WEXITSTATUS(status)) {
			fprintf(stderr,"wait pid: exit status");
			return ERROR_EXIT;
		}
		else if(WIFSIGNALED(status)) {
			fprintf(stderr, "%s killed: signal %d (%score dumped)\n",
					editor, WTERMSIG(status), WCOREDUMP(status) ? "": "no ");
		}
		else {
			break;
		}
	}

	/* Restore signals. */
	(void) signal(SIGHUP,  SIG_DFL);
	(void) signal(SIGINT,  SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);

	if(compute_digest(tmp_file, digest_after) < 0)
	 return ERROR_EXIT;

	if (strcmp(digest_before, digest_after) != 0) {
		int count;

		/* Get the authorization. */
		for (count = 0; count < EDIT_PAM_MAXTRIES; count++) {
			if (pam_check(username) == PAM_SUCCESS) {
				break;
			}
			if (count < EDIT_PAM_MAXTRIES - 1)
				fprintf(stderr, "PAM authorization failed\n");
			else {
				editor_garbage_collect_do_not_unlink(garbage_collect_fd);
				fprintf(stderr, "A copy of the edited scado file has been saved as %s\n", tmp_file);
				return ERROR_AUTH;
			}
		}
		scado_copy_update(tmp_file, scado_file, NULL);
	}

	close(garbage_collect_fd);
	return SUCCESS;
}

typedef int (* option_function) (char *progname, char *username, char *program_path);

/* array to select the requested option */
option_function option_map[] = {
	scado_none_or_toomany, 
	scado_update, 
	scado_delete, 
	scado_list, 
	scado_edit, 
	scado_none_or_toomany};

/* update the chosen option. If one was already chosen, switch to "toomany" */
static inline enum scado_option set_option(enum scado_option option, enum scado_option newvalue) {
	return option == none ? newvalue : toomany;
}

int main(int argc, char **argv)
{
	struct passwd *pw;
	char *progname = basename(argv[0]);
	char *username;
	char *program_path = ""; /* "" means ALL */
	enum scado_option option=none;

	int rval = 1;

	if ((pw = getpwuid(getuid())) ==  NULL) {
		fprintf(stderr, "Your UID isn't in the passwd file.\n");
		exit(ERROR_EXIT);
	}
	username = pw->pw_name;

	if ((tmpdir = getenv("TMPDIR")) == NULL || *tmpdir== '\0') {
		tmpdir = TMPDIR;
	}

	while(1) {
		int c = getopt_long(argc, argv, OPTSTRING, long_options, NULL);
		if (c < 0)
			break;
		switch(c) {
			case 'h':
				usage_n_exit(progname, NULL);
				break;
			case 'u':
				program_path = optarg;
			case 'U':
				option=set_option(option,update);
				break;
			case 'D':
				option=set_option(option,delete);
				break;
			case 'l':
				option=set_option(option,list);
				break;
			case 'e':
				option=set_option(option,edit);
				break;
			default:
				usage_n_exit(progname, NULL);
		}
	}
	if (optind != argc) /* unknown trailing arguments */
		usage_n_exit(progname, "unknown trailing arguments");

	rval = option_map[option](progname, username, program_path);

	if (rval == ERROR_AUTH) {
		fprintf(stderr, "%s: Authorization failure.\n", progname);
	} else if (rval) {
		fprintf(stderr, "%s returned: %d, %s\n", progname, rval, strerror(errno));
	}

	return rval;
}
