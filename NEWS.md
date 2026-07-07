# NEWS

## 2.1 - 2026-07-07

- Replaced checked-in Visual Studio solution/project files with a CMake-based
  build that generates IDE projects on demand.
- Added host simulator builds for Windows x86/x64, Linux x86/x64, and macOS.
- Added GoogleTest-based unit tests for queues, fixed-size memory pools, and
  malloc/realloc behavior.
- Added a scheduler smoke test for supported simulator hosts. This test is
  intentionally skipped on macOS ARM64 because simulator context switching is
  not supported on Apple Silicon yet.
- Added GitHub Actions CI for Linux x64/x86, Windows x64/Win32, macOS ARM64,
  and the QEMU ARM firmware smoke tests.
- Added a bare-metal ARM Cortex-M3 port for QEMU's `lm3s6965evb` machine,
  including hello-world, console, and RAM-backed filesystem firmware images.
- Added helper scripts for launching QEMU firmware images.
- Updated README documentation with CMake, testing, CI, and QEMU ARM usage.
