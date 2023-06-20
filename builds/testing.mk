# Variables
FTBENCH_DIR = $(TOP_DIR)/src/tools/ftbench
FTBENCH_SRC = $(FTBENCH_DIR)/ftbench.c
FTBENCH_BIN = $(OBJ_DIR)/bench.o
FTBENCH_FLAG = -c 50
FONTS = $(wildcard $(FTBENCH_DIR)/fonts/*.ttf)
BASELINE = $(addprefix $(FTBENCH_DIR)/baseline/, $(notdir $(FONTS:.ttf=.txt)))
BENCHMARK = $(addprefix $(FTBENCH_DIR)/benchmark/, $(notdir $(FONTS:.ttf=.txt)))
BASELINE_DIR = $(FTBENCH_DIR)/baseline/
BENCHMARK_DIR = $(FTBENCH_DIR)/benchmark/
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
	@echo "Creating baseline..."
	@$(foreach font, $(FONTS), \
		echo "Parameters: $(FTBENCH_FLAG)" > $(BASELINE_DIR)$(notdir $(font:.ttf=.txt)); \
		echo "Commit ID: `git rev-parse HEAD`" >> $(BASELINE_DIR)$(notdir $(font:.ttf=.txt)); \
		echo "Commit Date: `git show -s --format=%ci HEAD`" >> $(BASELINE_DIR)$(notdir $(font:.ttf=.txt)); \
		echo "Branch: `git rev-parse --abbrev-ref HEAD`" >> $(BASELINE_DIR)$(notdir $(font:.ttf=.txt)); \
		$(FTBENCH_BIN) $(FTBENCH_FLAG) $(font) >> $(BASELINE_DIR)$(notdir $(font:.ttf=.txt)); \
	)
	@echo "Baseline created."

# Benchmark and compare to baseline
.PHONY: benchmark
benchmark: $(FTBENCH_BIN) $(BENCHMARK_DIR)
	@echo "Creating benchmark..."
	@$(foreach font, $(FONTS), \
		echo "Parameters: $(FTBENCH_FLAG)" > $(BENCHMARK_DIR)$(notdir $(font:.ttf=.txt)); \
		echo "Commit ID: `git rev-parse HEAD`" >> $(BENCHMARK_DIR)$(notdir $(font:.ttf=.txt)); \
		echo "Commit Date: `git show -s --format=%ci HEAD`" >> $(BENCHMARK_DIR)$(notdir $(font:.ttf=.txt)); \
		echo "Branch: `git rev-parse --abbrev-ref HEAD`" >> $(BENCHMARK_DIR)$(notdir $(font:.ttf=.txt)); \
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
