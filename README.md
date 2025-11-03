# Agner

A suite of tools, drivers and scripts for investigating the performance of x86 CPUs using Performance Monitor Counters (PMCs).

Based very heavily on [Agner Fog](http://www.agner.org)'s [test programs](http://www.agner.org/optimize/#testp).

## Features

- **Branch Prediction Tests**: Measure branch predictor behavior with different patterns
- **BTB Investigation**: Explore Branch Target Buffer size, set bits, ways, and address bits
- **Performance Counters**: Access hardware PMCs for microarchitecture analysis
- **Dynamic Test Generation**: Generate x86-64 assembly test code on-the-fly
- **Visualization**: Plot results with matplotlib

## Quick Start

### Prerequisites

- Python 3.9+
- uv (install via `curl -LsSf https://astral.sh/uv/install.sh | sh`)
- g++ compiler
- nasm assembler
- Linux kernel headers (for driver)
- sudo access (for kernel driver)

### Installation

```bash
# Clone the repository
git clone <your-repo-url>
cd agner

# Setup and build
make setup    # Install Python dependencies
make build    # Build C++ test harness
make driver   # Install kernel driver (requires sudo)
```

### Running Tests

```bash
# List available tests
uv run python agner list

# Run all tests with interactive plots
uv run python agner run

# Run specific test
uv run python agner run branch.Ahead_not_taken

# Save results to JSON
uv run python agner test_only -r results.json

# Plot existing results
uv run python agner plot -r results.json

# Export as PDF
uv run python agner plot -r results.json --pdf output.pdf
```

## Available Commands

- `agner install` - Build and install kernel driver (requires sudo)
- `agner uninstall` - Uninstall kernel driver (requires sudo)
- `agner list` - List all available tests
- `agner run [test]` - Run tests and display plots interactively
- `agner test_only [test]` - Run tests and save results to JSON
- `agner plot` - Plot existing results from JSON

## Available Tests

### Branch Prediction (`branch`)
- `Ahead not taken` - Forward branch, not taken
- `Behind not taken` - Backward branch, not taken
- `Ahead taken` - Forward branch, taken
- `Behind taken` - Backward branch, taken

### BTB Size (`btb_size`)
- `Total size` - Investigate total BTB size
- `Bits in set` - Determine set index bits
- `Number of ways` - Find associativity
- `Number of address bits for set` - Address bit mapping

## Architecture

```
agner (CLI)
  ├─> Python test framework (lib/agner.py)
  ├─> C++ test harness (src/PMCTestA.cpp)
  ├─> x86-64 assembly tests (src/PMCTestB64.nasm)
  ├─> Test definitions (tests/*.py)
  └─> Kernel driver (src/driver/MSRdrv)
```

## Development

```bash
# Format code
make format

# Lint code
make lint

# Clean build artifacts
make clean
```

See [CLAUDE.md](CLAUDE.md) for detailed development documentation.

## Requirements

- **Platform**: Linux x86-64
- **CPU**: Intel/AMD x86-64 with performance counters
- **Access**: Tests must run on bare metal (PMCs not available in most VMs)
- **Privileges**: Kernel driver requires sudo for installation

## Security Note

The kernel driver (`MSRdrv`) provides userspace access to Model-Specific Registers (MSRs), which are privileged CPU resources. Only use on test/development machines. The driver is not persistent across reboots.

## Tips for Best Results

- Disable CPU frequency scaling: `cpupower frequency-set -g performance`
- Close unnecessary applications to reduce system noise
- Run tests multiple times to account for variability
- Disable turbo boost for consistent results

## License

GNU GPL (see LICENSE file)

## References

- [Agner Fog's Optimization Resources](http://www.agner.org/optimize/)
- [Intel Software Developer Manuals](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
