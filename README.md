# Windows Injection PoCs

**Windows x64 process-injection research in C++20.** Independent experiments with source code, documented mechanisms and reproducible evidence where a technique has been implemented.

**Platform:** Windows x64 · **Language:** C++20 · **Build:** CMake · **Validation:** Local Windows runs, no CI

[Verified PoC](#verified-poc) · [Technique catalogue](#technique-catalogue) · [Build](#build-and-test) · [Targets and tools](#targets-and-tools) · [Evidence policy](#evidence-policy)

---

## Verified PoC

### Classic DLL Injection — `poc01`

**Implemented and documented.** The existing PoC uses `CreateRemoteThread` + `LoadLibraryW` against an independently running target that has no injector-specific loading interface. Its recorded validation environment is Windows 10 Pro 22H2 (x64).

| Resource | Direct link |
| --- | --- |
| Documentation and usage | [Classic DLL Injection](techniques/pe-injection/dll-injection/README.md) |
| Injector source | [`poc01-injector.cpp`](techniques/pe-injection/dll-injection/code/poc01-injector.cpp) |
| DLL payload | [`poc01-payload.cpp`](techniques/pe-injection/dll-injection/code/poc01-payload.cpp) |
| Heartbeat payload | [`poc01-payload-heartbeat.cpp`](techniques/pe-injection/dll-injection/code/poc01-payload-heartbeat.cpp) |
| Local self-test | [`poc01-selftest.cpp`](techniques/pe-injection/dll-injection/code/poc01-selftest.cpp) |
| Runtime proof | [Windows run record, raw output and hashes](docs/runs/2026-11-11-technique-01.md) |
| Independent corroboration | [System Informer screenshot](docs/runs/assets/2026-11-11-technique-01-system-informer.png) |

The documented self-test completed **absent → injected → present → duplicate refused**, and the target remained alive. System Informer corroborated the module base, while the heartbeat variant provided additional payload-side execution evidence. These results apply to the recorded test environment.

**Repository status:** 1 implemented injection PoC · 20 planned techniques. Planned entries do not contain working injectors.

## Technique catalogue

Technique names link **directly to their documentation**. The primary group describes the payload format or process-image mechanism; an execution method can span multiple groups.

### Shellcode Injection

| Technique | Status |
| --- | --- |
| [Remote Thread](techniques/shellcode-injection/remote-thread/README.md) | Planned |
| [APC Injection](techniques/shellcode-injection/apc-injection/README.md) | Planned |
| [Early Bird APC](techniques/shellcode-injection/early-bird-apc/README.md) | Planned |
| [Thread Hijacking](techniques/shellcode-injection/thread-hijacking/README.md) | Planned |
| [Section Mapping](techniques/shellcode-injection/section-mapping/README.md) | Planned |
| [Threadpool Injection](techniques/shellcode-injection/threadpool-injection/README.md) | Planned |
| [AtomBombing](techniques/shellcode-injection/atombombing/README.md) | Planned |
| [Threadless Injection](techniques/shellcode-injection/threadless-injection/README.md) | Planned |

### PE Injection

| Technique | Status |
| --- | --- |
| [Classic DLL Injection](techniques/pe-injection/dll-injection/README.md) | **Implemented** · [Proof](docs/runs/2026-11-11-technique-01.md) |
| [Reflective DLL Injection](techniques/pe-injection/reflective-dll-injection/README.md) | Planned |
| [Manual Mapping](techniques/pe-injection/manual-mapping/README.md) | Planned |
| [PE Image Injection](techniques/pe-injection/pe-image-injection/README.md) | Planned |
| [DLL Hollowing](techniques/pe-injection/dll-hollowing/README.md) | Planned |
| [Module Stomping](techniques/pe-injection/module-stomping/README.md) | Planned |

### Process Image Manipulation

| Technique | Status |
| --- | --- |
| [Process Hollowing](techniques/process-image-manipulation/process-hollowing/README.md) | Planned |
| [Transacted Hollowing](techniques/process-image-manipulation/transacted-hollowing/README.md) | Planned |
| [Process Doppelgänging](techniques/process-image-manipulation/process-doppelganging/README.md) | Planned |
| [Process Ghosting](techniques/process-image-manipulation/process-ghosting/README.md) | Planned |
| [Process Herpaderping](techniques/process-image-manipulation/process-herpaderping/README.md) | Planned |

This group includes process-creation and image-replacement techniques that are not necessarily remote injection into an existing process.

### Managed Code Injection

| Technique | Status |
| --- | --- |
| [.NET Assembly Injection](techniques/managed-code-injection/dotnet-assembly-injection/README.md) | Planned |
| [CLR Hosting](techniques/managed-code-injection/clr-hosting/README.md) | Planned |

[Full technique index](techniques/README.md)

## Build and test

**Requirements:** Windows x64, CMake 3.20+, Visual Studio C++ build tools.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The build stages the `poc01` target, injector, two DLL payloads and self-test under `tools/bin/`. See [bundle contents](tools/bin/README.md) and the [usage guide](techniques/pe-injection/dll-injection/README.md#usage).

There is no GitHub Actions CI. A documentation or directory change is not a new runtime validation.

## Targets and tools

| Component | Purpose |
| --- | --- |
| [Basic Target](targets/basic-target/main.cpp) | Independent heartbeat process used for Technique 01 |
| [Multithreaded Target](targets/multithreaded-target/main.cpp) | Independent process with multiple worker threads |
| [Process Observer](tools/process-observer/README.md) | Read-only process, thread, module and memory metadata |
| [PE Inspector](tools/pe-inspect/README.md) | Read-only PE headers, sections and imports |
| [Process Memory](tools/process-memory/README.md) | Memory fundamentals and region queries; **not injection** |
| [DLL Loading Demo](tools/dll-load-demo/README.md) | Normal in-process DLL loading; **not injection** |
| [DLL Host](tools/dll-host/README.md) | Explicit sample DLL loader |
| [Marker DLL](samples/marker-dll/marker.cpp) | Sample exported functions |

## Evidence policy

- Targets must not contain prearranged payload-loading mechanisms for the technique being tested.
- Record the actual mechanism, negative controls, outcomes and technical limitations.
- Distinguish compilation, module presence and confirmed payload execution.
- Report incomplete verification as incomplete; do not label a simulation as an injection PoC.
- Preserve historical run records, hashes and screenshots without rewriting past results.

[Experiment protocol](docs/experiment-protocol.md) · [Run records](docs/runs/README.md) · [Windows build notes](docs/build-validation.md)

---

[MIT License](LICENSE)
