# Process Observer

A read-only Windows x64 process telemetry tool. It inspects process path, thread IDs, loaded modules and aggregated memory-region metadata. It does not inject, hook, alter memory or communicate with the target.

## Usage

```powershell
.\build\bin\process-observer.exe --self
.\build\bin\process-observer.exe --pid 1234 --samples 5 --interval-ms 1000
```

The PID can be any accessible process. Use `--self` for a standalone smoke test. The output includes UTC timestamps and explicit enumeration error fields, so unavailable information is not treated as evidence of absence.

This is a supporting observation tool, not a DLL injector or a substitute for an actual technique implementation.
