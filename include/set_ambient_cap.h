#ifndef SET_AMBIENT_CAP_H
#define SET_AMBIENT_CAP_H
#include <stdint.h>

void set_ambient_cap(uint64_t capset);

void drop_ambient_cap(uint64_t capset);

int drop_all_ambient_cap(void);

#endif
