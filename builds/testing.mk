# Variables
FTBENCH_DIR = $(TOP_DIR)/src/tools/ftbench
FTBENCH_SRC = $(FTBENCH_DIR)/ftbench.c
FTBENCH_OBJ = $(OBJ_DIR)/bench.$(SO)
FTBENCH_BIN = $(OBJ_DIR)/bench$E
FTBENCH_FLAG ?= -c 750 -w 50
INCLUDES = $(TOP_DIR)/include
FONTS = $(wildcard $(FTBENCH_DIR)/fonts/*.ttf)
BASELINE_DIR = $(OBJ_DIR)/baseline/
BENCHMARK_DIR = $(OBJ_DIR)/benchmark/
BASELINE = $(addprefix $(BASELINE_DIR), $(notdir $(FONTS:.ttf=.txt)))
BENCHMARK = $(addprefix $(BENCHMARK_DIR), $(notdir $(FONTS:.ttf=.txt)))
BASELINE_INFO = $(BASELINE_DIR)info.txt
BENCHMARK_INFO = $(BENCHMARK_DIR)info.txt
HTMLCREATOR = $(FTBENCH_DIR)/src/tohtml.py
HTMLFILE = $(OBJ_DIR)/benchmark.html

FT_INCLUDES := $(OBJ_BUILD) \
                 $(INCLUDES) 

COMPILE = $(CC) $(ANSIFLAGS) \
                  $(INCLUDES:%=$I%) \
                  $(CFLAGS)

ifeq ($(PLATFORM),unix)
    ifdef DEVEL_DIR
      PLATFORM := unixdev
    endif
endif

ifneq ($(findstring -pedantic,$(COMPILE)),)
    ifeq ($(findstring ++,$(CC)),)
      COMPILE += -std=c99
    endif
endif

FTLIB := $(LIB_DIR)/$(LIBRARY).$A

ifeq ($(PLATFORM),unix)
    LINK_CMD    = $(LIBTOOL) --mode=link $(CCraw) \
                  $(subst /,$(COMPILER_SEP),$(LDFLAGS))
    LINK_LIBS   = $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE)) 
else
    LINK_CMD = $(CC) $(subst /,$(COMPILER_SEP),$(LDFLAGS))
    ifeq ($(PLATFORM),unixdev)
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

ifeq ($(OS),Windows_NT)
    LINK_LIBS += -lgdiplus
endif

ifeq ($(PLATFORM),unix)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
endif

ifeq ($(PLATFORM),unixdev)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
endif

INCLUDES := $(subst /,$(COMPILER_SEP),$(FT_INCLUDES))

# Create directories for baseline and benchmark
$(BASELINE_DIR) $(BENCHMARK_DIR):
	@mkdir -p $@

$(FTBENCH_OBJ): $(FTBENCH_SRC) 
	@$(COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<) $(EXTRAFLAGS)
	@echo "Object created."

# Build ftbench
$(FTBENCH_BIN): $(FTBENCH_OBJ) 
	@echo "Linking ftbench..."
	@$(LINK_CMD) $T$(subst /,$(COMPILER_SEP),$@ $<) $(LINK_LIBS)
	@echo "Built."

# Create a baseline
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
		printf "\rProcessing %d%%..." $$percent; \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) "$$font" > $(BASELINE_DIR)$$(basename $$font .ttf).txt; \
	done
	@echo "Baseline created."

# Benchmark and compare to baseline
.PHONY: benchmark
benchmark: $(FTBENCH_BIN) $(BENCHMARK_DIR)
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
		printf "\rProcessing %d%%..." $$percent; \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) "$$font" > $(BENCHMARK_DIR)$$(basename $$font .ttf).txt; \
	done
	@$(PYTHON) $(HTMLCREATOR) $(OBJ_DIR)
	@echo "Benchmark results created in file: $(HTMLFILE)"

.PHONY: clean-benchmark
clean-benchmark:
	@echo "Cleaning..."
	@$(RM) $(FTBENCH_BIN) $(FTBENCH_OBJ)
	@$(RM) -rf $(BASELINE_DIR) $(BENCHMARK_DIR) $(HTMLFILE)
	@echo "Cleaned"
