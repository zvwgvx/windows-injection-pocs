# Normal DLL loading baseline

This utility explicitly loads a DLL into its own process using the Windows loader. It obtains exported functions and unloads the DLL afterward.

It is not a process injector, and is not part of the process injection technique catalogue.

```powershell
.\build\bin\dll-load-demo.exe .\build\bin\marker-dll.dll
```
