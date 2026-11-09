# 02 — Process memory and permissions

**Status:** Implemented as two independent C++ programs.

## memory-layout.exe

A local process allocates a page, writes a harmless marker, changes access from read/write to read-only and restores protection before release.

```powershell
.\build\bin\memory-layout.exe
```

Expected output includes the process and thread IDs, the allocation base address, protection values before and after `VirtualProtect`, and successful release. No executable pages or remote writes are created.

## memory-regions.exe

A read-only region enumerator that queries committed virtual-memory regions for a supplied PID.

```powershell
.\build\bin\basic-target.exe --duration-sec 60
.\build\bin\memory-regions.exe 4321
```

Replace `4321` with the PID reported by `basic-target`. `memory-regions` uses `OpenProcess(PROCESS_QUERY_INFORMATION)` and `VirtualQueryEx` without reading or modifying the process contents.

## Expected observations

- `MEM_PRIVATE`, `MEM_IMAGE` and `MEM_MAPPED` may all be present.
- Memory protections can be examined without injecting code.
- The result represents a snapshot; concurrent allocations can change while enumerating.

## Limitations

Protected processes may refuse access. The tool lists region metadata, not the data contained in those regions, and it does not prove injection.
