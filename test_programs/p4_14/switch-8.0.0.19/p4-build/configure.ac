#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([p4-build], [0.1], [antonin@barefootnetworks.com])
# This prevents autotools from automatically adding -O2 flag.
: ${CXXFLAGS="-g"}
AM_INIT_AUTOMAKE([foreign subdir-objects])
# TODO(antonin): had to comment this out, because I am using a GNU make
# extension in bmv2/Makefile.am
# AM_INIT_AUTOMAKE([-Wportability -Werror foreign subdir-objects])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

# will abort if there is no Python interpreter >= 2.7
AM_PATH_PYTHON([2.7])

AC_ARG_ENABLE([asan-checks],
              [AS_HELP_STRING([--enable-asan-checks],
                              [Build with address sanitizer checking])],
              [enable_asan=yes],
              [enable_asan=no])
AM_CONDITIONAL([IS_ASAN_ENABLED], [test "x$enable_asan" = "xyes"])

# Enable Openflow
AC_WITH([of], [want_of=yes], [])
AM_CONDITIONAL([COND_PLUGIN_OPENFLOW], [test "$want_of" = yes])

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

want_pi=no
AC_ARG_WITH([pi],
    AS_HELP_STRING([--with-pi], [Build for pi target]),
    [want_pi=yes], [])

AM_CONDITIONAL([WITH_PI], [test "$want_pi" = yes])

AC_ARG_WITH([p4c],
    AS_HELP_STRING([--with-p4c=path/to/p4c], [Build using the p4c compiler]),
    [P4C="$withval"], [])

AC_ARG_VAR([P4_NAME], [Name of the P4 program])
AS_IF([test "$P4_NAME" = ""], [
  AC_MSG_ERROR([Environment variable P4_NAME needs to be set])
])
AS_IF([$PYTHON $srcdir/tools/check_identifier.py "$P4_NAME"], [], [
  AC_MSG_ERROR([Environment variable P4_NAME needs to be a valid C identifier])
])

AC_ARG_VAR([P4_PATH], [Path to the P4 program])
AS_IF([test "$P4_PATH" = ""], [
  AC_MSG_ERROR([Environment variable P4_PATH needs to be set])
])
check_abs_path=$($PYTHON $srcdir/tools/check_p4_path.py $P4_PATH)
AS_IF([test "$check_abs_path" != yes], [
  AC_MSG_ERROR([The P4_PATH you provided does not exist or is not absolute])
])

AC_ARG_VAR([P4_PREFIX], [Prefix for the PD APIs])
AS_IF([test "$P4_PREFIX" = ""], [
  AC_MSG_NOTICE([Environment variable P4_PREFIX not set, using P4_NAME])
  P4_PREFIX="$P4_NAME"
  AC_SUBST([P4_PREFIX])
])
AS_IF([$PYTHON $srcdir/tools/check_identifier.py "$P4_PREFIX"], [], [
  AC_MSG_ERROR([Environment variable P4_PREFIX needs to be a valid C identifier])
])

AC_ARG_VAR([P4PPFLAGS], [Preprocessor flags for P4 program])
AC_ARG_VAR([P4FLAGS], [Compiler flags for p4c-tofino compiler])
AC_ARG_VAR([P4JOBS], [Specify the number of p4c-tofino jobs to run simultaneously])
AS_IF([test "$P4JOBS" = ""], [AC_SUBST([P4JOBS], "4")])
AC_ARG_VAR([PDFLAGS], [Compiler flags for tofino pd generation])
AC_ARG_VAR([BM_P4FLAGS], [Compiler flags for p4c-bm compiler])

AC_ARG_VAR([P4_BUILD_ASSUME_PDFIXED], [Bypass check for pdfixed headers])

# Test environments
diff_w_pdtest=$($PYTHON $srcdir/tools/diff.py $P4_PATH $srcdir/bmv2/tests/pdtest/pdtest.p4)
AM_CONDITIONAL([BMV2_PDTEST_ENV], [test "$P4_NAME" = "pdtest"&& test "$P4_PREFIX" = "pdtest" && test "$diff_w_pdtest" = "no"])
AM_COND_IF([BMV2_PDTEST_ENV], [AC_MSG_NOTICE([Bmv2 test environment detected])])

