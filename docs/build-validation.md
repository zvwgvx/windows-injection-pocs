# Build and validation

## Local cross-compilation

- Host: macOS, AppleClang 17
- Windows cross-compiler: x86_64-w64-mingw32-g++ 15.2.0
- Build system: CMake 4.3.0

The native macOS `pe-inspect` build and its rejection test passed. All Windows targets, sample DLLs and technique programs cross-compiled successfully as PE32+ binaries.

The native macOS PE inspector correctly reported sections and imports for the cross-compiled `basic-target.exe` and `marker-dll.dll`. It reported machine `0x8664` and expected PE32+ metadata.

## Windows runtime

Windows runtime execution is not yet locally verified. A GitHub Actions matrix runs the sample process and DLL loading smoke tests on Windows CI.

This document distinguishes a successful cross-compile from successful execution on Windows.
