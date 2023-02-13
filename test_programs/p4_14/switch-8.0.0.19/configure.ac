#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([switch], [0.1], [])
AM_PATH_PYTHON([2.7],,[:])
# This prevents autotools from automatically adding -O2 flag.
: ${CXXFLAGS=""}
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config.h])

AC_ARG_ENABLE([asan-checks],
              [AS_HELP_STRING([--enable-asan-checks],
                              [Build with address sanitizer checking])],
              [enable_asan=yes],
              [enable_asan=no])
AM_CONDITIONAL([IS_ASAN_ENABLED], [test "x$enable_asan" = "xyes"])
if test "x$enable_asan" = "xyes"; then
  ASAN_FLAGS="-fsanitize=address -fsanitize=undefined -fsanitize-recover=address,undefined"
else
  ASAN_FLAGS=""
fi

coverage_enabled=no
AC_ARG_ENABLE([coverage],
    AS_HELP_STRING([--enable-coverage], [Enable gcov flags]))
AS_IF([test "x$enable_coverage" = "xyes"], [
    coverage_enabled=yes
    AC_DEFINE([COVERAGE], [], ["Link with gcov."])
    COVERAGE_FLAGS="--coverage"
])
AM_CONDITIONAL([COVERAGE], [test "x$enable_coverage" = "xyes"])

want_bmv2=no
AC_ARG_WITH([bmv2],
    AS_HELP_STRING([--with-bmv2], [Build for bmv2 target]),
    [want_bmv2=yes], [])

AM_CONDITIONAL([WITH_BMV2], [test "$want_bmv2" = yes])

want_tofino=no
AC_ARG_WITH([tofino],
    AS_HELP_STRING([--with-tofino], [Build for tofino target]),
    [want_tofino=yes], [])

AM_CONDITIONAL([WITH_TOFINO], [test "$want_tofino" = yes])

want_bmv2tofino=no
AC_ARG_WITH([tofinobm],
    AS_HELP_STRING([--with-tofinobm], [Build for tofinoBM target]),
    [want_bmv2tofino=yes], [])

AM_CONDITIONAL([WITH_BMV2TOFINO], [test "$want_bmv2tofino" = yes])

want_switchapi=no
AC_ARG_WITH([switchapi],
    AS_HELP_STRING([--with-switchapi], [Build with switchapi]),
    [want_switchapi=yes], [])

want_switchsai=no
AC_ARG_WITH([switchsai],
    AS_HELP_STRING([--with-switchsai],
                   [Build with switchsai, will build switchapi as well]),
    [want_switchsai=yes], [])

cpu_veth=no
AC_ARG_WITH([cpu-veth],
    AS_HELP_STRING([--with-cpu-veth], [Build with cpu veth port]),
    [cpu_veth=yes], [])

bf_sysroot=
AC_ARG_WITH([libtool-sysroot],
            AS_HELP_STRING([--with-libtool-sysroot],[Specify sysroot directory]),
            [AS_IF([test "x$with_libtool_sysroot" = "xno"],
                   [bf_sysroot=],
                   [bf_sysroot=$with_libtool_sysroot])],
            [bf_sysroot=])
AC_SUBST([AM_CPPFLAGS], ["-I$bf_sysroot\$(includedir)"])

AS_IF([test "$want_switchsai" = yes], [
    want_switchapi=yes
    # need to be configurable at runtime for now
    # MY_CPPFLAGS="$MY_CPPFLAGS -DSWITCHSAI_ENABLE"
])

AS_IF([test "$want_switchapi" = yes], [
    # need to be configurable at runtime for now
    # MY_CPPFLAGS="$MY_CPPFLAGS -DSWITCHAPI_ENABLE"
    AC_CHECK_HEADERS([netlink/netlink.h netlink/msg.h netlink/route/neighbour.h netlink/route/nexthop.h])
    netlink_libs=$(pkg-config --libs libnl-route-3.0)
    AC_SUBST([NETLINK_LIBS], ["$netlink_libs"])
])

AM_CONDITIONAL([WITH_SWITCHAPI], [test "$want_switchapi" = yes])
AM_CONDITIONAL([WITH_SWITCHSAI], [test "$want_switchsai" = yes])

AC_ARG_ENABLE([thrift],
    AS_HELP_STRING([--enable-thrift], [Enable thrift flags]))
AS_IF([test "x$enable_thrift" = "xyes"], [
    AC_DEFINE([ENABLE_THRIFT], [], ["Build with thrift."])
])

