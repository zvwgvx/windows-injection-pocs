# Windows Injection PoCs

**A Windows x64 C++20 research collection of process-injection and process-image execution techniques.** Each PoC has a stable ID, independent documentation, multi-label metadata, and verifiable runtime evidence when implemented.

**Platform:** Windows x64 · **Build:** CMake · **Status:** 1 implemented / 21 catalogued · **CI:** None

[PoC Catalogue](#poc-catalogue) · [Verified PoC](#verified-poc-01) · [Build](#build--test) · [Metadata](#metadata-and-tags) · [Research Standards](#research-standards)

## PoC Catalogue

A flat index: **one PoC per directory**, with tags describing the payload, mapping/loading method, execution trigger and target type. These axes are not mutually exclusive.

| ID | PoC / Technique | Tags | Status |
| :--: | --- | --- | --- |
| 01 | [Classic DLL Injection](pocs/01-classic-dll-injection/README.md) | `pe` · `dll` · `windows-loader` · `remote-thread` | **Implemented** · [Proof](docs/runs/2026-11-11-technique-01.md) |
| 02 | [PE Image Injection](pocs/02-pe-image-injection/README.md) | `pe` · `pe-image` · `varies` | Planned |
| 03 | [Manual Mapping](pocs/03-manual-mapping/README.md) | `pe` · `dll` · `manual-mapping` · `varies` | Planned |
| 04 | [Reflective DLL Injection](pocs/04-reflective-dll-injection/README.md) | `pe` · `dll` · `reflective-loader` · `varies` | Planned |
| 05 | [DLL Hollowing](pocs/05-dll-hollowing/README.md) | `pe` · `dll` · `module-replacement` · `varies` | Planned |
| 06 | [Module Stomping](pocs/06-module-stomping/README.md) | `shellcode` · `pe` · `module-replacement` · `varies` | Planned |
| 07 | [Remote Thread Injection](pocs/07-remote-thread/README.md) | `shellcode` · `private-memory` · `remote-thread` | Planned |
| 08 | [APC Injection](pocs/08-apc-injection/README.md) | `varies` · `apc` | Planned |
| 09 | [Early Bird APC Injection](pocs/09-early-bird-apc/README.md) | `varies` · `early-bird-apc` | Planned |
| 10 | [Thread Execution Hijacking](pocs/10-thread-hijacking/README.md) | `varies` · `thread-hijacking` | Planned |
| 11 | [Section Mapping Injection](pocs/11-section-mapping/README.md) | `shellcode` · `pe` · `section-mapping` · `varies` | Planned |
| 12 | [Threadpool Injection](pocs/12-threadpool-injection/README.md) | `varies` · `threadpool` | Planned |
| 13 | [AtomBombing](pocs/13-atombombing/README.md) | `shellcode` · `atom-table-assisted` · `varies` | Planned |
| 14 | [Threadless Injection](pocs/14-threadless-injection/README.md) | `varies` · `threadless` | Planned |
| 15 | [Process Hollowing](pocs/15-process-hollowing/README.md) | `pe` · `image-replacement` · `process-creation` | Planned |
| 16 | [Transacted Hollowing](pocs/16-transacted-hollowing/README.md) | `pe` · `transacted-image` · `process-creation` | Planned |
| 17 | [Process Doppelgänging](pocs/17-process-doppelganging/README.md) | `pe` · `transaction-backed-image` · `process-creation` | Planned |
| 18 | [Process Ghosting](pocs/18-process-ghosting/README.md) | `pe` · `deleted-file-backed-image` · `process-creation` | Planned |
| 19 | [Process Herpaderping](pocs/19-process-herpaderping/README.md) | `pe` · `modified-file-backed-image` · `process-creation` | Planned |
| 20 | [.NET Assembly Injection](pocs/20-dotnet-assembly-injection/README.md) | `managed-assembly` · `clr-loading` · `managed-code` | Planned |
| 21 | [CLR Hosting](pocs/21-clr-hosting/README.md) | `managed-assembly` · `clr-hosting` · `managed-code` | Planned |

*Planned means no validated implementation exists for that entry. Process image creation techniques may not be injection into a previously running process.*

## Verified PoC: 01

### Classic DLL Injection

The recorded experiment uses `CreateRemoteThread` + `LoadLibraryW` against a target with no injector-specific loader. Validation was performed on Windows 10 Pro 22H2 (x64); the historical evidence is retained without modification.

| Resource | Direct link |
| --- | --- |
| PoC documentation | [01 — Classic DLL Injection](pocs/01-classic-dll-injection/README.md) |
| Metadata | [`poc.yaml`](pocs/01-classic-dll-injection/poc.yaml) |
| Injector | [`poc01-injector.cpp`](pocs/01-classic-dll-injection/code/poc01-injector.cpp) |
| DLL payload | [`poc01-payload.cpp`](pocs/01-classic-dll-injection/code/poc01-payload.cpp) |
| Heartbeat payload | [`poc01-payload-heartbeat.cpp`](pocs/01-classic-dll-injection/code/poc01-payload-heartbeat.cpp) |
| Self-test | [`poc01-selftest.cpp`](pocs/01-classic-dll-injection/code/poc01-selftest.cpp) |
| Runtime proof | [Run record, raw output, hashes](docs/runs/2026-11-11-technique-01.md) |
| Independent evidence | [System Informer screenshot](docs/runs/assets/2026-11-11-technique-01-system-informer.png) |

The documented self-test demonstrated DLL absence before execution, module presence afterward, duplicate-injection refusal, and target survival. This evidence is specific to the recorded lab environment.

## Metadata and Tags

Every `pocs/NN-name/` directory contains `README.md` and `poc.yaml`. The latter is machine-readable and assigns **multiple** values when applicable:

| Field | Meaning |
| --- | --- |
| `id` | Stable two-digit PoC identifier |
| `status` | `planned` or `implemented` |
| `tags.payload` | Payload format, such as `pe`, `dll`, `shellcode`, or `managed-assembly` |
| `tags.mapping` | Loading or memory-mapping mechanism |
| `tags.execution` | Execution trigger or method |
| `tags.target` | `existing-process` or `new-process` |
| `proof` | Run-record path for a verified PoC; `null` otherwise |

A mechanism such as APC may execute different payload types. Its PoC is not automatically a PE or shellcode implementation until the actual payload is specified and validated. See the [catalogue and metadata guide](pocs/README.md).

## Build & Test

**Requirements:** Windows x64, CMake 3.20+, MSVC C++ build tools.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The build stages the existing `poc01` binaries under `tools/bin/`. See [bundle contents](tools/bin/README.md) and [usage](pocs/01-classic-dll-injection/README.md#usage). The repository has no GitHub Actions CI. A directory restructure alone is not a new Windows runtime validation.

## Targets and Tools

| Component | Purpose |
| --- | --- |
| [Basic Target](targets/basic-target/main.cpp) | Independent single-threaded target |
| [Multithreaded Target](targets/multithreaded-target/main.cpp) | Independent multi-worker target |
| [Process Observer](tools/process-observer/README.md) | Read-only module, thread and memory metadata |
| [Process Memory](tools/process-memory/README.md) | Memory fundamentals and region queries; not an injection technique |
| [PE Inspector](tools/pe-inspect/README.md) | PE header, section and import inspection |
| [DLL Loading Demo](tools/dll-load-demo/README.md) | In-process loading baseline; not remote injection |

## Research Standards

The target must not contain a prearranged injector-specific loading interface. Source, direct observations, negative controls, test conditions and limitations must be distinguished. Only a reproducible runtime record can justify `implemented` status.

[Experiment protocol](docs/experiment-protocol.md) · [Run records](docs/runs/README.md) · [Windows build validation](docs/build-validation.md) · [MIT License](LICENSE)
