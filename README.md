# Windows Injection PoCs

A personal C++20 research repository studying Windows PE internals, process memory, and process-injection mechanisms.

**Author:** [zvwgvx](https://github.com/zvwgvx)
**Platform:** Windows x64 only

## Architecture

Targets are ordinary Windows processes. They do not provide special DLL-loading endpoints, prearranged injector events, or code-execution hooks.

Cooperation in experiments is limited to **ground-truth collection and comparison**: the target may expose its PID and diagnostic output, while an independent read-only observer measures module, thread, and memory-region metadata. This cooperation does not cause payload execution.

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
│   ├── process-observer/
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

A directory, a file called `injector.exe`, or a marker printed by the target is not proof of remote injection. The repository does not yet include an operational cross-process injector.

## Independent target programs

`basic-target.exe` is a heartbeat process with a marker, PID, TID and configurable lifetime. `multithreaded-target.exe` has three worker threads and local memory allocations. Neither program accepts external DLLs or implements a controller protocol.

## Supporting tools

| Tool | Purpose |
| --- | --- |
| `pe-inspect` | Read-only PE headers, sections and imports |
| `process-observer` | Independent process metadata, module list, thread list and memory summaries |
| `dll-load-demo` | Voluntary in-process DLL-loading baseline |
| `dll-host` | Explicit host for the harmless marker DLL |
| `marker-dll` | DLL exporting two marker functions |

The normal DLL loading demos are support tools, not injection PoCs.

## Build and test on Windows

Install CMake 3.20+ and Visual Studio C++ build tools.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The repository contains **no GitHub Actions CI**. Run all build and runtime checks directly on Windows.

## Observation baseline

Start the target in one terminal:

```powershell
.\build\bin\basic-target.exe --duration-sec 30
```

Read the PID from the target's output. Use a separate terminal to observe it:

```powershell
.\build\bin\process-observer.exe --pid 1234 --samples 5 --interval-ms 1000
```

Replace `1234` with the printed PID. This is metadata observation only: no process manipulation or payload execution is performed.

See [validation protocol](docs/experiment-protocol.md) for evidence requirements and limitations.

## License

MIT. See [LICENSE](LICENSE).
