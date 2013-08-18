#!/bin/sh
#
#                   OMEGA SECURITY SERVICES
#                     CONJOB PROGRAM CHCK
#                       (C) 2008-2010 Omega
#
#          Losely based on Anope's Services PID check
# 
#             $Id: secron.sh 1630 2010-01-01 08:50:44Z jer $



###############################################################
#                     CONFIGURATION
###############################################################

# Path to executables
OMMPATH=$HOME/omega

OMPATH=$OMMPATH/bin

# Name of the pid file
OMPIDF=$OMMPATH/omega.pid

# Name of the executable
OMPROG=security

OMARGS=""
#############################################################

# Do not edit below this line




OMPID=
echo "Checking $OMPIDF for pid"

cd $OMMPATH

if [ -f $OMPIDF ] ; then
	OMPID=`cat $OMPIDF`

	if `kill -CHLD $OMPID >/dev/null 2>&1`; then
		exit 0
	fi
	
	rm -rf $OMPIDF
fi

cd $OMPATH

./$OMPROG $OMARGS
