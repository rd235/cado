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

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <security/pam_appl.h> 
#include <security/pam_misc.h> 
#include <pwd.h>
#include <pam_check.h>

/* call PAM to authorize the current user.
	 usually this means to prompt the user for a password,
	 but it can be configured using PAM */

int pam_check(char *username)
{
  pam_handle_t* pamh; 
  struct pam_conv pamc={.conv=&misc_conv, .appdata_ptr=NULL}; 
	int rv;

	pam_start ("cado", username, &pamc, &pamh); 
	rv= pam_authenticate (pamh, 0);
	pam_end (pamh, 0); 

  return rv; 
} 

