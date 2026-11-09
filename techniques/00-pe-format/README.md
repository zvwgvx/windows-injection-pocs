# 00 — PE format and loader fundamentals

**Status:** Read-only PE inspector implemented in C++20.

The standalone `pe-inspect` program validates basic MZ and PE signatures and displays machine architecture, PE32/PE32+ format, image base, entry-point RVA, subsystem, section metadata and imported DLL names.

```powershell
.\build\bin\pe-inspect.exe .\build\bin\basic-target.exe
.\build\bin\pe-inspect.exe .\build\bin\marker-dll.dll
```

The same source also builds on macOS/Linux, where it can inspect PE files without running them.

The utility is intentionally read-only. It is a structural inspector, not an executable loader or a guarantee that every malformed or packed file will be classified correctly.

## Exercise

Compare `basic-target.exe`, `dll-host.exe`, and `marker-dll.dll` with respect to entry point, sections, imports, PE characteristics and mapped image size. Rename a copy of one file to `.dat` and check that its PE headers remain unchanged.
