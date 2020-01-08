/*
 * cado: execute a command in a capability ambient
 * Copyright (C) 2016  Renzo Davoli, University of Bologna
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
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <set_ambient_cap.h>

/* just in case prctl.h is not providing these definitions */
#ifndef PR_CAP_AMBIENT
#define PR_CAP_AMBIENT			47
#endif
#ifndef PR_CAP_AMBIENT_RAISE
#define PR_CAP_AMBIENT_RAISE	2
#endif
#ifndef PR_CAP_AMBIENT_LOWER
#define PR_CAP_AMBIENT_LOWER	3
#endif
#ifndef PR_CAP_AMBIENT_CLEAR_ALL
#define PR_CAP_AMBIENT_CLEAR_ALL    4
#endif

/* set the ambient capabilities to match the bitmap capset.
	 the capability #k is active if and only if the (k+1)-th least significative bit in capset is 1.
	 (i.e. if and only if (capset & (1ULL << k)) is not zero. */
void set_ambient_cap(uint64_t capset)
{
	cap_value_t cap;
	cap_t caps=cap_get_proc();
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		int op = (capset & (1ULL << cap)) ? CAP_SET : CAP_CLEAR;
		if (cap_set_flag(caps, CAP_INHERITABLE, 1, &cap, op)) {
			fprintf(stderr, "Cannot %s inheritable cap %s\n",op==CAP_SET?"set":"clear",cap_to_name(cap));
			exit(2);
		}
	}
	cap_set_proc(caps);
	cap_free(caps);

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		int op = (capset & (1ULL << cap)) ? PR_CAP_AMBIENT_RAISE : PR_CAP_AMBIENT_LOWER;
		if (prctl(PR_CAP_AMBIENT, op, cap, 0, 0)) {
			perror("Cannot set cap");
			exit(1);
		}
	}
}

/* drop the capabilities of the capset */

void drop_ambient_cap(uint64_t capset) {
	cap_value_t cap;
	cap_t caps=cap_get_proc();
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (capset & (1ULL << cap)) {
			if (cap_set_flag(caps, CAP_INHERITABLE, 1, &cap, CAP_CLEAR)) {
				fprintf(stderr, "Cannot clear inheritable cap %s\n",cap_to_name(cap));
				exit(2);
			}
		}
	}
	cap_set_proc(caps);
	cap_free(caps);

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if ((capset & (1ULL << cap))) {
			if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, cap, 0, 0)) {
				perror("Cannot set cap");
				exit(1);
			}
		}
	}
}

int drop_all_ambient_cap(void) {
	  return prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0);
}

/* turn cap_dac_read_search on and off to have "extra" powers only when needed */
void raise_cap_dac_read_search(void) {
	cap_value_t cap=CAP_DAC_READ_SEARCH;
	cap_t caps=cap_get_proc();
	cap_set_flag(caps, CAP_EFFECTIVE, 1, &cap, CAP_SET);
	cap_set_proc(caps);
}

void lower_cap_dac_read_search(void) {
	cap_value_t cap=CAP_DAC_READ_SEARCH;
	cap_t caps=cap_get_proc();
	cap_set_flag(caps, CAP_EFFECTIVE, 1, &cap, CAP_CLEAR);
	cap_set_proc(caps);
}

