# Windows build and validation

The project targets Windows x64 exclusively. GitHub Actions uses a Windows runner, Visual C++ build tools and CMake.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Passing build and smoke tests establishes that the current targets, PE inspection tools and process-memory utilities work under the tested Windows environment. It does not establish that any remote injection technique has been implemented.

## Independence contract

The `targets/basic-target` and `targets/multithreaded-target` programs have no payload-loading command channel, controller-specific named event, or callback that cooperates with an injector.

The normal DLL loading tools under `tools/` explicitly load DLLs themselves; they are not injection PoCs.

All unimplemented injection techniques remain labelled as planned. No injection claim is inferred from a directory name, successful compilation or a marker emitted by a cooperative target.
