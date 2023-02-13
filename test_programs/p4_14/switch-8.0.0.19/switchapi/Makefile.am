ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_P4PPFLAGS = $(P4PPFLAGS)

AM_CPPFLAGS += -I/usr/include/libnl3
AM_CPPFLAGS += -I$(srcdir)/include
AM_CPPFLAGS += -I$(builddir)/src/gen-cpp
AM_CPPFLAGS += -I$(top_srcdir)/p4src/includes

AM_CFLAGS += $(SWITCH_CFLAGS)

# TODO(antonin): check if belongs in CFLAGS or CPPFLAGS
AM_CFLAGS += $(AM_P4PPFLAGS)

if COVERAGE
LDFLAGS += -lgcov
endif

if IS_THRIFT_ENABLED
thrift_cpp = \
@builddir@/src/gen-cpp/switch_api_constants.cpp \
@builddir@/src/gen-cpp/switch_api_constants.h \
@builddir@/src/gen-cpp/switch_api_rpc.cpp \
@builddir@/src/gen-cpp/switch_api_rpc.h \
@builddir@/src/gen-cpp/switch_api_types.cpp \
@builddir@/src/gen-cpp/switch_api_types.h
thrift_py = \
@builddir@/gen-py/switch_api/constants.py \
@builddir@/gen-py/switch_api/__init__.py \
@builddir@/gen-py/switch_api/switch_api_rpc.py \
@builddir@/gen-py/switch_api/switch_api_rpc-remote \
@builddir@/gen-py/switch_api/ttypes.py

switchapi_ctypesgen = @builddir@/gen-py/switch_api/switch_api_headers.py

switchapidir = $(pythondir)/switchapi_thrift
switchapi_PYTHON = \
$(thrift_py) \
$(switchapi_ctypesgen)
endif

BUILT_SOURCES = \
$(thrift_cpp) \
$(thrift_py) \
$(switchapi_ctypesgen)

CTYPESGEN = $(top_srcdir)/third-party/ctypesgen/ctypesgen.py

switchapi_sources = \
src/switch_acl.c \
src/switch_acl_dump.c \
src/switch_acl_int.h \
src/switch_api.thrift \
src/switch_buffer.c \
src/switch_buffer_dump.c \
src/switch_buffer_int.h \
src/switch_config.c \
src/switch_config_dump.c \
src/switch_config_int.h \
src/switch_cli_int.h \
src/switch_failover.c \
src/switch_failover_int.h \
src/switch_handle.c \
src/switch_handle_dump.c \
src/switch_handle_int.h \
src/switch_hash.c \
src/switch_hash_int.h \
src/switch_hostif.c \
src/switch_hostif_dump.c \
src/switch_hostif_int.h \
src/switch_id.c \
src/switch_ila.c \
src/switch_ila_int.h \
src/switch_init.c \
src/switch_dtel_INT.c \
src/switch_dtel.c \
src/switch_dtel_int.h \
src/switch_dtel_postcard.c \
src/switch_dtel_mod.c \
src/switch_interface.c \
src/switch_interface_dump.c \
src/switch_interface_int.h \
src/switch_internal.h \
src/switch_l2.c \
src/switch_l2_dump.c \
src/switch_l2_int.h \
src/switch_l3.c \
src/switch_l3_dump.c \
src/switch_l3_int.h \
src/switch_label.c \
src/switch_label_int.h \
src/switch_lag.c \
src/switch_lag_dump.c \
src/switch_lag_int.h \
src/switch_log.c \
src/switch_log_int.h \
src/switch_lpm.c \
src/switch_lpm_int.h \
src/switch_mcast.c \
src/switch_mcast_int.h \
src/switch_mirror.c \
src/switch_mirror_dump.c \
src/switch_mirror_int.h \
src/switch_mpls.c \
src/switch_mpls_int.h \
src/switch_nat.c \
src/switch_nat_int.h \
src/switch_neighbor.c \
src/switch_neighbor_int.h \
src/switch_neighbor_dump.c \
src/switch_nhop.c \
src/switch_nhop_int.h \
src/switch_nhop_dump.c \
src/switch_packet.c \
src/switch_packet_dump.c \
src/switch_packet_int.h \
src/switch_pd.c \
src/switch_pd.h \
src/switch_pd_acl.c \
src/switch_pd_bd.c \
src/switch_pd_buffer.c \
src/switch_pd_hash.c \
src/switch_pd_ila.c \
src/switch_pd_l2.c \
src/switch_pd_l3.c \
src/switch_pd_lag.c \
src/switch_pd_meter.c \
src/switch_pd_mirror.c \
src/switch_pd_neighbor.c \
src/switch_pd_nhop.c \
src/switch_pd_ppg.c \
src/switch_pd_port.c \
src/switch_pd_qos.c \
src/switch_pd_queue.c \
src/switch_pd_rmac.c \
src/switch_pd_mcast.c \
src/switch_pd_mpls.c \
src/switch_pd_nat.c \
src/switch_pd_types.h \
src/switch_pd_dtel_INT.c \
src/switch_pd_dtel_postcard.c \
src/switch_pd_dtel_mod.c \
src/switch_pd_dtel.c \
src/switch_pd_dtel.h \
src/switch_pd_failover.c \
src/switch_pd_failover.h \
src/switch_pd_sr.c \
src/switch_pd_sflow.c \
src/switch_pd_table.c \
src/switch_pd_tunnel.c \
src/switch_pd_wred.c \
src/switch_pd_vlan.c \
src/switch_port.c \
src/switch_port_dump.c \
src/switch_port_int.h \
src/switch_qos.c \
src/switch_qos_int.h \
src/switch_queue.c \
src/switch_queue_dump.c \
src/switch_queue_int.h \
src/switch_rmac.c \
src/switch_rmac_dump.c \
src/switch_rmac_int.h \
src/switch_rpf.c \
src/switch_rpf_dump.c \
src/switch_rpf_int.h \
src/switch_rif.c \
src/switch_rif_dump.c \
src/switch_rif_int.h \
src/switch_scheduler.c \
src/switch_scheduler_int.h \
src/switch_sr.c \
src/switch_sr_int.h \
src/switch_stp.c \
src/switch_stp_dump.c \
src/switch_stp_int.h \
src/switch_tunnel.c \
src/switch_tunnel_dump.c \
src/switch_tunnel_int.h \
src/switch_utils.c \
src/switch_vlan.c \
src/switch_vlan_dump.c \
src/switch_vlan_int.h \
src/switch_vrf.c \
src/switch_vrf_dump.c \
src/switch_vrf_int.h \
src/switch_meter.c \
src/switch_meter_dump.c \
src/switch_meter_int.h \
src/switch_sflow.c \
src/switch_sflow_int.h \
src/switch_pd_common.c \
src/switch_pd_common.h \
src/switch_table.c \
src/switch_table_dump.c \
src/switch_table_int.h \
src/switch_device.c \
src/switch_device_dump.c \
src/switch_device_int.h \
src/switch_bd.c \
src/switch_bd_dump.c \
src/switch_bd_int.h \
src/switch_ln.c \
src/switch_ln_dump.c \
src/switch_ln_int.h \
src/thrift_cache.h \
src/switch_bfd.c \
src/switch_bfd_int.h \
src/switch_pd_bfd.c \
src/switch_pd_bfd.h \
src/switch_pd_pktgen.c \
src/switch_pd_pktgen.h \
src/switch_wred_int.h \
src/switch_wred.c \
src/switch_cli_int.h