AC_PROG_CC_STDC
AC_PROG_CXX
LT_INIT

AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/doxygen.m4])

# enforce -std=c++11
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

# Checks for header files.
AC_LANG_PUSH(C)
AC_LANG_PUSH(C++)

warn_if_no_ptf=no

# To simplify usage, we will update PATH, CPPFLAGS,.. to include the 'prefix'
# ones
adl_RECURSIVE_EVAL([$bindir], [BIN_DIR])
AC_SUBST([BIN_DIR], ["$BIN_DIR"])
adl_RECURSIVE_EVAL([$includedir], [INCLUDE_DIR])
adl_RECURSIVE_EVAL([$pythondir], [PYTHON_DIR])
adl_RECURSIVE_EVAL([$libdir], [LIB_DIR])
AC_SUBST([LIB_DIR], ["$bf_sysroot$LIB_DIR"])

conf_tgts=
AS_IF([test "$want_tofino" = yes], [
  AC_SUBST([TOFINO_CPPFLAGS], ["-D__TARGET_TOFINO__"])
  AC_SUBST([conf_tgts], ["tofino"])
])

AS_IF([test "$want_bmv2" = yes], [
  # A little extra: check for simple_switch target; of course it is not
  # critical if we cannot find it
  AC_PATH_PROG([BM_SIMPLE_SWITCH], [simple_switch], [],
    [$PATH$PATH_SEPARATOR$BIN_DIR])
  AS_IF([test x"$BM_SIMPLE_SWITCH" = x],
    [AC_MSG_WARN([You are compiling for bmv2, but simple_switch binary was not found])])

  warn_if_no_ptf=yes

  AC_SUBST([BMV2_CPPFLAGS], ["-DBMV2 -D__TARGET_BMV2__"])
])

AS_IF([test "$want_bmv2tofino" = yes], [
  # A little extra: check for bmv2-tofino binary; of course it is not
  # critical if we cannot find it
  AC_PATH_PROG([BMV2_TOFINO_BINARY_PATH], [tofinobmv2], [],
    [$PATH$PATH_SEPARATOR$BIN_DIR])
  AS_IF([test x"$BMV2_TOFINO_BINARY_PATH" = x],
    [AC_MSG_WARN([You are compiling for bmv2-tofino, but bmv2-tofino binary was not found])])
  warn_if_no_ptf=yes

  AC_SUBST([BMV2TOFINO_CPPFLAGS], ["-DBMV2TOFINO -D__TARGET_TOFINO__"])
])

AM_CONDITIONAL([IS_THRIFT_ENABLED], [test "x$enable_thrift" = "xyes"])
AS_IF([test "x$enable_thrift" = "xyes"], [
  # Check for thrift support (Apache Thrift RPC library)
  AC_PATH_PROG([THRIFT], [thrift], [])
  AS_IF([test x"$THRIFT" = x], [AC_MSG_ERROR([cannot find thrift])])
  AC_CHECK_LIB([thrift], [_init], [], [AC_MSG_ERROR([Missing thrift library])])
  # TODO: check if linking with these was necessary
  # AC_CHECK_LIB([ssl], [SSL_accept], [], [AC_MSG_ERROR([Missing ssl library])])
  # AC_CHECK_LIB([crypto], [MD5_init], [], [AC_MSG_ERROR([Missing crypto library])])
  AC_CHECK_HEADER([thrift/Thrift.h], [], [AC_MSG_ERROR([Thrift headers not found])])
  AC_SUBST([THRIFT_LIB], ["-lthrift"])
])

AC_CHECK_HEADERS([assert.h stdio.h string.h sys/stat.h sys/types.h unistd.h],
                 [], [AC_MSG_ERROR([Missing header file])])


AS_IF([test "x$enable_coverage" = "xyes"], [
  AC_CHECK_LIB([gcov], [__gcov_init], [], [AC_MSG_ERROR([Missing gcov library])])
])

dnl The following code snippet is copied from Judy's configure.ac
dnl If we're compiling for a little-endian system, define JU_LITTLE_ENDIAN.
dnl If we can't tell what kind of system we're compling for, alert the
dnl user as described in 'info autoconf'.
AC_C_BIGENDIAN(, AC_DEFINE(JU_LITTLE_ENDIAN, 1,
  [Define to 1 on little-endian systems.]))

dnl  Figure out if we are 32-bit or 64-bit (LP64)
AC_CHECK_SIZEOF(void *)
if test "$ac_cv_sizeof_void_p" = 8; then
  AC_MSG_RESULT(Detected 64-bit Build Environment)
  JUDY_CFLAGS="-DJU_64BIT"
