#ifndef READ_CONF_H
#define READ_CONF_H
#include <stdint.h>

uint64_t get_authorized_caps(char **user_groups, uint64_t reqset);

int set_self_capability(uint64_t capset); 

#endif
