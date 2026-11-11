# Technique index

Directory paths are organized by primary payload class or process-image mechanism; this is a research taxonomy rather than a claim that execution mechanisms cannot overlap across classes.

## Shellcode Injection
- [Remote Thread](shellcode-injection/remote-thread/) — Planned
- [APC Injection](shellcode-injection/apc-injection/) — Planned
- [Early Bird APC](shellcode-injection/early-bird-apc/) — Planned
- [Thread Hijacking](shellcode-injection/thread-hijacking/) — Planned
- [Section Mapping](shellcode-injection/section-mapping/) — Planned
- [Threadpool Injection](shellcode-injection/threadpool-injection/) — Planned
- [AtomBombing](shellcode-injection/atombombing/) — Planned
- [Threadless Injection](shellcode-injection/threadless-injection/) — Planned

## PE Injection
- [Classic DLL Injection](pe-injection/dll-injection/) — **Implemented: poc01**, [Windows run record](../docs/runs/2026-11-11-technique-01.md)
- [Reflective DLL Injection](pe-injection/reflective-dll-injection/) — Planned
- [Manual Mapping](pe-injection/manual-mapping/) — Planned
- [PE Image Injection](pe-injection/pe-image-injection/) — Planned
- [DLL Hollowing](pe-injection/dll-hollowing/) — Planned
- [Module Stomping](pe-injection/module-stomping/) — Planned

## Process Image Manipulation
- [Process Hollowing](process-image-manipulation/process-hollowing/) — Planned
- [Transacted Hollowing](process-image-manipulation/transacted-hollowing/) — Planned
- [Process Doppelgänging](process-image-manipulation/process-doppelganging/) — Planned
- [Process Ghosting](process-image-manipulation/process-ghosting/) — Planned
- [Process Herpaderping](process-image-manipulation/process-herpaderping/) — Planned

## Managed Code Injection
- [.NET Assembly Injection](managed-code-injection/dotnet-assembly-injection/) — Planned
- [CLR Hosting](managed-code-injection/clr-hosting/) — Planned

## Notes

APC execution, section mapping, thread hijacking and module stomping can overlap several payload categories. A PoC should document both its payload type and execution trigger. Some process-image-manipulation techniques are process-creation techniques rather than remote injection.

The former `02-process-memory` is now a supporting utility at [tools/process-memory](../tools/process-memory/).

[Evidence protocol](../docs/experiment-protocol.md)
