# Windows Injection PoCs

A personal C++20 research repository studying Windows PE internals, process memory, and process injection.

**Author:** [zvwgvx](https://github.com/zvwgvx)
**Primary platform:** Windows x64

## Architecture

Targets are ordinary, independent processes. They do not contain named events, command channels, shared loading hooks, or logic to accept DLL payloads from another program.

```text
windows-injection-pocs/
├── targets/
│   ├── basic-target/
│   └── multithreaded-target/
├── techniques/
│   ├── 01-dll-injection/
│   ├── 02-process-memory/
│   └── ...
├── tools/
│   ├── pe-inspect/
│   ├── dll-load-demo/
│   └── dll-host/
├── samples/
│   └── marker-dll/
└── docs/
```

## Current status

| ID | Topic | Status |
| --- | --- | --- |
| 01 | DLL injection | Planned; no injector implemented |
| 02 | Process memory | Implemented: `memory-layout` and `memory-regions` |
| 03 | PE injection | Planned |
| 04 | Manual mapping | Planned |
| 05 | Reflective loading | Planned |
| 06 | Section mapping | Planned |
| 07 | APC-based execution | Planned |
| 08 | Thread execution hijacking | Planned |
| 09 | Process hollowing | Planned |
| 10 | Module stomping | Planned |
| 11 | Process doppelgänging | Planned |
| 12 | Process herpaderping | Planned |
| 13 | Process ghosting | Planned |

A directory is not evidence of a working injector. No technique should claim injection unless a separate process actually initiates the loading without cooperation from the target. The repository currently contains independent test targets, memory utilities, and PE/DLL inspection tools, but no operational remote injector.

## Target programs

`basic-target.exe` is a standalone heartbeat process with a marker, PID, TID and configurable lifetime. `multithreaded-target.exe` uses three independent worker threads and allocations. Neither target accepts external DLLs, implements a remote-control protocol, nor invokes `LoadLibrary` in response to another process.

## Supporting tools

The normal DLL loading demo is deliberately located under `tools/`, not `techniques/`. It demonstrates how a process loads a DLL voluntarily and is not process injection.

| Tool | Purpose |
| --- | --- |
| `pe-inspect` | Read-only PE header, section, and import analysis |
| `dll-load-demo` | Normal in-process DLL loading baseline |
| `dll-host` | Explicit host for the benign marker DLL |
| `marker-dll` | DLL with two harmless exported functions |

## Build on Windows

Install CMake 3.20+ and Visual Studio C++ build tools.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

```powershell
.\build\bin\basic-target.exe --duration-sec 30
.\build\bin\multithreaded-target.exe --duration-sec 30
.\build\bin\pe-inspect.exe .\build\bin\basic-target.exe
.\build\bin\dll-load-demo.exe .\build\bin\marker-dll.dll
```

The PE inspector also builds on macOS and Linux, where it can examine Windows PE files without running them.

## Validation

GitHub Actions builds and runs smoke tests for Windows and macOS. Cross-compiling to Windows is distinct from runtime verification. Forensics notes are supplementary to Windows Internals research.

## License

MIT. See [LICENSE](LICENSE).
