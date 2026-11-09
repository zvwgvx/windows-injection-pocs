# 01 — Cooperative DLL loading baseline

**Status:** Implemented. This is a cooperative loading baseline, not remote process injection.

## Build outputs

```text
build/bin/01-dll-loading/
├── target.exe
├── injector.exe
└── payload.dll
```

All three programs have independent C++ source files under [code](code/). The executable named `injector.exe` is an experiment controller: it signals a named Windows event created by the dedicated sample target. The target then loads the fixed `payload.dll` located in its own output directory and invokes two exported marker functions. It does not allocate or write memory in unrelated processes.

## Run

Build with CMake on Windows x64, then start:

```powershell
.\build\bin\01-dll-loading\target.exe --duration-sec 30
```

The target prints its PID. In a second terminal:

```powershell
.\build\bin\01-dll-loading\injector.exe <target-pid>
```

The first terminal should print `event_received=true`, `payload_marker=POC01_PAYLOAD_INSIDE_TARGET` and `payload_unloaded=true`.

The marker DLL is loaded by the target using `LoadLibraryExW`. No arbitrary PE loader, memory injection, or evasion behavior is implemented.

A separate baseline `dll-load-demo` also demonstrates direct in-process DLL loading with the common `marker-dll` sample.
