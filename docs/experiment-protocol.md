# Research implementation and evidence requirements

## Scope

All builds and runtime observations target Windows x64. There is no hosted CI workflow.

For every technique, separate the source, test process, payload (if applicable), resulting artifacts and independent evidence. A technique remains **Planned** unless its actual effect has been demonstrated under Windows and its mechanism has been described accurately.

## Cooperative observation

The researcher may cooperate with the person running the target to record PID, program version, start and stop times, diagnostic output and independent process snapshots.

The target does not expose an injector-specific API, named event, command channel, loading hook or callback. Observability is not permission or a mechanism for executing a payload. Any experimental target that voluntarily loads a DLL is a **cooperative loader** and must be described as such.

## Objective observations

The `process-observer` program records the target's process identity, modules, thread IDs and aggregated virtual-memory attributes without reading or changing the target's memory contents.

When a tool fails to open or inspect a process, record the Windows error instead of treating missing data as evidence of absence. Changes in module count or executable memory alone do not prove malicious injection.

## Reporting

For each run, record build revision, Windows version, compiler, target executable and PID, observation timestamps, source/output paths, tool exit statuses, actual output and limitations.

Distinguish direct observations from inferences. Avoid claiming that an injection technique is implemented just because a payload, demo binary or marker exists.
