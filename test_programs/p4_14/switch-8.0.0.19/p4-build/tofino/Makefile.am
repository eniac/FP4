ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS) -I m4

AM_CPPFLAGS = -I$(FULL_INC_DIR)

# TODO(antonin):
# this is not portable (GNU make extension), but this is the only think I can
# think of right now to keep track of dependencies
# anyone with a better solution is welcome to change this
refdeps := $(shell ../tools/produce_deps_tofino.sh @P4_PATH@)

if WITH_P4C_V6
AM_P4FLAGS  = -x p4-14
AM_P4FLAGS += --target tofino-v1model-barefoot
AM_P4FLAGS += -o $(builddir)/$(P4_NAME)
AM_P4FLAGS += $(P4FLAGS)

COMPILER_CONTEXT = @P4_NAME@/context.json
COMPILER_NAMES =
COMPILER_OUTPUT_FILES = \
	@P4_NAME@/tofino.bin \
	$(COMPILER_CONTEXT) \
	$(COMPILER_NAMES) \
	@P4_NAME@/cli/xml/pd_@P4_NAME@.xml
else
AM_P4FLAGS = --verbose 2 --o $(builddir)/$(P4_NAME)
AM_P4FLAGS += --p4-name=$(P4_NAME)
AM_P4FLAGS += -S
AM_P4FLAGS += --jobs $(P4JOBS)
AM_P4FLAGS += --p4-prefix=$(P4_PREFIX) $(P4FLAGS)
COMPILER_CONTEXT = @P4_NAME@/context/context.json
COMPILER_NAMES = @P4_NAME@/context/p4_name_lookup.json
COMPILER_OUTPUT_FILES = \
	@P4_NAME@/tofino.bin \
	$(COMPILER_CONTEXT) \
	$(COMPILER_NAMES) \
	@P4_NAME@/context/deparser.context.json \
	@P4_NAME@/context/phv.context.json \
	@P4_NAME@/context/parser.context.json \
	@P4_NAME@/cli/xml/pd_@P4_NAME@.xml
endif

AM_PDFLAGS = $(PDFLAGS)
AM_PDFLAGS += --p4-name=$(P4_NAME)
AM_PDFLAGS += --p4-prefix=$(P4_PREFIX)

AM_P4PPFLAGS = $(P4PPFLAGS)

pddatadir = $(datadir)/tofinopd/@P4_NAME@
pddata_DATA = \
	@builddir@/@P4_NAME@/tofino.bin \
	$(COMPILER_CONTEXT) $(COMPILER_NAMES)

clixmldir = $(datadir)/cli/xml
clixml_DATA = $(builddir)/@P4_NAME@/cli/xml/pd_@P4_NAME@.xml

pdlibdir = $(libdir)/tofinopd/@P4_NAME@

pdlib_LTLIBRARIES = libpd.la libpdcli.la

if IS_THRIFT_ENABLED
pdlib_LTLIBRARIES += libpdthrift.la
endif

if IS_ASAN_ENABLED
ASAN_FLAGS=-fsanitize=address -fsanitize=undefined -fsanitize-recover=address,undefined
else !IS_ASAN_ENABLED
ASAN_FLAGS=
endif

# This sequence of CFLAGS is important so that all header files are first
# searched in the local package before installed locations (like @prefix@). At
# the same time, we want it to use header files from @prefix@ if they have been
# installed there by other packages (like bf-drivers).
libpd_la_CPPFLAGS = -I$(builddir)/@P4_NAME@
libpd_la_CPPFLAGS += $(AM_CPPFLAGS)
libpd_la_CFLAGS = $(AM_CFLAGS) $(ASAN_FLAGS)
libpd_la_SOURCES = \
@P4_NAME@/src/pd.c

libpdcli_la_CPPFLAGS = -I$(builddir)/@P4_NAME@
libpdcli_la_CPPFLAGS += $(AM_CPPFLAGS)
libpdcli_la_CFLAGS = $(AM_CFLAGS) $(ASAN_FLAGS)
libpdcli_la_SOURCES = \
@P4_NAME@/src/pdcli.c

if TOFINO_LINK_PROFILER
libpd_la_LIBADD = -lprofiler
libpdcli_la_LIBADD = -lprofiler
endif

libpd_includedir = $(includedir)/tofinopd/@P4_NAME@/pd
libpd_include_HEADERS = @P4_NAME@/pd/pd.h

pdthrift_server_sources = \
@P4_NAME@/thrift-src/bfn_pd_rpc_server.cpp \
@P4_NAME@/thrift-src/bfn_pd_rpc_server.h \
@P4_NAME@/thrift-src/p4_pd_rpc_server.ipp

