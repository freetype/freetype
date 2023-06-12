# Variables
FTBENCH_DIR = $(TOP_DIR)/src/tools/ftbench
FTBENCH_SRC = $(FTBENCH_DIR)/ftbench.c
FTBENCH_BIN = $(FTBENCH_DIR)/bench
FTBENCH_FLAGS = $(shell pkg-config --cflags freetype2) -lfreetype
FONTS = $(wildcard $(FTBENCH_DIR)/fonts/*.ttf)
BASELINES = $(addprefix $(FTBENCH_DIR)/baselines/, $(notdir $(FONTS:.ttf=.txt)))
BENCHMARKS = $(addprefix $(FTBENCH_DIR)/benchmarks/, $(notdir $(FONTS:.ttf=.txt)))
PYTHON = python3
HTMLCREATOR = $(FTBENCH_DIR)/src/tohtml.py
HTMLFILE = $(TOP_DIR)/benchmark.html

# Create directories for baselines and benchmarks
$(FTBENCH_DIR)/baselines/ $(FTBENCH_DIR)/benchmarks/:
	@mkdir -p $@

# Build ftbench
$(FTBENCH_BIN): $(FTBENCH_SRC)
	@echo "Building ftbench..."
	@gcc $(FTBENCH_FLAGS) $< -o $@

# Create a baseline
.PHONY: baseline
baseline: $(FTBENCH_BIN) $(FTBENCH_DIR)/baselines/
	@echo "Creating baseline..."
	@$(foreach font, $(FONTS), \
		$(FTBENCH_BIN) $(font) > $(FTBENCH_DIR)/baselines/$(notdir $(font:.ttf=.txt)); \
	)
	@echo "Baseline created."

# Benchmark and compare to baseline
.PHONY: benchmark
benchmark: $(FTBENCH_BIN) $(FTBENCH_DIR)/benchmarks/
	@echo "Creating benchmark..."
	@$(foreach font, $(FONTS), \
		$(FTBENCH_BIN) $(font) > $(FTBENCH_DIR)/benchmarks/$(notdir $(font:.ttf=.txt)); \
	)
	@$(PYTHON) $(HTMLCREATOR) > $(HTMLFILE)
	@echo "Benchmark created."

.PHONY: clean-benchmark
clean-benchmark:
	@echo "Cleaning..."
	@rm -f $(FTBENCH_BIN)
	@rm -rf $(FTBENCH_DIR)/baselines/ $(FTBENCH_DIR)/benchmarks/ $(HTMLFILE)
	@echo "Cleaned."