switchapiincdir = $(includedir)/switchapi
switchapiinc_HEADERS = \
include/switchapi/switch_acl.h \
include/switchapi/switch_base_types.h \
include/switchapi/switch_bfd.h \
include/switchapi/switch_bd.h \
include/switchapi/switch_buffer.h \
include/switchapi/switch_config.h \
include/switchapi/switch_dox.h \
include/switchapi/switch_device.h \
include/switchapi/switch_failover.h \
include/switchapi/switch_handle.h \
include/switchapi/switch_hash.h \
include/switchapi/switch_hostif.h \
include/switchapi/switch_id.h \
include/switchapi/switch_ila.h \
include/switchapi/switch_interface.h \
include/switchapi/switch_dtel.h \
include/switchapi/switch_l2.h \
include/switchapi/switch_l3.h \
include/switchapi/switch_label.h \
include/switchapi/switch_lag.h \
include/switchapi/switch_ln.h \
include/switchapi/switch_log.h \
include/switchapi/switch_meter.h \
include/switchapi/switch_mcast.h \
include/switchapi/switch_mirror.h \
include/switchapi/switch_mpls.h \
include/switchapi/switch_nat.h \
include/switchapi/switch_neighbor.h \
include/switchapi/switch_nhop.h \
include/switchapi/switch_port.h \
include/switchapi/switch_porting.h \
include/switchapi/switch_qos.h \
include/switchapi/switch_queue.h \
include/switchapi/switch_rmac.h \
include/switchapi/switch_rif.h \
include/switchapi/switch_rpf.h \
include/switchapi/switch_scheduler.h \
include/switchapi/switch_sflow.h \
include/switchapi/switch_status.h \
include/switchapi/switch_stp.h \
include/switchapi/switch_table.h \
include/switchapi/switch_tunnel.h \
include/switchapi/switch_utils.h \
include/switchapi/switch_vlan.h \
include/switchapi/switch_vrf.h \
include/switchapi/switch_wred.h \
include/switchapi/switch.x

switchapi_test_sources = \
tests/switch_interface_test.c \
tests/switch_l2_test.c \
tests/switch_l3_test.c \
tests/switch_rmac_test.c \
tests/switch_rpf_test.c \
tests/switch_vlan_test.c \
tests/switch_vrf_test.c \
tests/switch_meter_test.c \
tests/switch_test.h \
tests/switch_test.c

