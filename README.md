# Windows Injection PoCs

A personal C++20 collection exploring Windows PE internals, loading behavior, process memory, and process-injection research through independent proof-of-concept programs.

**Author:** [zvwgvx](https://github.com/zvwgvx)
**Primary platform:** Windows x64
**Repository type:** Standalone PoCs, not a unified injection framework.

## Structure and build contract

Each numbered topic has a `code/` directory. An **implemented loading/injection experiment** is expected to have three independently built artifacts:

```text
techniques/<id>-<topic>/
├── CMakeLists.txt
├── README.md
└── code/
    ├── target.cpp
    ├── injector.cpp
    └── payload.cpp

build/bin/<id>-<topic>/
├── target.exe
├── injector.exe
└── payload.dll
```

`injector.exe` is a naming convention for the experiment's control program. Its actual behavior is documented for each topic. It must not be assumed to perform remote injection merely because of its filename. Foundation topics and incomplete entries are explicitly exempt from the three-artifact contract.

## Current status

| ID | Topic | Result |
| --- | --- | --- |
| 00 | PE format | Read-only `pe-inspect` executable; cross-platform |
| 01 | DLL loading baseline | Built `target.exe`, `injector.exe`, `payload.dll` |
| 02 | Process memory | `memory-layout`, `memory-regions` utilities |
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

The catalogue contains several related process and image execution techniques; not every item is strictly MITRE ATT&CK T1055.002.

The current `01` implementation is a **cooperative, opt-in DLL loading PoC**. The controller only signals the dedicated owned target, and the target loads the bundled DLL itself. This is not remote DLL injection, manual mapping, or screenshare evasion.

## Build

Windows: CMake 3.20+ and Visual Studio C++ build tools.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

macOS or Linux can build the read-only PE inspector:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run PoC 01

First terminal:

```powershell
.\build\bin\01-dll-loading\target.exe --duration-sec 30
```

Read the PID printed by the target. Second terminal:

```powershell
.\build\bin\01-dll-loading\injector.exe <target-pid>
```

The target should report `payload_marker=POC01_PAYLOAD_INSIDE_TARGET` followed by `payload_unloaded=true`.

The payload is the local file `build/bin/01-dll-loading/payload.dll`, compiled from `techniques/01-dll-loading/code/payload.cpp`.

## Other programs

```text
targets/basic-target
targets/multithreaded-target
targets/dll-host
samples/marker-dll
techniques/00-pe-format/code
techniques/02-process-memory/code
```

See the README inside each technique for its supported commands, technical scope and limitations. Every implemented PoC must have self-contained source and a reproducible result. The optional `forensics/` directory holds observations and detection notes.

## License

MIT. See [LICENSE](LICENSE).
