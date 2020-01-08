#ifndef _SCADO_PARSE_H
#define _SCADO_PARSE_H

#include <stdint.h>

/* copy file inpath to file outpath.
	 if path == NULL, add missing HASH digests
	 else if *path == 0, update all HASH digests
	 else update only the digest for path */
void scado_copy_update(char *inpath, char *outpath, char *path);

/* get info for file path.
	 if scado_path_getinfo returns 1 the path is authorized by scado, 
	 pcapset and digest are the permitted set of capabilities and the digest respectively */
int scado_path_getinfo(char *inpath, const char *path, uint64_t *pcapset, char *digest);

#endif // _SCADO_PARSE.H
