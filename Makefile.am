bin_PROGRAMS = cado caprint

cado_SOURCES = cado.c pam_check.c get_user_groups.c capset_from_namelist.c read_conf.c set_ambient_cap.c

cado_LDADD = -lpam -lpam_misc -lcap

caprint_LDADD = -lcap

man_MANS = cado.1 caprint.1 cado.conf.5
