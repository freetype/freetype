# Variables
FTBENCH_SRC = ftbench.c
FTBENCH_BIN = bench
FTBENCH_FLAGS = $(shell pkg-config --cflags freetype2) -lfreetype
FONTS = $(wildcard fonts/*.ttf)
BASELINES = $(addprefix baselines/, $(notdir $(FONTS)))
BENCHMARKS = $(addprefix benchmarks/, $(notdir $(FONTS)))
PYTHON = python3
HTMLCREATOR = src/tohtml.py

# Default target
all: $(FTBENCH_BIN)

# Build ftbench
$(FTBENCH_BIN): $(FTBENCH_SRC)
	gcc $(FTBENCH_FLAGS) $(FTBENCH_SRC) -o $(FTBENCH_BIN)
	@echo "Built."

# Create directories for baselines and benchmarks
baselines/ benchmarks/:
	mkdir -p $@

# Create a baseline
.PHONY: baseline
baseline: $(FTBENCH_BIN) baselines/
	$(foreach font, $(FONTS), \
		./$(FTBENCH_BIN) $(font) > baselines/$(notdir $(font)).txt; \
	)
	@echo "baseline created"

# Benchmark and compare to baseline
.PHONY: benchmark
benchmark: $(FTBENCH_BIN) benchmarks/
	$(foreach font, $(FONTS), \
		./$(FTBENCH_BIN) $(font) > benchmarks/$(notdir $(font)).txt; \
	)
	$(PYTHON) $(HTMLCREATOR)
	@echo "benchmark created."
	
.PHONY: page
page: benchmarks/ baselines/
	$(PYTHON) $(HTMLCREATOR)
	@echo "HTML page of the benchmark created."

.PHONY: clean
clean:
	rm -f $(FTBENCH_BIN)
	rm -rf baselines/ benchmarks/ *.html
	@echo "cleaned."