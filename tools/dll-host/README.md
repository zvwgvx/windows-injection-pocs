# DLL Host

A supporting sample program that voluntarily loads the benign marker DLL for inspection.

```powershell
.\build\bin\dll-host.exe .\build\bin\marker-dll.dll --hold-ms 10000
```

This is a loader baseline, not an injection target. Independent targets are located in the repository's `targets/` directory.
