#CHECK for doxygen
AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen])

#Always distribute these files
AC_CONFIG_FILES([doc/Doxyfile])	
AC_CONFIG_FILES([doc/DoxygenLayout.xml]) 

#Set conditional
AM_CONDITIONAL([have_doxygen], [test -n "$DOXYGEN"])


#Always add doc Makefile
AC_CONFIG_FILES([
	doc/Makefile
])
