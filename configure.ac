dnl configure.ac - autoconf configuration file

AC_PREREQ(2.61)
AC_COPYRIGHT([$Id: configure.ac 2357 2012-02-11 23:49:35Z twitch $])

AC_INIT([Omega],[Rework])
AC_CONFIG_HEADER(include/setup.h)
AC_PREFIX_DEFAULT(`echo -n $HOME/omega`)

AC_GNU_SOURCE
AC_PROG_CC
AC_LANG(C)

sinclude([aclocal.m4])
sinclude([mysql.m4])

OSNAME=`uname -s`
PWD=`pwd`

AC_DEFINE_UNQUOTED(CONFIGURE_ARGS, "$ac_configure_args", [Arguments passed to configure])

dnl This is an evil hack for detecting some libraries that on FreeBSD reside in /usr/local
case "$OSNAME" in
	FreeBSD*)
		LDFLAGS="$LDFLAGS -L/usr/local/lib"
		CFLAGS="$CFLAGS -I/usr/local/include"
	;;
esac

dnl AC_DEFINE(OMG_PKGNAME ,$PACKAGE_NAME, [Define this to disable debugging support.])
dnl AC_DEFINE(OMG_RCNAME  ,$PACKAGE_VERSION, [Define this to disable debugging support.])

AC_ISC_POSIX
AC_C_INLINE
AC_PROG_GCC_TRADITIONAL
AC_PROG_MAKE_SET
AC_PROG_INSTALL

dnl AC_HEADER_STC
AC_CHECK_HEADERS([errno.h stddef.h sys/event.h sys/resource.h])

AC_SEARCH_LIBS(socket, socket, , [AC_MSG_ERROR([You have no socket()! Aborting.])])
AC_SEARCH_LIBS(dlopen, dl, ,)
AC_SEARCH_LIBS(backtrace, execinfo, ,)
AC_CHECK_FUNCS([strlcpy strlcat getaddrinfo strdup setproctitle backtrace snprintf])

AC_PATH_PROG(RM, rm)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(BASH, bash)


AX_LIB_MYSQL([0.0])


dnl These checks are for our modules, if we are forcing modules we need to make sure
dnl we error when the needed options are unavailable

hold_ldflags=$LDFLAGS
AC_MSG_CHECKING(for the ld -export-dynamic flag)
LDFLAGS="${LDFLAGS} -Wl,-export-dynamic -Werror"
dnl AC_LINK_IFELSE(AC_LANG_PROGRAM([],[int i;]), found=yes, found=no)
AC_LINK_IFELSE([AC_LANG_SOURCE([int main() { int i; }])], found=yes, found=no)
LDFLAGS=$hold_ldflags

AC_MSG_RESULT($found)
if test "$found" = yes; then
    LDFLAGS="${LDFLAGS} -Wl,-export-dynamic"
fi

dnl This checks for the compiler option to produce PIC
AC_MSG_CHECKING(for compiler option to produce PIC)
if test "$ac_cv_c_compiler_gnu" = "yes"; then
    AC_MSG_RESULT(gcc: -fPIC -DPIC -shared)
    PICFLAGS="-fPIC -DPIC -shared"
    CFLAGS="$CFLAGS -fPIC -DPIC"
else
    AC_MSG_RESULT(no)
fi

dnl Debug-related options
dnl =====================

AC_ARG_ENABLE(assert,
AC_HELP_STRING([--enable-assert],[Enable assert(). Choose between soft(warnings) and hard(aborts the daemon)]),
[assert=$enableval], [assert=no])

if test "$assert" = "no"; then
        AC_DEFINE(NDEBUG, 1, [Define this to disable debugging support.])
elif test "$assert" = "soft"; then
        AC_DEFINE(SOFT_ASSERT, 1, [Define this to enable soft asserts.])
        AC_DEFINE(NDEBUG, 1, [Define this to disable debugging support.])
elif test "$assert" = "yes"; then
	assert = "hard";
fi

dnl AC_ARG_ENABLE(efence,
dnl AC_HELP_STRING([--enable-efence],[Enable Electric Fence debugging]),
dnl [efence=$enableval], [efence=no])

dnl if test "$efence" = "yes"; then
dnl	LIBS="$LIBS -lefence"
dnl fi


AC_ARG_ENABLE(kqueue, AC_HELP_STRING([--enable-kqueue],[Force kqueue() usage.]),
[use_kqueue=$enableval],[use_kqueue=no])

