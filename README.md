# Windows Injection PoCs

A personal collection of independent C++ proof-of-concept experiments exploring Windows PE internals, DLL loading, virtual memory, and process-injection research.

**Author:** [zvwgvx](https://github.com/zvwgvx)
**Language:** C++20
**Primary platform:** Windows x64
**Repository type:** Standalone PoCs, not a single unified injector.

## Implemented experiments

| Number | Topic | Programs | Status |
| --- | --- | --- | --- |
| 00 | PE format | `pe-inspect` | Implemented |
| 01 | DLL loading baseline | `dll-load-demo`, `marker-dll`, `dll-host` | Implemented |
| 02 | Process memory | `memory-layout`, `memory-regions`, `multithreaded-target` | Implemented |
| 03 | PE injection | — | Planned |
| 04 | Manual mapping | — | Planned |
| 05 | Reflective loading | — | Planned |
| 06 | Section mapping | — | Planned |
| 07 | APC-based execution | — | Planned |
| 08 | Thread execution hijacking | — | Planned |
| 09 | Process hollowing | — | Planned |
| 10 | Module stomping | — | Planned |
| 11 | Process doppelgänging | — | Planned |
| 12 | Process herpaderping | — | Planned |
| 13 | Process ghosting | — | Planned |

The catalogue covers related techniques, not exclusively MITRE ATT&CK T1055.002.

## Build on Windows

Install CMake 3.20+ and Visual Studio C++ build tools.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Executables and the sample DLL are placed in `build/bin` for supported generators.

## Run the first experiments

```powershell
.\build\bin\basic-target.exe --duration-sec 20
.\build\bin\pe-inspect.exe .\build\bin\basic-target.exe
.\build\bin\dll-load-demo.exe .\build\bin\marker-dll.dll
.\build\bin\dll-host.exe .\build\bin\marker-dll.dll --hold-ms 10000
.\build\bin\memory-layout.exe
.\build\bin\multithreaded-target.exe --duration-sec 20
.\build\bin\memory-regions.exe 4321
```

Use the actual PID printed by `basic-target` or `multithreaded-target` for `memory-regions`.

## Portable PE inspector

`pe-inspect` also builds on macOS and Linux, where it can inspect a PE file without executing it.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/bin/pe-inspect /path/to/windows.exe
```

## Targets

- **BasicTarget:** heartbeat, stable marker string, PID/TID, configurable runtime.
- **MultithreadedTarget:** three worker threads, independent private RW allocations and marker strings.
- **DllHost:** opt-in loading of the provided DLL, with a configurable inspection window.
- **Marker DLL:** two harmless exports, `LabMarker` and `LabIncrement`.

## Layout

```text
windows-injection-pocs/
├── targets/
│   ├── basic-target/
│   ├── multithreaded-target/
│   └── dll-host/
├── samples/
│   └── marker-dll/
├── tools/
│   └── pe-inspect/
├── techniques/
│   ├── 00-pe-format/
│   ├── 01-dll-loading/
│   ├── 02-process-memory/
│   └── 03-pe-injection/ ... 13-process-ghosting/
├── forensics/
├── docs/
└── .github/workflows/
```

Each numbered technique is documented independently. A directory marked planned is not an implemented injector.

## Experiment standards

Every completed experiment should include source, an independent build target, prerequisites, observed results, reproducibility notes, and limitations. See [experiment protocol](docs/experiment-protocol.md).

Experiments are designed for authorized local research using owned sample processes. The current programs do not inject into another process or provide evasion functionality. Forensics notes are optional supporting material.

## License

MIT. See [LICENSE](LICENSE).
