#!/bin/bash
set -x

if [ "x$1" == "xnewtag" ]
then
	SOURCE_ROOT=${PWD##*/}
	SOURCE_NAME=$(dpkg-parsechangelog --show-field Source)
  UPSTREAM_VERSION="$(dpkg-parsechangelog --show-field Version | sed 's/-.*$//')"
  (
	cd ..
  tar zcvf ${SOURCE_NAME}_${UPSTREAM_VERSION}.orig.tar.gz \
		--transform "s/^${SOURCE_ROOT}/${SOURCE_NAME}-${UPSTREAM_VERSION}/" \
		--exclude "${SOURCE_ROOT}/debian" \
		--exclude "*/.git" \
		${SOURCE_ROOT}
	)
fi

debuild -uc -us -sa
