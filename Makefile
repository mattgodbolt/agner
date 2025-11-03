.PHONY: setup build driver clean format lint help test

# Default target
all: build

help:
	@echo "Agner - x86 CPU Performance Investigation Tool"
	@echo ""
	@echo "Available targets:"
	@echo "  setup       - Install Python dependencies with uv"
	@echo "  build       - Build C++ test harness"
	@echo "  driver      - Build and install kernel driver (requires sudo)"
	@echo "  test        - Run all available tests (requires driver)"
	@echo "  clean       - Remove build artifacts"
	@echo "  format      - Format code with ruff"
	@echo "  lint        - Lint code with ruff"
	@echo "  help        - Show this help message"

setup:
	uv sync

build: setup
	$(MAKE) -C src

driver:
	@echo "Building and installing kernel driver (requires sudo)..."
	$(MAKE) -C src/driver
	cd src/driver && sudo ./install.sh

uninstall-driver:
	@echo "Uninstalling kernel driver (requires sudo)..."
	cd src/driver && sudo ./uninstall.sh

test: build
	@if [ ! -e /dev/MSRdrv ]; then \
		echo "Error: Kernel driver not loaded. Run 'make driver' first."; \
		exit 1; \
	fi
	uv run python agner list

clean:
	$(MAKE) -C src clean
	rm -rf src/out
	find . -type f -name '*.pyc' -delete
	find . -type d -name '__pycache__' -delete

format:
	uv run ruff format .

lint:
	uv run ruff check .