if test ! "x$use_kqueue" = "xno"; then
	AC_CHECK_FUNCS(kevent, [havekqueue=yes], [havekqueue=no])
	AC_DEFINE(USE_KQUEUE, 1, [Use kqueue() instead of the standard select()])
fi

dnl Threading support
ac_cv_threads="yes"
ac_cv_threads_output="enabled"

AC_ARG_ENABLE(threads,
AC_HELP_STRING([--disable-threads],[Disable threading support.]),
[ac_cv_threads="$enableval"],)

AC_MSG_CHECKING(wether to enable dmalloc)

use_dmalloc="no"
ac_cv_dmalloc="disabled"

AC_ARG_ENABLE(dmalloc,
AC_HELP_STRING([--enable-dmalloc],[Enable Dmalloc - Malloc Debugging]),
[use_dmalloc=yes], [use_dmalloc=no])

if test "x$use_dmalloc" = "xyes"; then
    if test "x$ac_cv_threads" = "xyes"; then
        AC_SEARCH_LIBS(dmalloc_shutdown, dmallocth, ac_cv_dmalloc_threadedlib="yes",)

        if test "x$ac_cv_dmalloc_threadedlib" = "xyes"; then
            ac_cv_dmalloc="enabled"
            AC_DEFINE(USE_DMALLOC, 1, [Using debugging malloc])
            AC_MSG_RESULT(yes)
        else
            AC_MSG_RESULT([no, threaded dmalloc library missing])
            ac_cv_dmalloc="disabled, missing threaded dmalloc library"
        fi
    else
	    AC_SEARCH_LIBS(dmalloc_shutdown, dmalloc, ac_cv_dmalloc="enabled",)
        ac_cv_dmalloc="enabled"

	    if test "x$ac_cv_dmalloc" = "xenabled"; then
		    AC_DEFINE(USE_DMALLOC, 1, [Using debugging malloc])
            AC_MSG_RESULT(yes)
        fi
	fi
fi

ac_cv_debug="disabled"
use_debug="no"

AC_ARG_ENABLE(debug,
AC_HELP_STRING([--enable-debug],[Enable core debugging mode.]),
[use_debug=yes], [use_debug=no])

if test "x$use_debug" = "xyes"; then
	AC_MSG_CHECKING(if we are to enable debug commands)
	AC_DEFINE(DEBUG_MODE, 1, [Using debug commands])
	AC_MSG_RESULT(ok)
	ac_cv_debug="enabled"
fi

with_omegastats="yes"

AC_ARG_ENABLE(omegastats,
AC_HELP_STRING([--enable-omegastats],[Enable Omega Statistical Reporting]),
[with_omegastats=$enableval],)


dnl libevent support
ac_cv_libevent="no"
ac_cv_libevent_output="disabled"

AC_ARG_ENABLE(libevent,
AC_HELP_STRING([--enable-libevent[=DIR]],[Enable libevent support (DIR optional).]),
[ac_cv_libevent=$enableval],)

dnl **********************************
dnl Check for --with-confdir
dnl **********************************

AC_MSG_CHECKING([whether to modify confdir])
AC_ARG_WITH(confdir,
AC_HELP_STRING([--with-confdir=DIR],
               [Directory to install config files.]),
               [ confdir=`echo $withval | sed 's/\/$//'`
                 AC_MSG_RESULT(yes)
                 AC_DEFINE_DIR(CONFIG_DIR, confdir, [Prefix where config files are installed.])
                 AC_SUBST_DIR([confdir]) ],
               [ confdir='${prefix}/config'
                 AC_MSG_RESULT(no)
                 AC_DEFINE_DIR(CONFIG_DIR, confdir, [Prefix where config files are installed.])
                 AC_SUBST_DIR([confdir])]
)

dnl 
dnl **********************************************************************
dnl Check for --with-logdir
dnl **********************************************************************

AC_MSG_CHECKING([whether to modify logdir])
AC_ARG_WITH(logdir,
AC_HELP_STRING([--with-logdir=DIR],
               [Directory where to write logfiles.]),
               [ logdir=`echo $withval | sed 's/\/$//'`
                 AC_MSG_RESULT(yes)
                 AC_DEFINE_DIR(LOG_DIR, logdir, [Prefix where to write logfiles.])
                 AC_SUBST_DIR([logdir]) ],
               [ logdir='${prefix}/logs'
                 AC_MSG_RESULT(no)
                 AC_DEFINE_DIR(LOG_DIR, logdir, [Prefix where to write logfiles.])
                 AC_SUBST_DIR([logdir])]
)

