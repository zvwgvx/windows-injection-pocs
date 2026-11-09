# Windows Injection PoCs

A personal C++ research collection exploring Windows Portable Executable (PE) internals, loading behavior, process memory, and process-injection techniques through independent proof-of-concept experiments.

**Author:** [zvwgvx](https://github.com/zvwgvx)  
**Primary language:** C++20  
**Platform:** Windows x64 (PE inspector also builds on macOS/Linux)  
**Project type:** Standalone PoC collection, not a unified injection framework.

## Repository layout

| Directory | Purpose |
| --- | --- |
| `targets/basic-target` | A heartbeat-driven Windows target process |
| `targets/multithreaded-target` | A Windows target with worker threads and allocated memory |
| `targets/dll-host` | A target that explicitly loads an experiment DLL |
| `samples/marker-dll` | Harmless exported marker DLL for controlled loading tests |
| `tools/pe-inspect` | Standalone, read-only PE header and section inspector |
| `techniques/` | Self-contained studies of injection and related execution mechanisms |
| `docs/` | Experiment protocol and technical references |
| `forensics/` | Optional forensic observations and detection notes |

## Quick start

Requirements: CMake 3.20+, a C++20 compiler, and Windows x64 for the Windows-specific programs.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
.\build\bin\Release\basic-target.exe
```

For Ninja or other single-configuration generators, omit `-A x64`. Use the generated executable location for your generator.

On macOS and Linux, the portable PE inspector can still be built:

```bash
cmake -S . -B build
cmake --build build
./build/bin/pe-inspect path/to/sample.exe
```

### Controlled DLL loading

`dll-host` accepts an explicit path to the provided `marker-dll` and calls its exported `LabMarker` function. This is normal opt-in DLL loading by the target, not remote injection.

## Research catalogue

The numbered directories document the following topics. An entry marked **planned** does not imply a working injector exists.

| ID | Topic | State |
| --- | --- | --- |
| 00 | PE format and loader fundamentals | PE inspector available |
| 01 | Normal DLL loading baseline | Marker DLL and target available |
| 02 | Process memory and permissions | Multithreaded target available |
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

The topics span PE loading, process injection, and related execution techniques. They are **not** all MITRE ATT&CK T1055.002 sub-techniques.

## Experiment standards

Every completed experiment should have its own source, CMake build target, clear prerequisites, exact observations, failure modes, and a reproducibility note. Targets must be owned by the researcher and run in an isolated test environment.

The optional forensics directory records observed memory regions, loaded modules, and event artifacts. The central purpose of this repository is PoC research, with forensics as supporting analysis.

## Scope

This repository is for authorized local experimentation and education. The initial release includes benign sample processes and a read-only PE inspector; it does not contain a general-purpose remote injector, anti-forensics capability, or screenshare-evasion tool.

## License

MIT. See [LICENSE](LICENSE).
