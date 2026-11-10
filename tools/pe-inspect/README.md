# PE Format Inspector

**Status:** Implemented as a read-only C++ inspector with source in [code](code/).

`pe-inspect` examines MZ and PE signatures, architecture, PE32/PE32+ format, image base, entry-point RVA, sections and imported DLL names. It does not load a PE, create a process or inject code.

```powershell
.\build\bin\pe-inspect.exe .\build\bin\basic-target.exe
.\build\bin\pe-inspect.exe .\build\bin\marker-dll.dll
```

It is a Windows supporting tool, not an injection technique, and does not produce an injector/payload pair.
