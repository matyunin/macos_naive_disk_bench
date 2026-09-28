# Naive macOS Native Disk Benchmark

A lightweight disk benchmark for macOS using only tools and APIs provided by the system.

The project measures sequential and random disk performance without requiring Homebrew, `fio`, Python, or other third-party dependencies.

## Features

- Native macOS implementation
- Uses Apple Clang and POSIX APIs
- No third-party dependencies
- Temporary benchmark data is stored exclusively in `/tmp`
- Temporary files are removed automatically after the benchmark
- Measures:
  - Sequential Read
  - Sequential Write
  - Random Read 32K QD20
  - Random Read 4K QD1
- Reports both throughput and IOPS for random workloads
- Designed for quick SSD/NVMe performance checks from Terminal

## Requirements

- macOS
- Apple Clang
- Xcode Command Line Tools

Check whether Clang is available:

    clang --version

If Command Line Tools are not installed, install them with:

    xcode-select --install

No Homebrew or additional packages are required.

## Quick Start

Clone the repository:

    git clone https://github.com/matyunin/macos_naive_disk_bench.git
    cd macos_naive_disk_bench

    clang -O3 -pthread diskbench.c -o diskbench

Then run:

    ./diskbench

## Example Output

    ============================================
               macOS Disk Benchmark
    ============================================

    Test file : /tmp/diskbench.dat
    Size      : 4.3 GB

    Sequential Read            2950.2 MB/s
    Sequential Write           987.6 MB/s

    Random Read 4K QD1            164.7 MB/s  (     40804 IOPS)
    Random Read 32K QD20         2082.4 MB/s  (    125102 IOPS)

    ============================================

Actual results depend on the Mac model, storage device, filesystem, available free space, thermal state, background activity, and other system conditions.

## Test Descriptions

### Sequential Read

Reads a large test file sequentially from beginning to end.

This workload primarily measures sustained sequential read throughput.

### Sequential Write

Creates and writes a large test file sequentially.

The benchmark calls `fsync()` before reporting the result in order to make the write measurement more representative of data being committed to storage.

### Random Read 32K QD20

Performs random reads using 32 KiB blocks with approximately 20 concurrent operations.

The result is reported both as:

- MB/s
- IOPS

Throughput is calculated from the measured IOPS and block size.

### Random Read 4K QD1

Performs random 4 KiB reads with a queue depth of 1.

This workload is useful for observing small-block, low-queue-depth storage performance, which is relevant to many everyday operating-system workloads.

## Temporary Files

The benchmark uses:

    /tmp/diskbench.dat

and, when applicable:

    /tmp/diskbench
    /tmp/diskbench.c

Temporary benchmark files are removed after the test completes.

The benchmark does not intentionally store test data in the user's home directory.

> **Warning:** The sequential write test creates a multi-gigabyte temporary file. Make sure sufficient free space is available on the filesystem containing `/tmp`.

## Running on the System Disk

On macOS, `/tmp` is normally located on the system filesystem. Therefore, this benchmark is intended to test the storage backing the system filesystem rather than requiring the user to manually select a disk device.

This also makes the benchmark safer than directly writing to a raw block device.

## Performance Considerations

For more consistent results:

- Close applications performing significant disk I/O.
- Avoid running the benchmark while large files are being copied.
- Run the benchmark more than once if you need to compare results.
- Allow the Mac to reach a normal operating temperature.
- Keep sufficient free space available on the system volume.
- Compare results using the same macOS version and test configuration.

Laptop SSD performance can vary significantly depending on temperature, power state, and sustained-write behavior.

## Important Limitations

This is a lightweight native benchmark rather than a replacement for a full storage benchmarking suite.

Results should not be directly compared with tools such as CrystalDiskMark, `fio`, Blackmagic Disk Speed Test, or AmorphousDiskMark unless the workload parameters and methodology are equivalent.

In particular, queue depth, caching behavior, filesystem effects, block sizes, read/write ratios, test duration, and synchronization semantics can significantly affect the reported numbers.

The benchmark is primarily intended for:

- Quick SSD performance checks
- Before/after comparisons
- Diagnosing unexpectedly slow storage
- Comparing Macs under similar conditions
- Simple command-line benchmarking

## Why No `fio`?

`fio` is an excellent storage benchmark, but this project intentionally avoids external dependencies.

The goal is to provide a small, portable benchmark that can be compiled and run on a standard macOS installation with Apple's developer tools.

The implementation relies on native APIs such as:

- `pread()`
- `write()`
- `fsync()`
- POSIX threads
- `gettimeofday()`
- Apple Clang

## Safety

The benchmark operates on a temporary file rather than a raw disk device.

It does not intentionally overwrite partitions, disk devices, or user files.

Nevertheless, disk benchmarks generate substantial I/O. Do not run benchmarks on systems where unexpected heavy disk activity could cause problems.

## License

This project is released under the MIT License.

See `LICENSE` for details.

## Contributing

Issues and pull requests are welcome.

Useful contributions include:

- More accurate queue-depth handling
- Additional read/write workloads
- Better cache control
- More consistent timing
- Support for additional block sizes
- Improved result formatting
- Reproducibility improvements
- Support for Apple Silicon and Intel Macs

When submitting benchmark-related changes, please include the macOS version, Mac model, storage configuration, and benchmark output where relevant.

## Disclaimer

Benchmark results are affected by many factors and should not be treated as a definitive specification of storage hardware performance.

This project is provided for testing and informational purposes.

