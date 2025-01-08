/* 
 * cado: execute a command in a capability ambient
 * Copyright (C) 2016  Renzo Davoli and Davide Berardi, University of Bologna
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
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <capset_from_namelist.h>
#include <compute_digest.h>

/* this structure stores the output of the "parsing" of a scado line.
	 a line has (at most) three colon ":" separated fields.
	 all field and end pointers are addresses within the string "line".
	 the i-th field begins at field[i] and terminates just before end[i] */
	 
struct scado_line {
	char *line;
	char *field[3];
	char *end[3];
};

/* parse a scado line */
static struct scado_line scado_parse(char *line) {
	struct scado_line out={line, {NULL, NULL, NULL}, {NULL, NULL, NULL}};
	char *scan=line;
	int i;
	for (i=0; i<3; i++) {
		while (isspace(*scan)) scan++;
		out.field[i] = scan;
		if (*scan == '#' || *scan == 0) goto end;
		while (*scan != ':') {
			if (scan[0] == '\\' && scan[1] != 0) scan++;
			if (*scan == '#' || *scan == 0) goto end;
			scan++;
		}
		out.end[i] = scan;
		scan++;
	}
	return out;
end:
	out.end[i] = scan;
	return out;
}

/* true if the current line matches path, false otherwise.
	 scado_path_match manages the escape chars '\' in the scado line
	 without any copy */
static int scado_path_match(const char *path, struct scado_line *line) {
	char *lpath;
	for (lpath = line->field[0]; *path && *lpath; path++, lpath++) {
		if (lpath[0] == '\\' && lpath[1] != 0) lpath++;
		if (*lpath != *path) 
			break;
	}
	return (*path == 0 && (*lpath == 0 || isspace(*lpath) || *lpath == ':'));
}

/* get the capability set from a scado file line */
static int scado_get_caps(struct scado_line *line, uint64_t *capset) {
	if (line->field[1]) {
		size_t caplen = line->end[1] - line->field[1];
		char caps[caplen+1];
		strncpy(caps,line->field[1],caplen);
		caps[caplen]=0;
		return capset_from_namelist(caps, capset);
	} else
		return -1;
}

/* scado_check_digest_syntax returns:
	 -1 if the digest field has syntax errors,
	 0 if it is an ampty field (but the field exists!)
	 1 if the line has just two fields (one ':') OR there is a well-formed hex digest */

static int scado_check_digest_syntax(struct scado_line *line) {
	if (line->field[2]) {
		int i;
		size_t digestlen = line->end[2] - line->field[2];
		char digest[digestlen+1];
		strncpy(digest,line->field[2],digestlen);
		digest[digestlen]=0;
		if (*digest == 0)
			return 0;
		if (strlen(digest) < DIGESTSTRLEN)
			return -1;
		for (i = 0; i < DIGESTSTRLEN && isxdigit(digest[i]); i++)
			;
		if (i < DIGESTSTRLEN)
			return -1;
		for (; digest[i] != 0; i++)
			if (!isspace(digest[i]))
				return -1;
		return 1;
	} else
		return 1;
}

/* true if the field exists and it is not empty */
static inline int fieldok(struct scado_line *line, int i) {
	return line->field[i] && line->end[i] != line->field[i];
}

/* some syntax sanity checks */
static int scado_syntax_check(char *path, int lineno, struct scado_line *line) {
	uint64_t capset;
	int rv=0;
	if (line->end[2] && line->end[2][0] == ':') {
		fprintf(stderr,"%s line %d: extra trailing chars\n", path, lineno);
		rv = -1;
	}
	if (fieldok(line, 0) && line->field[0][0] != '/') {
		fprintf(stderr,"%s line %d: pathname is not absolute\n", path, lineno);
		rv = -1;
	}
	if (fieldok(line, 0) && !fieldok(line, 1)) {
		fprintf(stderr,"%s line %d: missing capability set\n", path, lineno);
		rv = -1;
	}
	if (fieldok(line, 1) && scado_get_caps(line, &capset) < 0) {
		fprintf(stderr,"%s line %d: wrong capability syntax\n", path, lineno);
		rv = -1;
	}
	if (fieldok(line, 2) && scado_check_digest_syntax(line) < 0) {
		fprintf(stderr,"%s line %d: wrong digest syntax\n", path, lineno);
		rv = -1;
	}
	return rv;
}
	
/* clean the path (parsing escape chars) in place */
static void cleanpath(char *path) {
	char *out=path;
	while(*path != 0 && !isspace(*path)) {
		if (path[0] == '\\' && path[1] != 0) path++;
		*out++ = *path++;
	}
	*out=0;
}
	
/* update a line */
/* inpath == NULL -> add the digest it if is missing
	 inpath == "" -> update the digest in any case
	 inpath == "/....." (a valid path) -> update the digest if the path of this line matches */