else
  AC_MSG_RESULT(Detected 32-bit Build Environment)
  JUDY_CFLAGS="-UJU_64BIT"
fi

AC_PATH_PROG([PTF], [ptf], [], [$PATH$PATH_SEPARATOR$BIN_DIR])
ptf_found=no
AS_IF([test x"$PTF" != x],
  [ptf_found=yes],
  [AS_IF([test "$warn_if_no_ptf" = yes],
    [AC_MSG_NOTICE([No installed version of the ptf testing framework were found])])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([socket])
AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_TYPES([ptrdiff_t])
AC_C_INLINE
AC_FUNC_MALLOC
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_SUBST([SWITCH_CFLAGS], ["-g -Wall -Wextra -Wno-unused-parameter -Werror -Wno-error=unused -Wno-error=unused-variable -Wno-error=unused-label -Wno-error=unused-but-set-variable"])
AC_SUBST([AM_CFLAGS], ["$ASAN_FLAGS $JUDY_CFLAGS $COVERAGE_FLAGS"])
AC_SUBST([AM_CXXFLAGS], ["$ASAN_FLAGS $COVERAGE_FLAGS"])
AC_SUBST([AM_CPPFLAGS], ["$AM_CPPFLAGS $MY_CPPFLAGS"])
AC_SUBST([AM_LDFLAGS], ["$COVERAGE_FLAGS -L$LIB_DIR"])
AC_SUBST([MY_PYTHONPATH], ["$PYTHON_DIR"])

# Generate makefiles
AC_CONFIG_FILES([Makefile
                 switchapi/Makefile
                 switchsai/Makefile])

AS_IF([test "$want_tofino" = yes], [
  AC_CONFIG_FILES([targets/Makefile])
  for conf_tgt in $conf_tgts
  do
  conf_file=
  AS_IF([test "$cpu_veth" = yes],
        [conf_file=[targets/$conf_tgt/switch.conf:targets/$conf_tgt/switch-cpu-veth.conf.in]],
        [conf_file=[targets/$conf_tgt/switch.conf]],
  )
  AC_CONFIG_FILES([targets/$conf_tgt/multi_device_switch.conf
                   $conf_file
                   $conf_sai_file
                   targets/$conf_tgt/switch-sai-ptf.conf
                   targets/$conf_tgt/switch-pd.conf
                   targets/$conf_tgt/switch-sai.conf])
  done
])


AS_IF([test "$want_tofino" = yes], [
  AC_CONFIG_FILES([tofino/Makefile])
])

AS_IF([test "$want_bmv2" = yes], [
  AC_CONFIG_FILES([bmv2/Makefile])

  AC_CONFIG_FILES([bmv2/run_bm.sh], [chmod +x bmv2/run_bm.sh])
  AC_CONFIG_FILES([bmv2/run_drivers.sh], [chmod +x bmv2/run_drivers.sh])
])
AS_IF([test "$want_bmv2tofino" = yes], [
  AC_CONFIG_FILES([bmv2-tofino/Makefile])

])

AC_ARG_VAR([P4PPFLAGS], [Preprocessor flags for P4 program])

AX_APPEND_FLAG([--field-aliases $ac_abs_confdir/bmv2/field-aliases.bmv2], [BM_P4FLAGS])
export BM_P4FLAGS
export P4_NAME=switch
export P4_PATH=$ac_abs_confdir/p4src/switch.p4
export P4_PREFIX=dc
AC_CONFIG_SUBDIRS([p4-build])

AC_OUTPUT

AS_ECHO("")
AS_ECHO("switch features recap ......................")
AS_ECHO("Coverage enabled .............. : $coverage_enabled")
AS_ECHO("With switchsai ................ : $want_switchsai")
AS_ECHO("With switchapi ................ : $want_switchapi")
AS_ECHO("Compile for tofino ............ : $want_tofino")
AS_ECHO("Compile for bmv2 .............. : $want_bmv2")
AS_ECHO("Compile for tofinobm .......... : $want_bmv2tofino")
AS_IF([test "$want_bmv2" = yes], [
simple_switch_found=yes
AS_IF([test x"$BM_SIMPLE_SWITCH" = x], [simple_switch_found=no])
AS_ECHO("  simple_switch found ......... : $simple_switch_found")
])
AS_ECHO("Thrift enabled ................ : $enable_thrift")
AS_ECHO("PTF found ..................... : $ptf_found")
