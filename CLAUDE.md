# Agner - CPU Performance Investigation Tool

## Overview

Agner is a suite of tools for investigating x86 CPU microarchitecture performance characteristics using Performance Monitor Counters (PMCs). Based heavily on [Agner Fog's test programs](http://www.agner.org/optimize/#testp).

### What It Does

- Measures CPU microarchitecture behavior using hardware performance counters
- Tests branch prediction, BTB size/configuration, cache performance, and more
- Generates assembly test code dynamically using NASM
- Visualizes results with matplotlib

### Architecture

```
agner (Python CLI)
  ├─> lib/agner.py (Test framework)
  │   ├─> Dynamically compiles C++ harness (PMCTestA.cpp)
  │   ├─> Generates assembly test code (PMCTestB64.nasm)
  │   └─> Links and executes tests
  ├─> tests/*.py (Test definitions)
  │   ├─> branch.py - Branch prediction tests
  │   └─> btb_size.py - BTB size/configuration tests
  └─> src/driver/ (Linux kernel module for MSR access)
```

## Development Setup

### Prerequisites

- uv
- g++ (C++ compiler)
- nasm (Netwide Assembler)
- make
- Linux kernel headers (for driver: `linux-headers-$(uname -r)`)
- sudo access (for kernel driver installation)

### Initial Setup

```bash
# Install Python dependencies
make setup

# Build C++ test harness
make build

# Build and install kernel driver (requires sudo)
make driver
```

## Build Commands

### Common Operations

```bash
make setup          # Install Python dependencies with uv
make build          # Build C++ test harness
make driver         # Build and install kernel driver (sudo)
make uninstall-driver  # Remove kernel driver (sudo)
make clean          # Remove build artifacts
make format         # Format code with ruff
make lint           # Lint code with ruff
make help           # Show all available targets
```

### Running Tests

```bash
# List all available tests
uv run python agner list

# Run all tests and show interactive plots
uv run python agner run

# Run specific test
uv run python agner run "branch.Ahead not taken"

# Run test family with wildcard
uv run python agner run "branch.*"

# Save results to JSON
uv run python agner test_only -r results.json branch

# Plot existing results
uv run python agner plot -r results.json

# Export plots as PDF
uv run python agner plot -r results.json --pdf output.pdf

# Export plots as PNG (templated)
uv run python agner plot --png "plot_{test}_{subtest}.png"
```

## How It Works

### Test Execution Flow

1. **Test Definition** (`tests/*.py`)
   - Python modules define test cases
   - Each test generates x86-64 assembly code
   - Specifies which PMC counters to read

2. **Dynamic Compilation** (`lib/agner.py:run_test()`)
   - Writes assembly test code to `src/out/test.inc`
   - Writes PMC counter list to `src/out/counters.inc`
   - Compiles C++ harness: `g++ -O2 PMCTestA.cpp -o out/a64.o`
   - Assembles test code: `nasm -f elf64 PMCTestB64.nasm -o out/b64.o`
   - Links: `g++ -o out/test out/a64.o out/b64.o -lpthread`

3. **Execution**
   - Runs compiled test binary
   - Accesses MSRs via `/dev/MSRdrv` kernel driver
   - Returns PMC counter values as CSV

4. **Visualization**
   - Test-specific plotting functions render results
   - matplotlib generates interactive plots or exports

### Key Files

- **agner_main.py**: Main CLI entry point
- **lib/agner.py**: Test framework and dynamic compilation
- **src/PMCTestA.cpp**: C++ test harness (sets up PMCs, runs test code)
- **src/PMCTestB64.nasm**: Assembly test framework (includes generated test.inc)
- **src/PMCTest.h**: PMC definitions and test framework interface
- **src/driver/MSRdrv.c**: Linux kernel module for MSR access

## Code Conventions

### Python

- Python 3.9+ (migrated from Python 2)
- Use ruff for formatting and linting
- Type hints encouraged but not required
- Tests in `tests/` directory auto-discovered

### Test Writing

Tests are Python modules in `tests/` that define:

```python
def add_tests(agner):
    """Called by framework to register tests"""
    agner.add_test(name, test_function, plot_function)

def test_function():
    """Returns test results as list of dicts"""
    test_code = """
    ; x86-64 assembly using NASM syntax
    ; Use %REP/%ENDREP for loops
    ; align directives for alignment
    """
    return run_test(test_code, [counter_ids...])

def plot_function(results, alternative):
    """Plots results using matplotlib"""
    import matplotlib.pyplot as plt
    # ... plotting code ...
```

### Assembly Test Code

- NASM syntax (Intel style)
- Use macros for repeated patterns
- Use `align` directives to control layout
- Test code runs in a loop controlled by REPETITIONS

## Performance Counter Reference

Common PMC counter IDs (Intel):
- 1: Instructions retired
- 9: Branch instructions retired
- 207-404: Various branch/cache/execution counters

See `src/PMCTest.h` for full counter definitions.

## Kernel Driver Notes

- **Purpose**: Provides userspace access to Model-Specific Registers (MSRs)
- **Security**: Only install on test/dev machines (grants low-level CPU access)
- **Device**: Creates `/dev/MSRdrv` character device
- **Persistence**: Not persistent across reboots (by design)
- **Build**: Uses kernel build system (`make -C /lib/modules/$(uname -r)/build`)

### Driver Commands

```bash
# Build
cd src/driver && make

# Install (creates /dev/MSRdrv)
cd src/driver && sudo ./install.sh

# Uninstall
cd src/driver && sudo ./uninstall.sh

# Check if loaded
lsmod | grep MSRdrv
ls -l /dev/MSRdrv
```

## Testing & Validation

- Tests require kernel driver to be loaded
- Tests must run on bare metal (not VMs without PMC access)
- Results vary by CPU microarchitecture
- Some tests take significant time (minutes)

## Troubleshooting

### "The performance counter driver is not loaded"
```bash
make driver  # Install the kernel driver
```

### "Permission denied" on /dev/MSRdrv
```bash
ls -l /dev/MSRdrv  # Check permissions
sudo chmod 666 /dev/MSRdrv  # Or add user to device group
```

### Assembly doesn't compile
- Check NASM syntax (Intel style, case-insensitive)
- Verify alignment directives
- Check for unterminated macros

### Inconsistent results
- Disable CPU frequency scaling: `cpupower frequency-set -g performance`
- Disable turbo boost
- Reduce system load (close other apps)
- Increase test repetitions

## Design Decisions

### Why Python + C++ + Assembly?

- **Python**: High-level test orchestration, plotting
- **C++**: PMC access, test framework infrastructure
- **Assembly**: Precise control of instruction sequences for testing

### Why Dynamic Compilation?

- Tests generate unique assembly code per run
- Allows parameterized test patterns
- Ensures fresh code layout (no instruction cache effects)

### Why Kernel Driver?

- MSRs require ring 0 (kernel mode) access
- Safer than modifying /dev/mem
- Isolated interface for PMC operations

## Future Improvements

- [ ] Add pre-commit hook installation to setup
- [ ] Consider pytest for Python test infrastructure
- [ ] Add more CPU vendors (AMD, ARM?)
- [ ] Document PMC counter meanings per microarch
- [ ] Add result comparison/regression detection
- [ ] Cache compiled harness to speed up test runs