dnl **********************************************************************
dnl Check for --with-moduledir
dnl **********************************************************************

AC_MSG_CHECKING([whether to modify moduledir])
AC_ARG_WITH(moduledir,
AC_HELP_STRING([--with-moduledir=DIR],
               [Directory to install modules.]),
               [ moduledir=`echo $withval | sed 's/\/$//'`
                 AC_MSG_RESULT(yes)
                 AC_DEFINE_DIR(MODULE_DIR, moduledir, [Prefix where modules are installed.])
                 AC_SUBST_DIR([moduledir]) ],
               [ moduledir='${prefix}/modules'
                 AC_MSG_RESULT(no)
                 AC_DEFINE_DIR(MODULE_DIR, moduledir, [Prefix where modules are installed.])
                 AC_SUBST_DIR([moduledir])]
)

dnl *****************************************************************
dnl *  Check for --with-coredir
dnl ****************************************************************

AC_MSG_CHECKING([whether to modify coredir])
AC_ARG_WITH(coredir,
AC_HELP_STRING([--with-coredir=DIR],
               [Directory to install core modules.]),
               [ coredir=`echo $withval | sed 's/\/$//'`
                 AC_MSG_RESULT(yes)
                 AC_DEFINE_DIR(CORE_DIR, coredir, [Prefix where core modules are installed.])
                 AC_SUBST_DIR([coredir]) ],
               [ coredir='${prefix}/modules/core'
                 AC_MSG_RESULT(no)
                 AC_DEFINE_DIR(CORE_DIR, coredir, [Prefix where core modules are installed.])
                 AC_SUBST_DIR([coredir])]
)

dnl *****************************************************************
dnl *  Check for --with-gmoddir
dnl ****************************************************************

AC_MSG_CHECKING([whether to modify gmoddir])
AC_ARG_WITH(gmoddir,
AC_HELP_STRING([--with-gmoddir=DIR],
               [Directory to install core modules.]),
               [ gmoddir=`echo $withval | sed 's/\/$//'`
                 AC_MSG_RESULT(yes)
                 AC_DEFINE_DIR(GMOD_DIR, gmoddir, [Prefix where guardian modules are installed.])
                 AC_SUBST_DIR([gmoddir]) ],
               [ gmoddir='${prefix}/modules/guardian'
                 AC_MSG_RESULT(no)
                 AC_DEFINE_DIR(GMOD_DIR, gmoddir, [Prefix where guardian modules are installed.])
                 AC_SUBST_DIR([gmoddir])]
)

dnl *****************************************************************
dnl *  Check for --with-dbdir
dnl *****************************************************************

AC_MSG_CHECKING([wether to modify dbdir])
AC_ARG_WITH(dbdir,
AC_HELP_STRING([--with-dbdir=DIR],
		[Directory to store databases]),
		[ dbdir=`echo $withval | sed 's/\/$//'`
		  AC_MSG_RESULT(yes)
		  AC_DEFINE_DIR(DBDIR, dbdir, [Prefix where databases are saved.])
		  AC_SUBST_DIR([dbdir]) ],
		[ dbdir='${prefix}/var/db'
		  AC_MSG_RESULT(no)
		  AC_DEFINE_DIR(DBDIR, dbdir, [Prefix where databases are saved.])
		  AC_SUBST_DIR([dbdir])]
)

dnl *****************************************************************
dnl *  Check for --with-vardir
dnl *****************************************************************

AC_MSG_CHECKING([wether to modify vardir])
AC_ARG_WITH(vardir,
AC_HELP_STRING([--with-vardir=DIR],
        [Directory where misc runtime files are saved]),
        [ vardir=`echo $withval | sed 's/\/$//'`
          AC_MSG_RESULT(yes)
          AC_DEFINE_DIR(VAR_DIR, vardir, [Prefix where misc runtime files are saved.])
          AC_SUBST_DIR([vardir]) ],
        [ vardir='${prefix}/var'
          AC_MSG_RESULT(no)
          AC_DEFINE_DIR(VAR_DIR, vardir, [Prefix where misc runtime are saved.])
          AC_SUBST_DIR([vardir])]
)


dnl *****************************************************************
dnl *  Check for --with-dbdir
dnl *****************************************************************

