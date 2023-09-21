# Define a few important files and directories.
FTBENCH_DIR = $(TOP_DIR)/src/tools/ftbench
FTBENCH_SRC = $(FTBENCH_DIR)/ftbench.c
FTBENCH_OBJ = $(OBJ_DIR)/bench.$(SO)
FTBENCH_BIN = $(OBJ_DIR)/bench$E
INCLUDES = $(TOP_DIR)/include
FONTS = $(wildcard $(FTBENCH_DIR)/fonts/*.ttf)

# Define objects.
BASELINE_DIR = $(OBJ_DIR)/baseline/
BENCHMARK_DIR = $(OBJ_DIR)/benchmark/
BASELINE_INFO = $(BASELINE_DIR)info.txt
BENCHMARK_INFO = $(BENCHMARK_DIR)info.txt
HTMLCREATOR_SRC = $(FTBENCH_DIR)/src/tohtml.py
HTMLCREATOR = $(OBJ_DIR)/tohtml.py
HTMLFILE = $(OBJ_DIR)/benchmark.html

# Define flags, create default values in case of not inputted by user.
FTBENCH_FLAG ?= -c 1000 -w 100

# Define all test fonts in the fonts folder.
BASELINE = $(addprefix $(BASELINE_DIR), $(notdir $(FONTS:.ttf=.txt)))
BENCHMARK = $(addprefix $(BENCHMARK_DIR), $(notdir $(FONTS:.ttf=.txt)))

# Define including stuff to commit ftbench.c
FT_INCLUDES := $(OBJ_BUILD) \
                 $(INCLUDES)

# Define libraries to compile ftbench.c
FTLIB := $(LIB_DIR)/$(LIBRARY).$A

# The way of compiling ftbench.c
COMPILE = $(CC) $(ANSIFLAGS) \
                  $(INCLUDES:%=$I%) \
                  $(CFLAGS)

INCLUDES := $(subst /,$(COMPILER_SEP),$(FT_INCLUDES))

# Enable C99 for gcc to avoid warnings.
# Note that clang++ aborts with an error if we use `-std=C99',
# so check for `++' in $(CC) also.
ifneq ($(findstring -pedantic,$(COMPILE)),)
    ifeq ($(findstring ++,$(CC)),)
      COMPILE += -std=c99
    endif
endif

# Decide the way of compile ftbench.c regarding to platform
ifeq ($(PLATFORM),unix)
	# `LDFLAGS` comes from the `configure` script (via FreeType's
	# `builds/unix/unix-cc.mk`), holding all linker flags necessary to
	# link the FreeType library.
    LINK_CMD    = $(LIBTOOL) --mode=link $(CCraw) \
                  $(subst /,$(COMPILER_SEP),$(LDFLAGS))
    LINK_LIBS   = $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE))
else
    LINK_CMD = $(CC) $(subst /,$(COMPILER_SEP),$(LDFLAGS))
    ifeq ($(PLATFORM),unixdev)
		# For the pure `make` call (without using `configure`) we have to add
		# all needed libraries manually.
      LINK_LIBS := $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE)) \
                   -lm -lrt -lz -lbz2 -lpthread
      LINK_LIBS += $(shell pkg-config --libs libpng)
      LINK_LIBS += $(shell pkg-config --libs harfbuzz)
      LINK_LIBS += $(shell pkg-config --libs libbrotlidec)
      LINK_LIBS += $(shell pkg-config --libs librsvg-2.0)
    else
      LINK_LIBS = $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE))
    endif
endif

# Only on Windows we might fall back on GDI+ for PNG saving
ifeq ($(OS),Windows_NT)
    LINK_LIBS += -lgdiplus
endif

####################################################################
#
# POSIX TERMIOS: Do not define if you use OLD U*ix like 4.2BSD.
#
ifeq ($(PLATFORM),unix)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
endif

ifeq ($(PLATFORM),unixdev)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
endif


# Create directories for baseline and benchmark
$(BASELINE_DIR) $(BENCHMARK_DIR):
	@mkdir -p $@

# Create ftbench object
$(FTBENCH_OBJ): $(FTBENCH_SRC)
	@$(COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<) $(EXTRAFLAGS)
	@echo "Object created."

# Build ftbench
$(FTBENCH_BIN): $(FTBENCH_OBJ)
	@echo "Linking ftbench..."
	@$(LINK_CMD) $T$(subst /,$(COMPILER_SEP),$@ $<) $(LINK_LIBS)
	@echo "Built."

# Copy tohtml.py script into objs folder
.PHONY: copy-html-script
copy-html-script:
	@cp $(HTMLCREATOR_SRC) $(OBJ_DIR)
	@echo "Copied tohtml.py to $(OBJ_DIR)"

####################################################################
#
# Create Baseline:
#	Flags, commit hash, commit date, branch name infos printed into info.txt
#	All fonts are tested by ftbench, results printed to related .txt
#
.PHONY: baseline
baseline: $(FTBENCH_BIN) $(BASELINE_DIR)
	@$(RM) -f $(BASELINE)
	@echo "Creating baseline..."
	@echo "$(FTBENCH_FLAG)" > $(BASELINE_INFO)
	@echo "`git -C $(TOP_DIR) rev-parse HEAD`" >> $(BASELINE_INFO)
	@echo "`git -C $(TOP_DIR) show -s --format=%ci HEAD`" >> $(BASELINE_INFO)
	@echo "`git -C $(TOP_DIR) rev-parse --abbrev-ref HEAD`" >> $(BASELINE_INFO)
	@fonts=($(FONTS)); \
	total_fonts=$${#fonts[@]}; \
	step=0; \
	for font in $${fonts[@]}; do \
		step=$$((step+1)); \
		percent=$$((step * 100 / total_fonts)); \
		printf "\nProcessing %d%%..." $$percent; \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) "$$font" > $(BASELINE_DIR)$$(basename $$font .ttf).txt; \
	done
	@echo "Baseline created."

####################################################################
#
# Create Benchmark:
#	Flags, commit hash, commit date, branch name infos printed into info.txt
#	All fonts are tested by ftbench, results printed to related .txt
#	Result page is created by tohtml.py
#
.PHONY: benchmark
benchmark: $(FTBENCH_BIN) $(BENCHMARK_DIR) copy-html-script
	@$(RM) -f $(BENCHMARK) $(HTMLFILE)
	@echo "Creating benchmark..."
	@echo "$(FTBENCH_FLAG)" > $(BENCHMARK_INFO)
	@echo "`git -C $(TOP_DIR) rev-parse HEAD`" >> $(BENCHMARK_INFO)
	@echo "`git -C $(TOP_DIR) show -s --format=%ci HEAD`" >> $(BENCHMARK_INFO)
	@echo "`git -C $(TOP_DIR) rev-parse --abbrev-ref HEAD`" >> $(BENCHMARK_INFO)
	@fonts=($(FONTS)); \
	total_fonts=$${#fonts[@]}; \
	step=0; \
	for font in $${fonts[@]}; do \
		step=$$((step+1)); \
		percent=$$((step * 100 / total_fonts)); \
		printf "\nProcessing %d%%..." $$percent; \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) "$$font" > $(BENCHMARK_DIR)$$(basename $$font .ttf).txt; \
	done
	@$(PYTHON) $(HTMLCREATOR) $(OBJ_DIR)
	@echo "Benchmark results created in file: $(HTMLFILE)"

####################################################################
#
# Clean Benchmark:
#	All created stuff is cleaned.
#
.PHONY: clean-benchmark
clean-benchmark:
	@echo "Cleaning..."
	@$(RM) $(FTBENCH_BIN) $(FTBENCH_OBJ)
	@$(RM) -rf $(BASELINE_DIR) $(BENCHMARK_DIR) $(HTMLFILE) $(HTMLCREATOR)
	@echo "Cleaned"
