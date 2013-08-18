#!/bin/sh

# updateignores.sh: Update the svn:ignore definitions
# $Id: updateignores.sh 1629 2010-01-01 08:37:01Z jer $

DIRS="docs include modules protocol src contrib"

for i in ${DIRS}; do \
	if [ -f $i/.svnignore ]; then \
		svn propset 'svn:ignore' -F $i/.svnignore $i; \
	fi
done