static void scado_update_line(FILE *fout, struct scado_line *line, char *inpath) {
	if (line->field[2] != NULL) {
		/* there is the digest field */
		size_t pathlen = line->end[0] - line->field[0];
		char path[pathlen+1];
		/* create a temporary copy of the path on the stack and clean it*/
		strncpy(path,line->field[0],pathlen);
		path[pathlen]=0;
		cleanpath(path);
		int digestok=scado_check_digest_syntax(line);
		if (digestok < 0) // do not update in case of syntax error
			fprintf(fout, "%s\n", line->line);
		else if (digestok == 0) { //missing digest -> fill in!
			char digest[DIGESTSTRLEN + 1];
			int len = line->field[2] - line->line;
			if (compute_digest(path, digest) < 0)
				fprintf(fout, "%s\n", line->line);
			else if (line->end[2] != 0)
				fprintf(fout,"%*.*s%s %s\n",len,len,line->line,digest,line->end[2]);
			else 
				fprintf(fout,"%*.*s%s\n",len,len,line->line,digest);
		} else if (inpath && (*inpath == 0 || strcmp(path, inpath) == 0)) { /* if ALL or this path */
			char digest[DIGESTSTRLEN + 1];
			int len = line->field[2] - line->line;
			if (compute_digest(path, digest) < 0)
				fprintf(fout, "%s\n", line->line);
			else 
			  fprintf(fout,"%*.*s%s%s\n",len,len,line->line,digest,line->field[2]+DIGESTSTRLEN);
		} else
			fprintf(fout, "%s\n", line->line);
	} else
		fprintf(fout, "%s\n", line->line);
}

/* copy the file from inpath to outpath, updating the digest where required:
	 path == NULL -> add missing digests
	 path == "" -> all
*/
void scado_copy_update(char *inpath, char *outpath, char *path) {
	FILE *fin = NULL;
	mode_t oldmask;
	FILE *fout;
	char *line;
	size_t len=0;
	int lineno=0;
	ssize_t n;

	int fdin;
	struct stat l_stats;

	/* Symlinks check */
	if ((fdin = open(inpath, O_RDONLY | O_NOFOLLOW)) < 0) {
		return;
	}

	if (fstat(fdin, &l_stats) || l_stats.st_mode == S_IFLNK) {
		return;
	}

	fin = fdopen(fdin, "r");
	if (fin == NULL)
		return;
	oldmask = umask(0377);
	unlink(outpath);
	fout = fopen(outpath, "w");
	umask(oldmask);
	if (fout == NULL) {
		fclose(fin);
		return;
	}
	while ((n = getline(&line, &len, fin)) > 0) {
		struct scado_line scado_line;
		line[n-1] = 0;
		scado_line = scado_parse(line);
		lineno++;
		scado_syntax_check(outpath, lineno, &scado_line);
		scado_update_line(fout, &scado_line, path);
	}
	fclose(fin);
	fclose(fout);
	free(line);
}

/* scan the scado file whose pathname is inpath seeking for the line matching with 'path'
	 scado_path_getinfo returns:
	 -1 in case of error, 0 if the line is missing, 1 if the line has been found */
/* if scado_path_getinfo succeeds, pcapset and digest are set to
	 the set of permitted capabilities and the hash digest, respectively. */
int scado_path_getinfo(char *inpath, const char *path, uint64_t *pcapset, char *digest) {
	FILE *fin = fopen(inpath, "r");
	char *line;
	size_t len=0;
	ssize_t n;
	int lineno=0;
	int rv = 0;
	if (fin == NULL)
		return -1;
	int fdin = fileno(fin);
	struct stat st;
	if (fstat(fdin, &st) < 0)
		return -1;
	if (st.st_uid != getuid())
		return errno = EPERM, -1;
	while ((n = getline(&line, &len, fin)) > 0) {
		struct scado_line scado_line;
		line[n-1] = 0;
		scado_line = scado_parse(line);
		lineno++;
		if (scado_path_match(path, &scado_line)) {
			if (scado_syntax_check(inpath, lineno, &scado_line) == 0) {
				if (scado_get_caps(&scado_line, pcapset) >= 0) {
					if (scado_line.field[2]) {
						int i;
						for (i = 0; i < DIGESTSTRLEN ; i++) 
							digest[i] = tolower(scado_line.field[2][i]);
						digest[i] = 0;
						rv = 1;
					} else {
						digest[0] = 0;
						rv = 1;
					}	
				}
			}
			break;
		}
	}
	fclose(fin);
	free(line);
	return rv;
}

#if 0
int main() {
	char *line;
	size_t len=0;
	struct scado_line s;
	int lineno=0;
	while (1) {
		ssize_t n=getline(&line, &len, stdin);
		line[n-1]=0;
		lineno++;
		s=scado_parse(line);
		printf("L %s\n",s.line);
		printf("P %s|%s\n",s.field[0],s.end[0]);
		printf("C %s|%s\n",s.field[1],s.end[1]);
		printf("H %s|%s\n",s.field[2],s.end[2]);
		uint64_t cap=0;
		printf("%d ",scado_get_caps(&s,&cap));
		printf("%lx\n",cap);
		scado_syntax_check(lineno, &s);
		printf("NULL->");fflush(stdout);
		scado_update_line(1, &s, NULL);
		printf("ALL ->");fflush(stdout);
		scado_update_line(1, &s, "");
		printf("bash ->");fflush(stdout);
		scado_update_line(1, &s, "/bin/bash");
	}
}
#endif
#if 0
int main(int argc, char *argv[]) {
	scado_update(argv[1], argv[2], argv[3]);
}
#endif
#if 0
int main(int argc, char *argv[]) {
	char digest[DIGESTSTRLEN + 1];
	uint64_t capset;
	if (scado_path_getinfo(argv[1], argv[2], &capset, digest)) {
		printf("%lx %s\n",capset,digest);
	}
}
#endif
