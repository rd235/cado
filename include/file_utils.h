#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

/* copyfile copies the file infd (from the current offset) to
	 outfd (from the current offset) */
/* the return value is the number of bytes copied */
ssize_t copyfile(int infd, int outfd);

/* copytemp copies the file whose path is 'inpath' in a temporary file
	 whose pathname has been created by mkstemp(3) using 'outtemplate; */
ssize_t copytemp(char *inpath, char *outtemplate);

#endif
