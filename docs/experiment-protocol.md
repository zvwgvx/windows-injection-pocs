# Research implementation and evidence requirements

## Scope

All builds and runtime observations target Windows x64. There is no hosted CI workflow.

For every technique, separate the source, test process, payload (if applicable), resulting artifacts and independent evidence. A technique remains **Planned** unless its actual effect has been demonstrated under Windows and its mechanism has been described accurately.

## Cooperative observation

The researcher may cooperate with the person running the target to record PID, program version, start and stop times, diagnostic output and independent process snapshots.

The target does not expose an injector-specific API, named event, command channel, loading hook or callback. Observability is not permission or a mechanism for executing a payload. Any experimental target that voluntarily loads a DLL is a **cooperative loader** and must be described as such.

## Non-cooperative injection evidence

A cross-process injection claim is accepted only when the target remains an ordinary
process and every step is externally initiated. A test harness may launch the target
and learn its PID from the operating system; that is ground-truth process identity,
not a target-side loading protocol. The harness must not ask the target to load the
payload, wait for a target signal, or modify the target binary.

For technique 01, the minimum evidence chain is:

1. An independent module snapshot reports the exact payload path as **absent**
   before the injector runs. A same-named module from another path also causes a
   refusal.
2. The injector performs the memory write and remote thread creation itself; the
   target has no matching event, pipe, RPC endpoint, hook, export, or environment
   variable.
3. A post-injection module snapshot reports the payload at the exact full path.
4. The payload records a fresh line containing the target PID and observed module
   base; the injector verifies both.
5. A duplicate injection is refused, and the target is still alive after the
   experiment.

If any link is missing, the result is reported as a verification failure rather than
as successful injection.

## Objective observations

The `process-observer` program records the target's process identity, modules, thread IDs and aggregated virtual-memory attributes without reading or changing the target's memory contents.

Third-party process tools that are independent of this repository's toolchain — for example System Informer or Process Explorer — are acceptable corroborating observers for the module-preservation check: a module reported by the injector should also be visible there, with the same path and base address. Note that these tools read the same OS loader data as the injector's snapshot, so they confirm the mapping independently but do not by themselves prove payload code execution; the payload-side evidence log remains the execution-side link of the chain.

When a tool fails to open or inspect a process, record the Windows error instead of treating missing data as evidence of absence. Changes in module count or executable memory alone do not prove malicious injection.

## Reporting

For each run, record build revision, Windows version, compiler, target executable and PID, observation timestamps, source/output paths, tool exit statuses, actual output and limitations.

Distinguish direct observations from inferences. Avoid claiming that an injection technique is implemented just because a payload, demo binary or marker exists.
