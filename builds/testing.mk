# Variables
FTBENCH_DIR = $(TOP_DIR)/src/tools/ftbench
FTBENCH_SRC = $(FTBENCH_DIR)/ftbench.c
FTBENCH_BIN = $(OBJ_DIR)/bench.o
FTBENCH_FLAG ?= -c 200
FONTS = $(wildcard $(FTBENCH_DIR)/fonts/*.ttf)
BASELINE = $(addprefix $(FTBENCH_DIR)/baseline/, $(notdir $(FONTS:.ttf=.txt)))
BENCHMARK = $(addprefix $(FTBENCH_DIR)/benchmark/, $(notdir $(FONTS:.ttf=.txt)))
BASELINE_DIR = $(FTBENCH_DIR)/baseline/
BENCHMARK_DIR = $(FTBENCH_DIR)/benchmark/
BASELINE_INFO = $(BASELINE_DIR)info.txt
BENCHMARK_INFO = $(BENCHMARK_DIR)info.txt
HTMLCREATOR = $(FTBENCH_DIR)/src/tohtml.py
HTMLFILE = $(TOP_DIR)/benchmark.html

# Create directories for baseline and benchmark
$(OBJ_DIR) $(BASELINE_DIR) $(BENCHMARK_DIR):
	@echo "Creating directory..."
	@mkdir -p $@

# Build ftbench
$(FTBENCH_BIN): $(FTBENCH_SRC) | $(OBJ_DIR)
	@echo "Building ftbench..."
	@$(CC) -I$(TOP_DIR)/include -lfreetype $< -o $@

# Create a baseline
.PHONY: baseline
baseline: $(FTBENCH_BIN) $(BASELINE_DIR)
	@$(RM) -f $(BASELINE)
	@echo "Creating baseline..."
	@echo "$(FTBENCH_FLAG)" > $(BASELINE_INFO)
	@echo "`git -C $(TOP_DIR) rev-parse HEAD`" >> $(BASELINE_INFO)
	@echo "`git -C $(TOP_DIR) show -s --format=%ci HEAD`" >> $(BASELINE_INFO)
	@echo "`git -C $(TOP_DIR) rev-parse --abbrev-ref HEAD`" >> $(BASELINE_INFO)
	@$(foreach font, $(FONTS), \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) $(font) >> $(BASELINE_DIR)$(notdir $(font:.ttf=.txt)); \
	)
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
	@$(foreach font, $(FONTS), \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) $(font) >> $(BENCHMARK_DIR)$(notdir $(font:.ttf=.txt)); \
	)
	@$(PYTHON) $(HTMLCREATOR)
	@echo "Benchmark created."

.PHONY: clean-benchmark
clean-benchmark:
	@echo "Cleaning..."
	@$(RM) $(FTBENCH_BIN)
	@$(RM) -rf $(BASELINE_DIR) $(BENCHMARK_DIR) $(HTMLFILE)
	@echo "Cleaned."
