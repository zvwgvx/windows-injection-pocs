# 01 — Normal DLL loading

**Status:** Implemented as a standalone, opt-in loading experiment.

## Components

- `dll-load-demo.exe`: a separate C++ program using Windows' normal DLL loader.
- `marker-dll.dll`: a harmless DLL exporting `LabMarker` and `LabIncrement`.
- `dll-host.exe`: a target that can hold the same DLL in memory for inspection.

## Windows x64

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
.\build\bin\dll-load-demo.exe "$PWD\build\bin\marker-dll.dll"
.\build\bin\dll-host.exe "$PWD\build\bin\marker-dll.dll" --hold-ms 15000
```

The output directory may differ with the CMake generator. Each executable validates both exported functions and prints its own PID, the loaded module base and a marker result.

The DLL is deliberately loaded by the host itself. This experiment demonstrates normal image loading and should not be labeled remote DLL injection.

## Expected observations

- A DLL image mapped into the calling process
- An exported marker that returns deterministic data
- Module state visible while the target holds it open
- An orderly `FreeLibrary` on completion

## Limitations

Loading failure, invalid DLL architecture, missing exports and insufficient privileges can change the outcome. A successful DLL load is not evidence of malicious behavior.
