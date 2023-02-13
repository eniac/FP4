ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_CPPFLAGS += -I$(top_builddir)/p4-build -I$(includedir)
AM_CPPFLAGS += $(BMV2TOFINO_CPPFLAGS)

lib_LTLIBRARIES = libtofinobmswitchp4.la
libtofinobmswitchp4_la_CPPFLAGS = $(AM_CPPFLAGS)
libtofinobmswitchp4_la_LIBADD =
if WITH_SWITCHSAI
libtofinobmswitchp4_la_LIBADD += @top_builddir@/switchsai/libtofinobmswitchsai.la
AM_CPPFLAGS += -DSWITCHSAI_ENABLE
endif
if WITH_SWITCHAPI
libtofinobmswitchp4_la_LIBADD += @top_builddir@/switchapi/libtofinobmswitchapi.la
AM_CPPFLAGS += -DSWITCHAPI_ENABLE
endif
libtofinobmswitchp4_la_LIBADD += \
@top_builddir@/p4-build/bmv2-tofino/libpd.la \
@top_builddir@/p4-build/bmv2-tofino/libpdthrift.la \
-ltofinobmpdfixed -ltofinobmpdfixedthrift $(THRIFT_LIB)
libtofinobmswitchp4_la_SOURCES = \
bmv2_init.c

bin_PROGRAMS = tofinobmswitchp4_drivers
tofinobmswitchp4_drivers_CPPFLAGS = $(AM_CPPFLAGS)
tofinobmswitchp4_drivers_LDADD = \
libtofinobmswitchp4.la
tofinobmswitchp4_drivers_SOURCES = \
main.c