diff_w_oftest=$($PYTHON $srcdir/tools/diff.py $P4_PATH $srcdir/bmv2/tests/openflow/oftest.p4)
AM_CONDITIONAL([BMV2_OFTEST_ENV], [test "$P4_NAME" = "oftest" &&test "$P4_PREFIX" = "oftest" && test "$diff_w_oftest" = "no"])
AM_COND_IF([BMV2_OFTEST_ENV], [AC_MSG_NOTICE([Bmv2 test environment detected])])

AC_ARG_VAR([P4C_BM_FLAGS], [Options to pass to the p4c-bm compiler])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC_STDC
LT_INIT

# enforce -std=c++11
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

# Checks for header files.
AC_LANG_PUSH(C)
AC_LANG_PUSH(C++)

AC_CHECK_HEADERS([algorithm array cassert cmath queue \
cstdio string sys/stat.h sys/types.h ctime tuple unistd.h unordered_map \
utility vector], [], [AC_MSG_ERROR([Missing header file])])

# To simplify usage, we will update PATH, CPPFLAGS,.. to include the 'prefix'
# ones
adl_RECURSIVE_EVAL([$bindir], [BIN_DIR])
adl_RECURSIVE_EVAL([$includedir], [INCLUDE_DIR])
adl_RECURSIVE_EVAL([$pythondir], [PYTHON_DIR])
adl_RECURSIVE_EVAL([$libdir], [LIB_DIR])
bf_sysroot=
AC_ARG_WITH([libtool-sysroot],
            AS_HELP_STRING([--with-libtool-sysroot],[Specify sysroot directory]),
            [AS_IF([test "x$with_libtool_sysroot" = "xno"],
                   [bf_sysroot=],
                   [bf_sysroot=$with_libtool_sysroot])],
            [bf_sysroot=])
AC_SUBST([FULL_INC_DIR],[$bf_sysroot$includedir])

# Thrift
AC_ARG_ENABLE([thrift],
    AS_HELP_STRING([--enable-thrift], [Enable thrift flags]))
AS_IF([test "x$enable_thrift" = "xyes"], [
    AC_DEFINE([ENABLE_THRIFT], [], ["Build with thrift."])
])

check_for_pthread=no

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
    check_for_pthread=yes
], [enable_thrift=no])


AX_SAVE_FLAGS(SAVE_1)
CPPFLAGS="$CPPFLAGS -I$INCLUDE_DIR"