if IS_THRIFT_ENABLED
libpdthrift_la_CPPFLAGS = -I$(builddir)/$(P4_NAME)
libpdthrift_la_CPPFLAGS += -I$(builddir)/$(P4_NAME)/gen-cpp
libpdthrift_la_CPPFLAGS += $(AM_CPPFLAGS)
libpdthrift_la_CXXFLAGS = -fno-var-tracking-assignments $(AM_CXXFLAGS) $(ASAN_FLAGS)
libpdthrift_la_SOURCES = \
@P4_NAME@/gen-cpp/p4_prefix.h \
@P4_NAME@/gen-cpp/p4_prefix0.cpp \
@P4_NAME@/gen-cpp/p4_prefix1.cpp \
@P4_NAME@/gen-cpp/p4_prefix2.cpp \
@P4_NAME@/gen-cpp/p4_prefix3.cpp \
@P4_NAME@/gen-cpp/p4_prefix4.cpp \
@P4_NAME@/gen-cpp/p4_prefix5.cpp \
@P4_NAME@/gen-cpp/p4_prefix6.cpp \
@P4_NAME@/gen-cpp/p4_prefix7.cpp \
@P4_NAME@/gen-cpp/p4_pd_rpc_constants.h \
@P4_NAME@/gen-cpp/p4_pd_rpc_types.h \
@P4_NAME@/gen-cpp/res_constants.h \
@P4_NAME@/gen-cpp/res_types.h\
@P4_NAME@/gen-cpp/p4_pd_rpc_constants.cpp \
@P4_NAME@/gen-cpp/p4_pd_rpc_types.cpp \
@P4_NAME@/gen-cpp/res_constants.cpp \
@P4_NAME@/gen-cpp/res_types.cpp \
$(pdthrift_server_sources)

pd_python_sources = \
@builddir@/@P4_NAME@/gen-py/p4_pd_rpc/constants.py \
@builddir@/@P4_NAME@/gen-py/p4_pd_rpc/ttypes.py \
@builddir@/@P4_NAME@/gen-py/p4_pd_rpc/__init__.py
endif

if IS_THRIFT_ENABLED
pdthriftpythondir = @pythondir@/tofinopd/@P4_NAME@/p4_pd_rpc
pdthriftpython_PYTHON = $(pd_python_sources) \
@builddir@/@P4_NAME@/gen-py/p4_pd_rpc/$(P4_PREFIX).py

extrapythondir = @pythondir@/tofinopd/@P4_NAME@
extrapython_PYTHON = @builddir@/@P4_NAME@/gen-py/__init__.py
endif

# Adding all compiler output files here so that they are cleaned up with make
# clean.
BUILT_SOURCES = \
	$(libpd_la_SOURCES) \
	$(libpdcli_la_SOURCES) \
	$(COMPILER_OUTPUT_FILES)
if IS_THRIFT_ENABLED
BUILT_SOURCES += $(builddir)/$(P4_NAME)/thrift/p4_pd_rpc.thrift
BUILT_SOURCES += $(libpdthrift_la_SOURCES)
BUILT_SOURCES += $(pd_python_sources)
endif

# The recipe to invoke the p4c-tofino compiler is copied from:
# http://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
p4c.ts : $(P4_PATH) $(refdeps)
	@rm -f p4c.tmp
	@touch p4c.tmp
if WITH_P4C_V6
	$(P4C) $(AM_P4FLAGS) $(AM_P4PPFLAGS) $(P4_PATH)
else
	PYTHONPATH=$(MY_PYTHONPATH):$$PYTHONPATH $(P4C) $(AM_P4FLAGS) $(AM_P4PPFLAGS) $(P4_PATH)
endif
	PYTHONPATH=$(MY_PYTHONPATH):$$PYTHONPATH $(PDGEN) $(builddir)/$(COMPILER_CONTEXT) $(AM_PDFLAGS) -o $(builddir)/$(P4_NAME)
	PYTHONPATH=$(MY_PYTHONPATH):$$PYTHONPATH $(PDCLI) $(builddir)/$(P4_NAME)/cli/pd.json -po $(builddir)/$(P4_NAME)/src -xo $(builddir)/$(P4_NAME)/cli/xml -xd $(datadir)/cli/xml -ll $(pdlibdir)
if IS_THRIFT_ENABLED
	thrift -r --gen cpp -o $(builddir)/$(P4_NAME) $(builddir)/$(P4_NAME)/thrift/p4_pd_rpc.thrift
	mv -f $(builddir)/$(P4_NAME)/gen-cpp/$(P4_PREFIX).h $(builddir)/$(P4_NAME)/gen-cpp/p4_prefix.h
	sed --in-place 's/include "$(P4_PREFIX).h"/include "p4_prefix.h"/' $(builddir)/$(P4_NAME)/gen-cpp/$(P4_PREFIX).cpp
	python $(top_srcdir)/tools/split_pd_thrift.py $(builddir)/$(P4_NAME)/gen-cpp/$(P4_PREFIX).cpp $(builddir)/$(P4_NAME)/gen-cpp/ 8
	thrift -r --gen py -o $(builddir)/$(P4_NAME) $(builddir)/$(P4_NAME)/thrift/p4_pd_rpc.thrift
endif
	@mv -f p4c.tmp $@

$(BUILT_SOURCES) : p4c.ts
## Recover from the removal of $@
	@if test -f $@; then :; else \
	  trap 'rm -rf p4c.lock p4c.ts' 1 2 13 15; \
## mkdir is a portable test-and-set
	if mkdir p4c.lock 2>/dev/null; then \
## This code is being executed by the first process.
	  rm -f p4c.ts; \
	  $(MAKE) $(AM_MAKEFLAGS) p4c.ts; \
	  result=$$?; rm -rf p4c.lock; exit $$result; \
	else \
## This code is being executed by the follower processes.
## Wait until the first process is done.
	  while test -d p4c.lock; do sleep 1; done; \
## Succeed if and only if the first process succeeded.
	    test -f p4c.ts; \
	  fi; \
	fi

CLEANFILES = $(BUILT_SOURCES) p4c.ts
