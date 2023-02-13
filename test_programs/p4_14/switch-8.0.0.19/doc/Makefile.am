if have_doxygen
doc:
	@$(DOXYGEN) Doxyfile
	@echo  The documentation is under: $(builddir)/doc/
else
doc:
	@echo "ERROR: unable to generate documentation; DOXYGEN not found!"
	@exit -1
endif