AC_PATH_PROG([PI_GEN_NATIVE_JSON], [pi_gen_native_json], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
AM_CONDITIONAL([PI_GEN_NATIVE_JSON_EXISTS], [test x"$PI_GEN_NATIVE_JSON" != x])
AC_PATH_PROG([PI_GEN_DEFS], [pi_gen_fe_defines], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
AM_CONDITIONAL([PI_GEN_DEFS_EXISTS], [test x"$PI_GEN_DEFS" != x])

AS_IF([test "$want_bmv2" = yes || test "$want_bmv2tofino" = yes], [
    # Locate res.thrift needed to run the thrift compiler on the PD IDL
    # Quite nasty, but I don't see another way
    ab_FIND_HEADER_PATH(
      ["bm/pdfixed/thrift/res.thrift"],
      [pdfixedthrift_path],
      [AC_MSG_ERROR([cannot find bm/pdfixed/thrift/res.thrift, did you install bmv2 with pdfixed?])])
    AC_SUBST([BM_THRIFT_INCLUDE], ["$pdfixedthrift_path"])
])

AS_IF([test "$want_bmv2" = yes], [
    AS_IF([test x"$enable_thrift" != "xyes"],
      [AC_MSG_ERROR([When using bmv2, --enable-thrift is required])])

    AC_PATH_PROG([P4C_BM], [p4c-bmv2], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$P4C_BM" = x], AC_MSG_ERROR([cannot find p4c-bmv2 compiler]))

    AS_IF([test "$P4_BUILD_ASSUME_PDFIXED" = ""], [
      AC_CHECK_HEADER([bm/pdfixed/pd_common.h], [],
        [AC_MSG_ERROR([bmv2 pdfixed headers not found])])
    ])

    AC_CHECK_HEADER([bm/Standard.h], [],
      [AC_MSG_ERROR([bmv2 Thrift headers not found])])

    AC_CHECK_LIB([Judy], [Judy1Next], [], [AC_MSG_ERROR([Missing libJudy])])
    AC_CHECK_LIB([nanomsg], [nn_errno], [], [AC_MSG_ERROR([Missing libnanomsg])])
    check_for_pthread=yes
])

AS_IF([test "$want_tofino" = yes], [
    # if the compiler was not specified on the command line, default
    # to p4c-tofino
    AS_IF([ test x"$P4C" = x], [
        AC_PATH_PROG([P4C], [p4c-tofino], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
        AS_IF([test x"$P4C" = x],
              AC_MSG_ERROR([cannot find p4c-tofino compiler]))])
    # test the compiler version
    AS_IF([ test -x $P4C ],
          [p4cversion=`$P4C --version | cut -f 1 -d " "`],
          [p4cversion=""])
    AS_IF([test "$p4cversion" = "p4c"],
          [AC_SUBST([P4C_VERSION], ["V6"])], [AC_SUBST([P4C_VERSION], ["V5"])])

    AC_PATH_PROG([PDGEN], [generate_tofino_pd], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$PDGEN" = x], AC_MSG_ERROR([cannot find tofino PD-API generator]))

    AC_PATH_PROG([PDCLI], [gencli], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$PDCLI" = x], AC_MSG_ERROR([cannot find tofino PD-CLI plugin generator]))


    AS_IF([test "$P4_BUILD_ASSUME_PDFIXED" = ""], [
      AC_CHECK_HEADER([tofino/pdfixed/pd_common.h], [],
        [AC_MSG_ERROR([tofino pdfixed headers not found])])
    ])
])
AM_CONDITIONAL([WITH_P4C_V6], [ test "$P4C_VERSION" = "V6" -o "x$P4C_V6" = "xyes"])
AM_CONDITIONAL([TOFINO_LINK_PROFILER],
               [test "$want_tofino" = yes && $PYTHON $srcdir/tools/string_contains.py "$PDFLAGS" "--gen-perf-test-pd"])

AS_IF([test "$want_bmv2tofino" = yes], [
    AS_IF([test x"$enable_thrift" != "xyes"],
      [AC_MSG_ERROR([When using bmv2-tofino, --enable-thrift is required])])

    AC_PATH_PROG([P4C_BM], [p4c-bmv2], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$P4C_BM" = x], AC_MSG_ERROR([cannot find p4c-bmv2 compiler]))

    AS_IF([test "$P4_BUILD_ASSUME_PDFIXED" = ""], [
      AC_CHECK_HEADER([tofinobm/pdfixed/pd_common.h], [],
        [AC_MSG_ERROR([bmv2-tofino pdfixed headers not found])])
    ])

    # Locate res.thrift needed to run the thrift compiler on the PD IDL
    # Quite nasty, but I don't see another way
    ab_FIND_HEADER_PATH(
      ["tofinobm/pdfixed/thrift/res.thrift"],
      [pdfixedthrift_path],
      [AC_MSG_ERROR([cannot find tofinobm/pdfixed/thrift/res.thrift, did you install bmv2-tofino with pdfixed?])])
    AC_SUBST([BM_THRIFT_INCLUDE], ["$pdfixedthrift_path"])

    # Locate intrinsic_metadata.p4 needed to compile programs in bmv2-tofino
    # Quite nasty, but I don't see another way
    ab_FIND_HEADER_PATH(
      ["tofinobm/tofino/intrinsic_metadata.p4"],
      [bmv2tofino_header_path],
      [AC_MSG_ERROR([cannot find tofinobm/tofino/intrinsic_metadata.p4, did you install bmv2-tofino?])])
    AC_SUBST([BMV2TOFINO_HEADER_INCLUDE], [`AS_DIRNAME(["$bmv2tofino_header_path"])`])
    AC_CHECK_HEADER([bm/Standard.h], [],
      [AC_MSG_ERROR([bmv2-tofino Thrift headers not found])])

    AC_CHECK_LIB([Judy], [Judy1Next], [], [AC_MSG_ERROR([Missing libJudy])])
    AC_CHECK_LIB([nanomsg], [nn_errno], [], [AC_MSG_ERROR([Missing libnanomsg])])
    check_for_pthread=yes
])

AS_IF([test "$want_pi" = yes], [
    AC_PATH_PROG([P4C_BM], [p4c-bmv2], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$P4C_BM" = x], AC_MSG_ERROR([cannot find p4c-bmv2 compiler]))

    AC_PATH_PROG([PI_GEN_NATIVE_JSON], [pi_gen_native_json], [],
                 [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$PI_GEN_NATIVE_JSON" = x],
          AC_MSG_ERROR([cannot find pi_gen_native_json]))

    AC_PATH_PROG([PI_GEN_PD], [pi_gen_pd], [], [$BIN_DIR$PATH_SEPARATOR$PATH])
    AS_IF([test x"$PI_GEN_PD" = x], AC_MSG_ERROR([cannot find pi_gen_pd]))

    # for now, not possible to make it target-independent, so we generate for
    # tofino
    AS_IF([test "$P4_BUILD_ASSUME_PDFIXED" = ""], [
      AC_CHECK_HEADER([tofino/pdfixed/pd_common.h], [],
        [AC_MSG_ERROR([tofino pdfixed headers not found])])
    ])
])

AX_RESTORE_FLAGS(SAVE_1)

AS_IF([test "$check_for_pthread" = yes], [
    AC_CHECK_LIB([pthread], pthread_create, [],
                 [AC_MSG_ERROR([required library pthread missing])])
])

# Checks for typedefs, structures, and compiler characteristics.
# introduced in autoconf 2.69
# AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_LANG_POP(C++)

AC_PROG_SED

AC_SUBST([MY_PYTHONPATH], ["$PYTHON_DIR"])

# Generate makefiles
AC_CONFIG_FILES([Makefile
                 bmv2/Makefile
                 bmv2/tests/Makefile
                 bmv2/tests/pdtest/Makefile
                 bmv2/tests/openflow/Makefile
                 bmv2-tofino/Makefile
                 tofino/Makefile
                 pi/Makefile])

# Generate other files
AC_CONFIG_FILES([tools/produce_deps.py], [chmod +x tools/produce_deps.py])
AC_CONFIG_FILES([tools/produce_deps_tofino.sh],
                [chmod +x tools/produce_deps_tofino.sh])

AC_PROG_LN_S
AS_IF([! test -e bmpd], [$LN_S bmv2 bmpd])
AS_IF([! test -e tofinobmpd], [$LN_S bmv2-tofino tofinobmpd])
AS_IF([! test -e tofinopd], [$LN_S tofino tofinopd])
AS_IF([! test -e pipd], [$LN_S tofino pipd])

AC_OUTPUT

AS_ECHO("")
AS_ECHO("p4-build features recap ......................")
AS_ECHO("P4 Compiler ................... : $P4C version: $P4C_VERSION")
AS_ECHO("P4_PATH ....................... : $P4_PATH")
AS_ECHO("P4_NAME ....................... : $P4_NAME")
AS_ECHO("P4_PREFIX ..................... : $P4_PREFIX")
AS_ECHO("Compile for bmv2 .............. : $want_bmv2")
AS_ECHO("Compile for tofino ............ : $want_tofino")
AS_ECHO("Compile for tofinobm .......... : $want_bmv2tofino")
AS_ECHO("Compile for PI ................ : $want_pi")
AS_ECHO("Thrift enabled ................ : $enable_thrift")
