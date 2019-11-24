#ifndef _CADO_CONST_H
#define _CADO_CONST_H

#include <cado_paths.h>

/* Default message to print in a new scado file. */
#define DEFAULT_MESSAGE \
	"# This is a scado file.\n"\
	"# format: executable : capabilities_list [:]\n"\
	"# If you specify :,\n"\
	"# scado will automatically put the checksum of the file at the end of the line\n"\
	"# (for subsequent checks).\n"

/* Tmp file template */
#define TMP_TEMPLATE "cado-scado.XXXXXX"

/* Tmp Directory */
#define TMPDIR "/tmp"

#define BUFSIZE 4096

#define DIGEST_WARNING "***********************************************************\n"\
                       "*         WARNING: EXECUTABLE DIGEST HAS CHANGED!         *\n"\
                       "***********************************************************\n"\
                       "IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!\n"\
                       "Someone could be changed your executable, and it's trying to execute a malware!\n"\
                       "It is also possible that the executable is just changed (maybe a system upgrade?).\n"

#define COPY_DIR CADO_EXE_DIR

#define COPY_TEMPLATE "cado-exe-copy.XXXXXX"

#define COMMENT_LINE            0
#define NO_CHECKSUM_LINE        1
#define CALCULATE_CHECKSUM_LINE 2
#define CHECKSUM_LINE           3
#define NOT_CONSIDERED_LINE     4

#endif /* _CADO_CONST_H */