AC_MSG_CHECKING([wether to modify tmp_dir])
AC_ARG_WITH(tmpdir,
AC_HELP_STRING([--with-tmpdir=DIR],
        [Directory where temporary files are saved]),
        [ tmpdir=`echo $withval | sed 's/\/$//'`
          AC_MSG_RESULT(yes)
          AC_DEFINE_DIR(TMP_DIR, tmpdir, [Prefix where temporary files are saved.])
          AC_SUBST_DIR([tmpdir]) ],
        [ tmpdir='${vardir}/tmp'
          AC_MSG_RESULT(no)
          AC_DEFINE_DIR(TMP_DIR, tmpdir, [Prefix where temp files are saved.])
          AC_SUBST_DIR([tmpdir])]
)

dnl OpenSSL support

ac_cv_openssl="disabled"

AC_MSG_CHECKING(for OpenSSL)
AC_ARG_ENABLE(openssl,
[AC_HELP_STRING([--enable-openssl[=DIR]],[Enable OpenSSL support (DIR optional).])
AC_HELP_STRING([--disable-openssl],[Disable OpenSSL support.])],
[cf_enable_openssl=$enableval],
[cf_enable_openssl="auto"])

if test "$cf_enable_openssl" != "no" ; then
    cf_openssl_basedir=""
    if test "$cf_enable_openssl" != "auto" &&
    test "$cf_enable_openssl" != "yes" ; then
        dnl Support for --enable-openssl=/some/place
        cf_openssl_basedir="`echo ${cf_enable_openssl} | sed 's/\/$//'`"
    else
    dnl Do the auto-probe here.  Check some common directory paths.
        for dirs in /usr/local/ssl /usr/pkg /usr/local \
        /usr/local/openssl ; do
            if test -f "${dirs}/include/openssl/opensslv.h" ; then
                cf_openssl_basedir="${dirs}"
            break
            fi
        done
        unset dirs
    fi
    dnl Now check cf_openssl_found to see if we found anything.
    if test ! -z "$cf_openssl_basedir"; then
        if test -f "${cf_openssl_basedir}/include/openssl/opensslv.h" ; then
            SSL_INCLUDES="-I${cf_openssl_basedir}/include"
            SSL_LIBS="-L${cf_openssl_basedir}/lib"
        else
        dnl OpenSSL wasn't found in the directory specified.  Naughty
        dnl administrator...
        cf_openssl_basedir=""
        fi
    else
    dnl Check for stock FreeBSD 4.x and 5.x systems, since their files
    dnl are in /usr/include and /usr/lib.  In this case, we don't want to
    dnl change INCLUDES or LIBS, but still want to enable OpenSSL.
    dnl We can't do this check above, because some people want two versions
    dnl of OpenSSL installed (stock FreeBSD 4.x/5.x and /usr/local/ssl)
    dnl and they want /usr/local/ssl to have preference.
        if test -f "/usr/include/openssl/opensslv.h" ; then
            cf_openssl_basedir="/usr"
        fi
    fi
    dnl If we have a basedir defined, then everything is okay.  Otherwise,
    dnl we have a problem.
    if test ! -z "$cf_openssl_basedir"; then
        AC_MSG_RESULT($cf_openssl_basedir)
        cf_enable_openssl="yes"
    else
        AC_MSG_RESULT([not found. Specify a correct path?])
        cf_enable_openssl="no"
    fi
    unset cf_openssl_basedir
else
    dnl If --disable-openssl was specified
    AC_MSG_RESULT(disabled)
fi

save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $SSL_INCLUDES"
save_LIBS="$LIBS"
LIBS="$LIBS $SSL_LIBS"
if test "$cf_enable_openssl" != no; then
    dnl Check OpenSSL version (must be 0.9.6 or above!)
    AC_MSG_CHECKING(for OpenSSL 0.9.6 or above)
    AC_RUN_IFELSE([AC_LANG_SOURCE([#include <openssl/opensslv.h>
	#include <stdlib.h>

	int main() {
	if ( OPENSSL_VERSION_NUMBER >= 0x00906000)
	exit(0); else exit(1);}])],
	cf_openssl_version_ok=yes,
	cf_openssl_version_ok=no,
	cf_openssl_version_ok=no)
