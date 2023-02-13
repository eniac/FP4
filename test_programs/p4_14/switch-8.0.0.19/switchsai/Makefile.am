ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_CPPFLAGS += -I$(srcdir)/submodules/SAI/inc
AM_CPPFLAGS += -I$(builddir)/src/gen-cpp
AM_CPPFLAGS += -I$(top_srcdir)/switchapi/include
AM_CPPFLAGS += -I$(top_srcdir)/p4src
AM_CPPFLAGS += -I$(top_srcdir)/p4src/includes
# this flag needs to be last
AM_CPPFLAGS += -I$(includedir)

AM_CFLAGS += $(SWITCH_CFLAGS)

if IS_THRIFT_ENABLED
thrift_cpp_sources = \
@builddir@/src/gen-cpp/switch_sai_constants.cpp \
@builddir@/src/gen-cpp/switch_sai_constants.h \
@builddir@/src/gen-cpp/switch_sai_rpc.cpp \
@builddir@/src/gen-cpp/switch_sai_rpc.h \
@builddir@/src/gen-cpp/switch_sai_types.cpp \
@builddir@/src/gen-cpp/switch_sai_types.h
thrift_py_sources = \
@builddir@/src/gen-py/switch_sai/constants.py \
@builddir@/src/gen-py/switch_sai/__init__.py \
@builddir@/src/gen-py/switch_sai/switch_sai_rpc.py \
@builddir@/src/gen-py/switch_sai/switch_sai_rpc-remote \
@builddir@/src/gen-py/switch_sai/ttypes.py

sai_ctypesgen = @builddir@/src/gen-py/switch_sai/sai_headers.py

switchsaidir = $(pythondir)/switchsai_thrift
switchsai_PYTHON = \
$(thrift_py_sources) \
$(sai_ctypesgen)
endif

switchsai_sources =      \
src/sai.c                \
src/sai_bflib.c          \
src/saiacl.c             \
src/saiapi.h             \
src/saibridge.c	         \
src/saibuffer.c          \
src/saifdb.c             \
src/saihash.c            \
src/saihostif.c          \
src/saiinternal.h        \
src/saiipmc.c            \
src/sail2mc.c            \
src/sailag.c             \
src/saimirror.c          \
src/saineighbor.c        \
src/sainexthop.c         \
src/sainexthopgroup.c    \
src/saiobject.c          \
src/saipolicer.c         \
src/saiport.c            \
src/saiqosmap.c          \
src/saiqueue.c           \
src/sairoute.c           \
src/saivirtualrouter.c   \
src/sairouterinterface.c \
src/saischeduler.c       \
src/saischedulergroup.c  \
src/saistp.c             \
src/saiswitch.c          \
src/saitunnel.c          \
src/saidtel.c            \
src/saiudf.c             \
src/saiutils.c           \
src/saivlan.c            \
src/saiwred.c

switchsaiincdir = $(includedir)/switchsai
switchsaiinc_HEADERS =                  \
submodules/SAI/inc/sai.h                \
submodules/SAI/inc/saiacl.h             \
submodules/SAI/inc/saibridge.h          \
submodules/SAI/inc/saibuffer.h          \
submodules/SAI/inc/saifdb.h             \
submodules/SAI/inc/saihash.h            \
submodules/SAI/inc/saihostif.h          \
submodules/SAI/inc/saiipmc.h            \
submodules/SAI/inc/sail2mc.h            \
submodules/SAI/inc/sailag.h             \
submodules/SAI/inc/saimirror.h          \
submodules/SAI/inc/saineighbor.h        \
submodules/SAI/inc/sainexthop.h         \
submodules/SAI/inc/sainexthopgroup.h    \
submodules/SAI/inc/saiobject.h          \
submodules/SAI/inc/saipolicer.h         \
submodules/SAI/inc/saiport.h            \
submodules/SAI/inc/saiqosmap.h          \
submodules/SAI/inc/saiqueue.h           \
submodules/SAI/inc/sairoute.h           \
submodules/SAI/inc/saivirtualrouter.h   \
submodules/SAI/inc/sairouterinterface.h \
submodules/SAI/inc/saisamplepacket.h    \
submodules/SAI/inc/saischeduler.h       \
submodules/SAI/inc/saischedulergroup.h  \
submodules/SAI/inc/saistatus.h          \
submodules/SAI/inc/saistp.h             \
submodules/SAI/inc/saiswitch.h          \
submodules/SAI/inc/saitunnel.h          \
submodules/SAI/inc/saitypes.h           \
submodules/SAI/inc/saiudf.h             \
submodules/SAI/inc/saivlan.h            \
submodules/SAI/inc/saiwred.h            \
submodules/SAI/inc/sairpfgroup.h        \
submodules/SAI/inc/sail2mcgroup.h       \
submodules/SAI/inc/saiipmcgroup.h       \
submodules/SAI/inc/saimcastfdb.h        \
submodules/SAI/inc/saidtel.h

