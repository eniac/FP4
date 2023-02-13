ACLOCAL_AMFLAGS = $(ACLOCAL_FLAGS) -I m4

p4_targetdir = $(DESTDIR)/$(datadir)/p4/targets
p4_target_DATA =
install-data-hook:
	mkdir -p $(p4_targetdir)/tofino; \
	for target in $(P4PROGRAMS); do \
	  if [[ ! -e @srcdir@/tofino/$${target}.conf.in ]]; then \
	    target_conf_filename=$(p4_targetdir)/tofino/$${target}.conf; \
	    cp @builddir@/tofino/tofino_single_device.conf $$target_conf_filename; \
	    sed -i "s/TOFINO_SINGLE_DEVICE/$$target/" $$target_conf_filename; \
	  fi \
	done; \
	cp $(builddir)/tofino/*.conf $(p4_targetdir)/tofino/;
if IS_THRIFT_ENABLED
	:
else
	sed --in-place 's/^.*libpdthrift.so\", $$//' $(p4_targetdir)/tofino/*.conf
endif
uninstall-hook:
	for target in $(P4PROGRAMS); do \
	  rm -f $(p4_targetdir)/tofino/$${target}.conf; \
	done