dnl        AC_LANG_PROGRAM(
dnl            [#include <openssl/opensslv.h>
dnl        #include <stdlib.h>],
dnl        [[if ( OPENSSL_VERSION_NUMBER >= 0x00906000)
dnl        exit(0); else exit(1);]]),
dnl    cf_openssl_version_ok=yes,
dnl    cf_openssl_version_ok=no,
dnl    cf_openssl_version_ok=no)

    if test "$cf_openssl_version_ok" = yes; then
        AC_MSG_RESULT(found)
		ac_cv_openssl="enabled"

        dnl Do all the HAVE_LIBCRYPTO magic -- and check for ciphers
        CPPFLAGS="$CPPFLAGS $SSL_LIBS"
        AC_CHECK_LIB(crypto, RSA_free)
        SSL_LIBS="$SSL_LIBS -lcrypto -lssl"
        SSL_SRCS_ENABLE='$(SSL_SRCS)'

		AC_DEFINE(HAVE_OPENSSL, 1, [OpenSSL support enabled])
    else
            AC_MSG_RESULT(no - OpenSSL support disabled)
    fi
fi

CPPFLAGS="$save_CPPFLAGS"
LIBS="$save_LIBS"

dnl End OpenSSL detection

xmlrpc_c_config="no"

AC_ARG_WITH(xmlrpc-c,
AC_HELP_STRING([--with-xmlrpc-c=PATH], [Path to xmlrpc-c-config]), [xmlrpc_c_config="$withval"],[xmlrpc_c_config="no"])

if test "X$xmlrpc_c_config" = "Xyes"; then
	xmlrpc_c_config="no";
fi

AC_MSG_CHECKING(for xmlrpc-c)

ac_cv_xmlrpc="disabled"

if test "X$xmlrpc_c_config" = "Xno"; then
	if eval xmlrpc-c-config --version 2>/dev/null >/dev/null; then
		CFLAGS="$CFLAGS `xmlrpc-c-config client --cflags`"
		LIBS="$LIBS `xmlrpc-c-config client --libs`"

		AC_DEFINE(HAVE_XMLRPC_C, 1, [Support for XMLRCP-C])
		AC_MSG_RESULT(ok)

		ac_cv_xmlrpc="enabled"
	else
		AC_MSG_RESULT(not found)
	fi
else
	if eval $xmlrpc_c_config --version 2>/dev/null >/dev/null; then
		CFLAGS="$CFLAGS `$xmlrpc_c_config client --cflags`"
		LIBS="$LIBS `$xmlrpc_c_config client --libs`"

		AC_DEFINE(HAVE_XMLRPC_C, 1, [Support for XMLRPC-C])
		AC_MSG_RESULT(ok)

		ac_cv_xmlrpc="enabled"
	else
		AC_MSG_RESULT(not found)
	fi
fi

gnutls="no"
ac_cv_gnutls="disabled"
ac_cv_gnutls_ret=`pkg-config --modversion gnutls`

AC_ARG_ENABLE(gnutls,
AC_HELP_STRING([--enable-gnutls], [Enable GNUTls]), [gnutls="yes"],[gnutls="no"])

if test "X$gnutls" = "Xyes"; then
    if eval pkg-config --modversion gnutls 2>/dev/null >/dev/null; then
        CFLAGS="$CFLAGS `pkg-config --cflags gnutls`"
        LIBS="$LIBS `pkg-config --libs gnutls`"

        AC_DEFINE(HAVE_GNUTLS, 1, [Support for GNUTLS])
        ac_cv_gnutls="enabled"
    fi
fi

ac_cv_libopm=""
ac_cv_found_libopm_inc="no"
ac_cv_found_libopm_lib="no"

ac_cv_sqlite3="disabled"
ac_cv_sqlite3_val=""
ac_cv_sqlite3_path=""
ac_cv_found_sqlite3_lib=""
ac_cv_found_sqlite3_inc=""

dnl Search for SQLite
AC_ARG_WITH(sqlite3,
AC_HELP_STRING([--with-sqlite3=DIR],[Location to SQLite3]),
[ac_cv_sqlite3_val=$withval], [])

AC_CHECK_HEADER(sqlite3.h,
[ac_cv_found_sqlite3_inc="yes"],
[
	AC_MSG_CHECKING(for sqlite3.h in given path)
	if test -f "$ac_cv_sqlite3_val/include/sqlite3.h"; then
		AC_MSG_RESULT(yes)
		ac_cv_found_sqlite3_inc="yes"
		ac_cv_sqlite3_path="$ac_cv_sqlite3_val"
	else
		AC_MSG_RESULT(no)
	fi
], [])

AC_CHECK_LIB([sqlite3], [main], [ac_cv_found_sqlite3_lib="yes"],
[
	AC_MSG_CHECKING(for -lsqlite3 in given path)
	if test -e "$ac_cv_sqlite3_val/lib/libsqlite3.so"; then
		AC_MSG_RESULT(yes)
		ac_cv_found_sqlite3_lib="yes"
	else
		AC_MSG_RESULT(no)
	fi
])

