execute_process(COMMAND bash "-c"
    "(useradd -r -s /bin/nologin -g `getent passwd | grep cado | cut -f 3 -d ':'` cado || useradd -r -s /bin/nologin -U cado) || true;\
    mkdir -p /usr/local/var/spool/cado;\
    chown root:cado /usr/local/var/spool/cado && chmod 4770 /usr/local/var/spool/cado;\
    chown :cado ${BINDIR}/scado;\
    chmod g+s ${BINDIR}/scado;\
    chown cado ${BINDIR}/cado;\
    chmod u+s ${BINDIR}/cado;\
    ldconfig ${LIBDIR};\
    ${BINDIR}/cado -s"
)
