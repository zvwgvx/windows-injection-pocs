# 01 — DLL Injection

**Status:** Implemented — runtime evidence recorded in [docs/runs/2026-11-11-technique-01.md](../../docs/runs/2026-11-11-technique-01.md).

This experiment uses the classic remote `LoadLibraryW` sequence. It does not use a
named event, pipe, RPC endpoint, window message, hook, target export, command-line
switch, or environment variable to make the target load the payload.

## Validation status

Runtime evidence was recorded on Windows x64 on 2026-11-11: the full
absence → inject → presence → duplicate-refusal chain passed on the real target, with
the payload log verified from inside the target. See
[docs/runs/2026-11-11-technique-01.md](../../docs/runs/2026-11-11-technique-01.md).

## Components

| File | Role |
| --- | --- |
| `code/poc01-injector.cpp` | Opens the target, allocates/writes the DLL path, starts a remote thread at `LoadLibraryW`, and verifies the result |
| `code/poc01-payload.cpp` | Ordinary DLL. `DllMain` writes a PID/base-stamped evidence line to `%TEMP%\wininj-poc01\poc01-payload-<pid>.log` |
| `code/poc01-payload-heartbeat.cpp` | Payload variant that keeps appending a heartbeat line while loaded; both variants satisfy the same log contract |
| `code/poc01-selftest.cpp` | Negative control, injection, postcondition, duplicate-refusal, and target-health checks |

The target programs under [`targets/`](../../targets/) are unchanged. They have no
injector-aware code, callback, event, or loader interface.

## Mechanism

The injector performs the following operations against an already-running x64
process:

1. `OpenProcess` with `PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
   PROCESS_VM_OPERATION | PROCESS_VM_WRITE`.
2. A read-only `Toolhelp32` module snapshot confirms that the exact payload path is
   not already loaded. A same-named module from another path also causes refusal.
3. `VirtualAllocEx` + `WriteProcessMemory` place the full DLL path in the target.
4. The injector resolves the local `LoadLibraryW` export, finds the module that
   contains it (`kernel32.dll` or `kernelbase.dll`), and computes the same offset in
   the target's copy of that module.
5. `CreateRemoteThread` calls the remote `LoadLibraryW` with the remote path.
6. After the thread completes, a second module snapshot must show the payload at
   the exact full path.
7. The payload's `DllMain` log must be fresh and contain both the target PID and the
   observed module base.

Before injection, the injector removes a previous per-PID evidence file so stale
content cannot satisfy the log check. File freshness and the PID/base tokens are
still verified after the remote thread completes.

The injector's exit status is success only when all checks pass. A loaded module by
itself is not treated as sufficient proof.

## Why this is not cooperative injection

- The target source is not modified and is never rebuilt for this technique.
- The target does not create an event, pipe, socket, RPC server, window message
  handler, hook, or exported "load DLL" function.
- The injector receives only a PID and a DLL path. It does not ask the target for
  permission or wait for a target-side signal.
- The self-test may launch the target and read its PID from `CreateProcessW`. That is
  ground-truth process identity, not a target-side loading protocol.
- Before injection, `--check-only` must report the payload as absent. After
  injection, a second injector run must refuse with exit code 6. Those two negative
  controls make a prearranged load visible.

## Usage

Build on Windows x64, then start an independent target:

```powershell
.\tools\bin\basic-target.exe --duration-sec 60
```

Read the PID printed by the target. Confirm that the payload is absent:

```powershell
.\tools\bin\poc01-injector.exe --pid 1234 --dll .\tools\bin\poc01-payload.dll --check-only
# exit code 3 = absent
```

Inject and verify:

```powershell
.\tools\bin\poc01-injector.exe --pid 1234 --dll .\tools\bin\poc01-payload.dll
```

Expected key output:

```text
module_before=absent
provider_module=kernelbase.dll
remote_thread_id=....
module_after=present
module_path_match=true
payload_log=...\wininj-poc01\poc01-payload-1234.log
payload_log_fresh=true
payload_log_pid_match=true
payload_log_base_match=true
result=success
```

The payload remains loaded until the target exits. The injector does not call
`FreeLibrary` in the target.

## Exit codes

| Code | Meaning |
| --- | --- |
| `0` | Injection and full evidence chain succeeded, or `--check-only` found the DLL |
| `1` | Invalid command line |
| `2` | Win32/process/snapshot/thread error |
| `3` | `--check-only` found the DLL absent |
| `4` | Module appeared, but the evidence chain was incomplete (freshness/PID/base/path) |
| `5` | Remote thread completed but the module was not found |
| `6` | Payload was already loaded, or a same-named module exists from another path; no injection attempted |

## Test

```powershell
cmake --build build --config Release
ctest --test-dir build -C Release -R dll-injection-smoke --output-on-failure
```

`dll-injection-smoke` runs the actual injector against `basic-target.exe`, checks the
absence/presence transition, rejects a duplicate injection, and confirms the target
is still alive. It requires same-user x64 Windows. Windows Defender or another EDR
may block `CreateRemoteThread`; use a lab VM and record the block instead of
weakening the evidence chain.

## Limits

- Same-user, same-session, same-integrity x64 target only. No privilege escalation.
- `CreateRemoteThread` is a well-known, heavily monitored API. This PoC does not
  implement evasion, persistence, process discovery, or payload staging.
- The evidence log path is under the user's `%TEMP%`. If the target runs as a
  different user, the injector may not see the log and will report verification
  failure rather than success.
- The payload writes a small file from `DllMain` while the loader lock is held. That
  is acceptable for this controlled experiment but is not a template for production
  DLLs.
- A successful run proves this mechanism under the recorded Windows build. It does
  not prove that the technique is stealthy or universally reliable.

Use only against processes and binaries you own or are explicitly authorized to
test.
