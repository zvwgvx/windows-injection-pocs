# 00 — PE format and loader fundamentals

**Status:** Implemented as a read-only inspector with source in [code](code/).

`pe-inspect` examines MZ and PE signatures, architecture, PE32/PE32+ format, image base, entry-point RVA, sections and imported DLL names. It does not load a PE, create a process or inject code.

```powershell
.\build\bin\pe-inspect.exe .\build\bin\basic-target.exe
.\build\bin\pe-inspect.exe .\build\bin\marker-dll.dll
```

The inspector can also build on macOS/Linux and inspect Windows PE files offline. This is a foundation utility, not an injection technique; it does not produce an injector/payload pair.
