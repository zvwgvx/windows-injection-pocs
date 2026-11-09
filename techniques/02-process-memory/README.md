# 02 — Process memory and permissions

**Status:** Implemented as two independent read-only/local-memory C++ utilities in [code](code/).

## memory-layout.exe

Allocates a page in its own process, writes a marker, changes the page from read/write to read-only, restores its original permissions and releases it.

```powershell
.\build\bin\memory-layout.exe
```

## memory-regions.exe

Lists committed virtual memory region metadata from a supplied PID using `OpenProcess(PROCESS_QUERY_INFORMATION)` and `VirtualQueryEx`.

```powershell
.\build\bin\basic-target.exe --duration-sec 60
.\build\bin\memory-regions.exe <reported-pid>
```

This foundation topic does not perform any injection, patching, executable memory allocation or payload execution. It does not produce an injector/payload pair.
