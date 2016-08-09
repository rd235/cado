#ifndef _GET_SPOOL_FILE_H
#define _GET_SPOOL_FILE_H
#include <limits.h>
#include <cado_paths.h>

/* Get the user scado file */
static inline int get_scado_file(const char *username, char *path) {
	return snprintf(path, PATH_MAX, "%s/%s", SPOOL_DIR,  username);
}
#endif /* _GET_SPOOL_FILE_H */

