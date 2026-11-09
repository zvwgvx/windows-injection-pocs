# Build and validation

## Local cross-compilation

- Host: macOS, AppleClang 17
- Windows cross-compiler: x86_64-w64-mingw32-g++ 15.2.0
- Build system: CMake 4.3.0

The native macOS `pe-inspect` build and rejection test passed. All existing Windows targets, sample DLLs, and foundation utilities cross-compiled successfully as PE32+ binaries.

The `01-dll-loading` cooperative experiment also cross-compiled to three standalone Windows x64 files: `target.exe`, `injector.exe`, and `payload.dll`.

## Windows runtime

Windows runtime tests are configured through GitHub Actions. The `poc01-cooperative-load` test starts the dedicated target, signals it through the companion controller, and checks for the marker exported by `payload.dll`.

A successful compile is not proof of successful runtime injection; the `01` experiment is normal target-initiated DLL loading after an explicit signal.
