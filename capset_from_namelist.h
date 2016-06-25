#ifndef CAP_FROM_NAMEX_H
#define CAP_FROM_NAMEX_H
#include <sys/capability.h>
#include <stdint.h>

int capset_from_namelist(char *namelist, uint64_t *capset);

#endif
