execute_process(COMMAND bash -c "\
	if ! getent group _cado >/dev/null 2>&1; then
		groupadd \
			--system \
			_cado;
	fi")
execute_process(COMMAND bash -c "\
	if ! getent passwd _cado >/dev/null 2>&1; then
		useradd \
			--no-create-home \
			--home-dir /nonexistent \
			--system \
			--shell /bin/nologin \
			-g _cado \
			_cado;
	fi")
execute_process(COMMAND mkdir -p /usr/local/var/spool/cado)
execute_process(COMMAND chown root:_cado /usr/local/var/spool/cado)
execute_process(COMMAND chmod 4770 /usr/local/var/spool/cado)
execute_process(COMMAND chown :_cado ${BINDIR}/scado)
execute_process(COMMAND chmod g+s ${BINDIR}/scado)
execute_process(COMMAND chown _cado: ${BINDIR}/cado)
execute_process(COMMAND chmod u+s ${BINDIR}/cado)
execute_process(COMMAND ldconfig ${LIBDIR})
execute_process(COMMAND ${BINDIR}/cado --setcap)