if IS_THRIFT_ENABLED
switchapi_sources  += $(thrift_cpp) src/switch_api_rpc_server.cpp
endif

$(switchapi_ctypesgen) : $(thrift_py) $(switchapiinc_HEADERS)
	@echo "Running ctypesgen on switchapi include files"
	@$(CTYPESGEN) -I/usr/include -I$(top_srcdir)/p4src/includes \
		-I$(includedir) --no-macros \
		$(srcdir)/include/switchapi/*.h -o $@ > /dev/null 2>&1

# The recipe to invoke the thrift compiler is copied from:
# http://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html
switchapi.ts : @srcdir@/src/switch_api.thrift
	@rm -f switchapi.tmp
	@touch switchapi.tmp
	$(THRIFT) -o @builddir@/src/ --gen cpp -r @srcdir@/src/switch_api.thrift
	$(THRIFT) -o @builddir@/ --gen py -r @srcdir@/src/switch_api.thrift
	@mv -f switchapi.tmp $@

if IS_THRIFT_ENABLED
$(thrift_cpp) $(thrift_py) : switchapi.ts
## Recover from the removal of $@
	@if test -f $@; then :; else \
	  trap 'rm -rf switchapi.lock switchapi.ts' 1 2 13 15; \
## mkdir is a portable test-and-set
	if mkdir switchapi.lock 2>/dev/null; then \
## This code is being executed by the first process.
	  rm -f switchapi.ts; \
	  $(MAKE) $(AM_MAKEFLAGS) switchapi.ts; \
	  result=$$?; rm -rf switchapi.lock; exit $$result; \
	else \
## This code is being executed by the follower processes.
## Wait until the first process is done.
	  while test -d switchapi.lock; do sleep 1; done; \
## Succeed if and only if the first process succeeded.
	    test -f switchapi.ts; \
	  fi; \
	fi


CLEANFILES = $(BUILT_SOURCES) switchapi.ts
endif

lib_LTLIBRARIES =
cpp_flags =

if WITH_BMV2
libbmswitchapi_la_CPPFLAGS = $(BMV2_CPPFLAGS)
libbmswitchapi_la_CPPFLAGS += -I$(top_builddir)/p4-build $(AM_CPPFLAGS)
libbmswitchapi_la_SOURCES = $(switchapi_sources) $(switchapiinc_HEADERS)
libbmswitchapi_la_LIBADD = -lbfutils $(THRIFT_LIB)
libbmswitchapi_la_LIBADD += $(NETLINK_LIBS)
lib_LTLIBRARIES += libbmswitchapi.la
cpp_flags = $(libbmswitchapi_la_CPPFLAGS)
endif

if WITH_TOFINO
libswitchapi_la_CPPFLAGS = $(TOFINO_CPPFLAGS)
libswitchapi_la_CPPFLAGS += -I$(top_builddir)/p4-build $(AM_CPPFLAGS)
libswitchapi_la_SOURCES = $(switchapi_sources) $(switchapiinc_HEADERS)
libswitchapi_la_LIBADD = -lbfutils $(THRIFT_LIB)
libswitchapi_la_LIBADD += $(NETLINK_LIBS)
lib_LTLIBRARIES += libswitchapi.la
cpp_flags = $(libswitchapi_la_CPPFLAGS)
endif

if WITH_BMV2TOFINO
libtofinobmswitchapi_la_CPPFLAGS = $(BMV2TOFINO_CPPFLAGS)
libtofinobmswitchapi_la_CPPFLAGS += -I$(top_builddir)/p4-build $(AM_CPPFLAGS)
libtofinobmswitchapi_la_SOURCES = $(switchapi_sources) $(switchapiinc_HEADERS)
libtofinobmswitchapi_la_LIBADD = -lbfutils $(THRIFT_LIB)
libtofinobmswitchapi_la_LIBADD += $(NETLINK_LIBS)
lib_LTLIBRARIES += libtofinobmswitchapi.la
cpp_flags = $(libtofinobmswitchapi_la_CPPFLAGS)
endif

switchapi_xmldir = $(datadir)/cli/xml
switchapi_xml_DATA = xml/switchapi.xml

lib_LTLIBRARIES += bfshell_plugin_switchapi.la
bfshell_plugin_switchapi_la_CFLAGS = $(AM_CFLAGS)
bfshell_plugin_switchapi_la_CPPFLAGS = $(cpp_flags)
bfshell_plugin_switchapi_la_LDFLAGS = -module
bfshell_plugin_switchapi_la_SOURCES = src/switch_cli.c

lib_LTLIBRARIES += libswitchapitest.la
libswitchapitest_la_CFLAGS = $(AM_CFLAGS)
libswitchapitest_la_LDFLAGS = -module
libswitchapitest_la_SOURCES = $(switchapi_test_sources)