if IS_THRIFT_ENABLED
switchsai_sources += $(thrift_cpp_sources) src/switch_sai_rpc_server.cpp

BUILT_SOURCES = \
$(thrift_cpp_sources) \
$(thrift_py_sources) \
$(sai_ctypesgen)
endif

CTYPESGEN = $(top_srcdir)/third-party/ctypesgen/ctypesgen.py

if IS_THRIFT_ENABLED
$(sai_ctypesgen) : $(thrift_py_sources)
	@$(CTYPESGEN) -I/usr/include \
	-I$(srcdir)/submodules/SAI/inc \
	$(srcdir)/submodules/SAI/inc/*.h -o $@ \
	> /dev/null 2>&1


# The recipe to invoke the thrift compiler is copied from:
# http://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
switchsai.ts : @srcdir@/src/switch_sai.thrift
	@rm -f switchsai.tmp
	@touch switchsai.tmp
	$(THRIFT) -o @builddir@/src/ --gen cpp -r @srcdir@/src/switch_sai.thrift
	$(THRIFT) -o @builddir@/src/ --gen py -r @srcdir@/src/switch_sai.thrift
	@mv -f switchsai.tmp $@

$(thrift_cpp_sources) $(thrift_py_sources) : switchsai.ts
## Recover from the removal of $@
	@if test -f $@; then :; else \
	  trap 'rm -rf switchsai.lock switchsai.ts' 1 2 13 15; \
## mkdir is a portable test-and-set
	if mkdir switchsai.lock 2>/dev/null; then \
## This code is being executed by the first process.
	  rm -f switchsai.ts; \
	  $(MAKE) $(AM_MAKEFLAGS) switchsai.ts; \
	  result=$$?; rm -rf switchsai.lock; exit $$result; \
	else \
## This code is being executed by the follower processes.
## Wait until the first process is done.
	  while test -d switchsai.lock; do sleep 1; done; \
## Succeed if and only if the first process succeeded.
	    test -f switchsai.ts; \
	  fi; \
	fi

CLEANFILES = $(BUILT_SOURCES) switchsai.ts
endif

lib_LTLIBRARIES =

if WITH_BMV2
libbmswitchsai_la_CPPFLAGS = $(BMV2_CPPFLAGS)
libbmswitchsai_la_CPPFLAGS += -I$(top_builddir)/p4-build $(AM_CPPFLAGS)
libbmswitchsai_la_SOURCES = $(switchsai_sources)
libbmswitchsai_la_LIBADD = $(THRIFT_LIB)
lib_LTLIBRARIES += libbmswitchsai.la
endif

if WITH_TOFINO
libswitchsai_la_CPPFLAGS = $(TOFINO_CPPFLAGS)
libswitchsai_la_CPPFLAGS += -I$(top_builddir)/p4-build $(AM_CPPFLAGS)
libswitchsai_la_SOURCES = $(switchsai_sources)
libswitchsai_la_LIBADD = $(THRIFT_LIB)
lib_LTLIBRARIES += libswitchsai.la
endif

if WITH_BMV2TOFINO
libtofinobmswitchsai_la_CPPFLAGS = $(BMV2TOFINO_CPPFLAGS)
libtofinobmswitchsai_la_CPPFLAGS += -I$(top_builddir)/p4-build $(AM_CPPFLAGS)
libtofinobmswitchsai_la_SOURCES = $(switchsai_sources)
libtofinobmswitchsai_la_LIBADD = $(THRIFT_LIB)
lib_LTLIBRARIES += libtofinobmswitchsai.la
endif
