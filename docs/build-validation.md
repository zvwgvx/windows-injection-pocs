# Windows build and validation

The project targets Windows x64 exclusively. There is no GitHub Actions CI or other hosted pipeline.

Run directly on Windows:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The `targets/basic-target` and `targets/multithreaded-target` programs have no payload-loader interface. `process-observer` reads process metadata without a target-specific protocol.

The CTest suite includes `dll-injection-smoke`. It launches `basic-target.exe`,
checks that the payload is absent, runs the real `poc01-injector.exe`, verifies the
module and payload log, refuses a duplicate injection, and checks that the target is
still alive. The test requires same-user x64 Windows. Antivirus or EDR may block
`CreateRemoteThread`; record the block and the Windows build in the run notes rather
than weakening the evidence chain.

Cross-compiling with MinGW on a non-Windows host verifies compilation only. It is not
runtime evidence of injection.

A Windows build, a successful metadata snapshot, or normal in-process DLL loading is not evidence of remote process injection.

The completed experiments remain labelled by their demonstrated behavior. Other technique directories remain planned.
