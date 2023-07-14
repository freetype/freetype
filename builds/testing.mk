# Variables
FTBENCH_DIR = $(TOP_DIR)/src/tools/ftbench
FTBENCH_SRC = $(FTBENCH_DIR)/ftbench.c
FTBENCH_OBJ = $(OBJ_DIR)/bench.$(SO)
FTBENCH_BIN = $(OBJ_DIR)/bench
FTBENCH_FLAG ?= -c 200
INCLUDES = -I$(TOP_DIR)/include
FONTS = $(wildcard $(FTBENCH_DIR)/fonts/*.ttf)
BASELINE_DIR = $(OBJ_DIR)/baseline/
BENCHMARK_DIR = $(OBJ_DIR)/benchmark/
BASELINE = $(addprefix $(BASELINE_DIR), $(notdir $(FONTS:.ttf=.txt)))
BENCHMARK = $(addprefix $(BENCHMARK_DIR), $(notdir $(FONTS:.ttf=.txt)))
BASELINE_INFO = $(BASELINE_DIR)info.txt
BENCHMARK_INFO = $(BENCHMARK_DIR)info.txt
HTMLCREATOR = $(FTBENCH_DIR)/src/tohtml.py
HTMLFILE = $(OBJ_DIR)/benchmark.html

# Create directories for baseline and benchmark
$(OBJ_DIR) $(BASELINE_DIR) $(BENCHMARK_DIR):
	@mkdir -p $@

# Build ftbench.o
$(FTBENCH_OBJ): $(FTBENCH_SRC)
	@echo "Building ftbench object..."
	$(CC) $(INCLUDES) -c $< -o $@
	
# Build ftbench
$(FTBENCH_BIN): $(FTBENCH_OBJ)
	@echo "Linking ftbench..."
	$(LIBTOOL) --mode=link gcc -L$(LIB_DIR) -lfreetype $< -o $@
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
	@echo "\nBaseline created."

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
	@echo "\nBenchmark created."

.PHONY: clean-benchmark
clean-benchmark:
	@echo "Cleaning..."
	@$(RM) $(FTBENCH_BIN) $(FTBENCH_OBJ)
	@$(RM) -rf $(BASELINE_DIR) $(BENCHMARK_DIR) $(HTMLFILE)
	@echo "Cleaned."
