# PoC layout and validation

Every numbered topic contains a `code/` directory. Incomplete techniques are identified as planned and must not be presented as implemented.

## Artifact contract

Each completed injection/loader experiment has independently buildable source files and, where technically applicable, produces:

```text
build/bin/<id>-<technique>/target.exe
build/bin/<id>-<technique>/injector.exe
build/bin/<id>-<technique>/payload.dll
```

The exact activation mechanism, supported target and technical limitations must be documented. In particular, a cooperative target-triggered DLL load must not be described as remote process injection.

Read-only PE inspection and process-memory enumeration are foundational utilities rather than injection techniques. They are not required to produce artificial injector or payload files.

## Validation record

Each implemented PoC should identify the Windows version, compiler, target PID, executable architecture, input conditions, expected result and observed result.

Use owned sample processes. Keep the ground truth separate from optional forensic interpretations. Distinguish cross-compilation, native execution, and successful payload invocation.
