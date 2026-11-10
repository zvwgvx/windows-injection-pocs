# Windows build and validation

The project targets Windows x64 exclusively. There is no GitHub Actions CI or other hosted pipeline.

Run directly on Windows:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The `targets/basic-target` and `targets/multithreaded-target` programs have no payload-loader interface. `process-observer` reads process metadata without a target-specific protocol.

A Windows build, a successful metadata snapshot, or normal in-process DLL loading is not evidence of remote process injection.

The completed experiments remain labelled by their demonstrated behavior. Other technique directories remain planned.
