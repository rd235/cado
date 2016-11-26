bin_PROGRAMS = cado cadrop scado caprint

cado_SOURCES = cado.c pam_check.c get_user_groups.c capset_from_namelist.c read_conf.c set_ambient_cap.c \
							 compute_digest.c file_utils.c scado_parse.c cado_scado_check.c

cado_LDADD = -lpam -lpam_misc -lcap -lmhash

cadrop_SOURCES = cadrop.c capset_from_namelist.c set_ambient_cap.c

cadrop_LDADD = -lcap

caprint_LDADD = -lcap

scado_SOURCES = scado.c pam_check.c file_utils.c compute_digest.c capset_from_namelist.c scado_parse.c

scado_LDADD = -lpam -lpam_misc -lcap -lmhash

common_nodist = cado_paths.h
BUILT_SOURCES = $(common_nodist)

man_MANS = cado.1 cadrop.1 caprint.1 scado.1 cado.conf.5

install-exec-hook:
	(useradd -r -s /bin/nologin -g `getent passwd | grep cado | cut -f 3 -d ':'` cado ||\
	 useradd -r -s /bin/nologin -U cado) || true
	(mkdir -p ${SPOOL_DIR} ; chown root:cado ${SPOOL_DIR} && chmod 4770 $(SPOOL_DIR)) 
	chown :cado $(DESTDIR)$(bindir)/scado
	chmod g+s $(DESTDIR)$(bindir)/scado
	chown cado $(DESTDIR)$(bindir)/cado
	chmod u+s $(DESTDIR)$(bindir)/cado
	$(DESTDIR)$(bindir)/cado -s

CLEANFILES = cado_paths.h
cado_paths.h: Makefile
	@echo 'creating $@'
	@sed >$@ 's/ *\\$$//' <<\END #\
	/* This file has been automatically generated.  Do not edit.  */ \
	#ifndef _CADO_PATHS_H \
	#define _CADO_PATHS_H \
	\
	/* Spool directory path */ \
	#define SPOOL_DIR "$(SPOOL_DIR)" \
	\
	/* Cado temporary exe directory path */ \
	#define CADO_EXE_DIR  "$(CADO_EXE_DIR)" \
	\
	#endif /* _SCADO_PATHS_H */\
	END

