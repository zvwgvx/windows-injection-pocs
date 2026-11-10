# Build and validation

The PE inspector and Windows memory utilities have been built using CMake. The Windows targets and DLL loading support tools cross-compile with MinGW-w64.

The `targets/basic-target` and `targets/multithreaded-target` projects do not expose any channel for a payload controller. Normal in-process DLL loading tools live under `tools/`.

The old cooperative DLL loading CI test and controller have been removed; they did not implement cross-process injection. Other smoke tests validate the supporting programs.

Windows CI tests are authoritative for Windows runtime behavior. Do not describe a Windows cross-compile as proof of successful runtime injection.
