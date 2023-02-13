dnl ab_FIND_HEADER_PATH(HEADER, RESULT)
dnl =================================
dnl Find the location of the header file HEADER
dnl and set the result to $RESULT.
dnl WARNING: This may not be very robust, but I believe it works with gcc, clang
dnl Antonin Bas, Barefoot Networks
AC_DEFUN([ab_FIND_HEADER_PATH],
  [SAVED_CPPFLAGS="$CPPFLAGS"
   CPPFLAGS+="$CPPFLAGS -H"
   AC_MSG_CHECKING([for header $1])
   AC_PREPROC_IFELSE(
     [AC_LANG_SOURCE([[#include $1]])],
     [AC_MSG_RESULT([yes])
      _ab_tmp_path=$(cat conftest.err | awk 'NR==1' | awk 'END {print $NF}')],
     [AC_MSG_RESULT([no])])
   CPPFLAGS=$SAVED_CPPFLAGS
   AS_IF([test x"$_ab_tmp_path" = x], [
     :
     $3
   ], [
     $2=`AS_DIRNAME(["$_ab_tmp_path"])`
   ])
])
