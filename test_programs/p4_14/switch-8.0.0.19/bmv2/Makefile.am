ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_CPPFLAGS += -I$(top_builddir)/p4-build -I$(includedir)
AM_CPPFLAGS += $(BMV2_CPPFLAGS)

lib_LTLIBRARIES = libbmswitchp4.la
libbmswitchp4_la_CPPFLAGS = $(AM_CPPFLAGS)
libbmswitchp4_la_LIBADD =
if WITH_SWITCHSAI
libbmswitchp4_la_LIBADD += @top_builddir@/switchsai/libbmswitchsai.la
AM_CPPFLAGS += -DSWITCHSAI_ENABLE
endif
if WITH_SWITCHAPI
libbmswitchp4_la_LIBADD += @top_builddir@/switchapi/libbmswitchapi.la
AM_CPPFLAGS += -DSWITCHAPI_ENABLE
endif
libbmswitchp4_la_LIBADD += \
@top_builddir@/p4-build/bmv2/libpd.la \
@top_builddir@/p4-build/bmv2/libpdthrift.la \
-lbmpdfixed -lbmpdfixedthrift $(THRIFT_LIB)
libbmswitchp4_la_SOURCES = \
bmv2_init.c

bin_PROGRAMS = bmswitchp4_drivers
bmswitchp4_drivers_CPPFLAGS = $(AM_CPPFLAGS)
bmswitchp4_drivers_LDADD = \
libbmswitchp4.la
bmswitchp4_drivers_SOURCES = \
main.c
