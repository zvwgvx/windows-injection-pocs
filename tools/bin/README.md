# Technique 01 run bundle

Generated copies of the files needed to run the technique 01 experiment on a Windows
x64 machine. Not sources; ignored by the repository's binary rules (`*.exe`, `*.dll`).

| File | Role |
| --- | --- |
| `basic-target.exe` | Independent heartbeat target, unmodified |
| `poc01-injector.exe` | The injector |
| `poc01-payload.dll` | Payload variant 1: one evidence line on attach |
| `poc01-payload-heartbeat.dll` | Payload variant 2: evidence on attach plus a heartbeat line every 500 ms while loaded |
| `poc01-selftest.exe` | Full self-test: absence check, injection, presence check, duplicate refusal |

The executables are statically linked, so the folder runs as-is on a plain Windows
x64 host (Windows 10/11) with no extra runtime DLLs. Rebuild the project to refresh;
the canonical CMake output remains `build/bin/`.

One-command validation:

```powershell
.\poc01-selftest.exe --target .\basic-target.exe --injector .\poc01-injector.exe --payload .\poc01-payload.dll
```
