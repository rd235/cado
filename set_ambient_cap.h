#ifndef SET_AMBIENT_CAP_H
#define SET_AMBIENT_CAP_H
#include <stdint.h>

void set_ambient_cap(uint64_t capset);

void drop_ambient_cap(uint64_t capset);

void raise_cap_dac_read_search(void);

void lower_cap_dac_read_search(void);

#endif
