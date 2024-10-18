# PeIterator

A simple, lightweight, header-only library for iterating over components of PE (Portable Executable) files, written in C++.

## Features

- **Supports both x86 and x64 PE files**: The architecture of the file does not depend on the architecture of the running process.
- **Works with raw files and loaded images**: Can parse PE files directly from disk or already loaded and processed in memory.
- **Zero memory allocations**: No dynamic memory allocations are performed during parsing or iteration.

### The library provides iterators for the following PE components:

- Sections
- Imports
- Delayed imports
- Exports
- Relocations
- Exception handling (exception directory)
- TLS callbacks

## Usage

A complete example of usage can be found [here](https://github.com/CXDXVR/PeIterator/tree/main/Example).

### Basic Build:

```bash
cmake -S . -B Build
cmake --build Build
```

### Build with Example:
```bash
cmake -S . -B Build -D_BUILD_EXAMPLE=ON
cmake --build Build
```
