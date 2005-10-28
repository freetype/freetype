# this sub-Makefile is used to compute the list of exported symbols whenever
# the EXPORTS_LIST variable is defined by one of the platform or compiler
# specific build files
#
# EXPORTS_LIST contains the name of the "list" file, which can be a Windows
# .DEF file by the way
#
ifneq ($(EXPORTS_LIST),)

  # CCexe is the compiler used to compile the "apinames" tool program
  # on the host machine. This isn't necessarily the same than the compiler
  # which can be a cross-compiler for a different architecture
  #
  ifeq ($(CCexe),)
    CCexe := $(CC)
  endif

  # TE acts as T, but for executables instead of object files
  ifeq ($(TE),)
    TE := $T
  endif

  # the list of public headers we're going to parse
  PUBLIC_HEADERS := $(wildcard $(PUBLIC_DIR)/*.h)

  # the "apinames" source and executable. We use E as the executable
  # suffix, which *includes* the final dot
  # note that $(APINAMES_OPTIONS) is empty, except for Windows compilers
  #
  APINAMES_SRC := $(TOP_DIR)/src/tools/apinames.c
  APINAMES_EXE := $(OBJ_DIR)/apinames$E

  $(APINAMES_EXE): $(APINAMES_SRC)
	$(CCexe) $(TE)$@ $<

  .PHONY: symbols_list  clean_symbols_list  clean_apinames

  symbols_list: $(EXPORTS_LIST)

  $(EXPORTS_LIST): $(APINAMES_EXE) $(PUBLIC_HEADERS)
	$(subst /,$(SEP),$(APINAMES_EXE)) -o$@ $(APINAMES_OPTIONS) $(PUBLIC_HEADERS)

  $(PROJECT_LIBRARY): $(EXPORTS_LIST)

  clean_symbols_list:
	-$(DELETE) $(subst /,$(SEP),$(EXPORTS_LIST))

  clean_apinames:
	-$(DELETE) $(subst /,$(SEP),$(APINAMES_EXE))

  clean_project: clean_symbols_list clean_apinames

endif