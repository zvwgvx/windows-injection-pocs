# Windows Injection PoCs

A C++20 research collection of Windows x64 injection and process-image experiments, classified by **payload type** and **execution mechanism**.

**Author:** [zvwgvx](https://github.com/zvwgvx)

**Platform:** Windows x64 only

## Repository structure

```text
windows-injection-pocs/
├── techniques/
│   ├── shellcode-injection/
│   ├── pe-injection/
│   │   └── dll-injection/
│   ├── process-image-manipulation/
│   └── managed-code-injection/
├── targets/
├── tools/
│   ├── pe-inspect/
│   ├── process-memory/
│   ├── process-observer/
│   ├── dll-host/
│   └── dll-load-demo/
├── samples/
├── payloads/
├── docs/
└── CMakeLists.txt
```

The original global numeric directory names have been replaced by a four-group taxonomy. Techniques such as APC, thread hijacking, section mapping and module stomping may operate on several payload types; the location is their **primary catalogue entry**, not an exclusive constraint.

[Full technique index and implementation status](techniques/README.md)

## Implemented: Classic DLL Injection

The existing **[Classic DLL Injection PoC](techniques/pe-injection/dll-injection/README.md)**, previously called Technique 01, uses `CreateRemoteThread` + `LoadLibraryW` against an unchanged target. Its source and self-test are moved without rewriting their code.

The stable `poc01` filenames are retained to preserve links to the original [Windows run record](docs/runs/2026-11-11-technique-01.md), SHA-256 hashes, recorded output and System Informer screenshot. That historical run record and screenshot remain unmodified.

**All other injection techniques are Planned**. Empty code directories are not implementations or proof of execution.

## Targets and tools

`targets/basic-target` and `targets/multithreaded-target` are independent Windows processes. They do not expose an injector-specific loader channel.

`tools/process-memory` is a foundation utility rather than an injection technique. `tools/process-observer` and `tools/pe-inspect` provide read-only observations. The normal DLL loader demos remain under `tools/`.

## Build and test

Windows x64, CMake 3.20+ and Visual Studio C++ build tools:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The project still stages Technique 01's target, injector, payloads and self-test under `tools/bin/`.

See the [Technique 01 README](techniques/pe-injection/dll-injection/README.md) and its [runtime evidence](docs/runs/2026-11-11-technique-01.md). There is no GitHub Actions CI. A directory refactor alone is not additional runtime validation.

## Research standards

Successful injection requires an independent target, the real execution mechanism and reproducible observations. A cooperative target loading a DLL is not remote injection. See the [experiment protocol](docs/experiment-protocol.md).

## License

MIT. See [LICENSE](LICENSE).