if test "X$ac_cv_found_sqlite3_inc" = "Xyes" &&
   test "X$ac_cv_found_sqlite3_lib" = "Xyes"; then
		LIBS="$LIBS -lsqlite3"

		if test "X$ac_cv_sqlite3_path" != "X"; then
			CFLAGS="$CFLAGS -I $ac_cv_sqlite3_path/include/"
			LDFLAGS="$LDFLAGS -L $ac_cv_sqlite3_path/lib/"
		fi

		AC_DEFINE(HAVE_SQLITE, 1, [SQLite is available for use])
		ac_cv_sqlite3="enabled"
fi

if test "X$ac_cv_libevent" != "Xno"; then
    ac_cv_libevent_found="no"

    if test "X$ac_cv_libevent" != "Xyes"; then
        CFLAGS="$CFLAGS -I$ac_cv_libevent/include"
        LDFLAGS="$LDFLAGS -L$ac_cv_libevent/lib/"
    fi

    AC_CHECK_LIB(event, event_init, [ac_cv_libevent_found=yes], )

    if test "X$ac_cv_libevent_found" = "Xyes"; then
        AC_DEFINE(HAVE_LIBEVENT, 1, [libevent is available for use])
        ac_cv_libevent_output="enabled"
    fi
fi

if test "X$ac_cv_threads" = "Xyes"; then
    ACX_PTHREAD()
    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
    LDFLAGS="$LDFLAGS $PTHREAD_LDFLAGS"
    LIBS="$LIBS $PTHREAD_LIBS"
else
    ac_cv_threads_output="disabled"
fi

if test "X$with_omegastats" = "Xyes"; then
	AC_DEFINE(OMEGASTATS, 1, [Compiled with omega statistical module.])
	OSTATSMOD="m_omegastats.c"
	AC_SUBST(OSTATSMOD)
fi


AC_SUBST(LDFLAGS)
AC_SUBST(PICFLAGS)

AC_SUBST(SSL_INCLUDES)
AC_SUBST(SSL_LIBS)

if test "$prefix" = "NONE"; then
	AC_DEFINE_UNQUOTED(PREFIX, "$ac_default_prefix", [Prefix where omega is installed.])
else
	prefix=`echo $prefix | sed 's/\/$//'`
	AC_DEFINE_UNQUOTED(PREFIX, "$prefix", [Prefix where omega is installed.])
fi

if test ! -d libopm; then
	AC_MSG_ERROR(libopm is not present. Cannot continue.)
fi

AC_CONFIG_SUBDIRS(libopm)

CFLAGS="$CFLAGS $SSL_INCLUDES $MYSQL_CFLAGS -I$PWD/libopm/src/"
LIBS="$LIBS $SSL_LIBS $MYSQL_LDFLAGS $PWD/libopm/src/.libs/libopm.a"

AC_CONFIG_FILES(
    Makefile                   \
    src/Makefile               \
    src/socketengine/Makefile  \
    modules/Makefile           \
    modules/core/Makefile      \
    modules/db/Makefile        \
    modules/client/Makefile    \
    modules/protocol/Makefile  \
    docs/Makefile              \
    tools/Makefile             \
)


AC_OUTPUT

dnl I make this look good :D

echo
echo "Compiling $PACKAGE_NAME ($PACKAGE_VERSION)"
echo
echo "Installing into: $prefix"
echo

if test "X$with_omegastats" = "Xyes"; then
echo "Omega Stats.............. enabled (read docs/OmegaStatistics.txt for information)"
else
echo "Omega Stats.............. disabled"
fi

echo
echo
echo "Compiling $PACKAGE_NAME ($PACKAGE_VERSION)"
echo
echo "Installing into: $prefix"
echo
echo "Assert................... $assert"
echo "Dmalloc.................. $ac_cv_dmalloc"
echo "Threading................ $ac_cv_threads_output"
echo "Debug.................... $ac_cv_debug"
echo "XMLRPC................... $ac_cv_xmlrpc"
echo
echo "Secure Socket "
echo "   OpenSSL............... $ac_cv_openssl"
echo "   GnuTLS................ $ac_cv_gnutls ($ac_cv_gnutls_ret)"
echo "Database "
echo "   MySQL  ............... $found_mysql"
echo "   SQLite3............... $ac_cv_sqlite3"
echo "Misc "
echo "   libevent.............. $ac_cv_libevent_output"
echo


echo "To complete installation type make"
echo
