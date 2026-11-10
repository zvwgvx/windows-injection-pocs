# Research implementation requirements

Each technique has a dedicated source directory. The technique status must accurately reflect whether executable code has been implemented and tested.

## Independent targets

Targets do not expose a prearranged loading command, named event, RPC endpoint, signal channel, or other cooperation mechanism to receive a payload. A baseline demonstrating voluntary DLL loading is a support tool, not an injector PoC.

## Evidence of implementation

Document the precise mechanism, build output, execution requirements, observable effect and limitations. Distinguish code that runs in a target process, code voluntarily loaded by that process, and true cross-process execution.

Neither a file named `injector.exe` nor a returned marker proves process injection. Planned experiments must not be presented as operational.

## Validation

Record Windows build, architecture, compiler, test target, input and outcome. Separate successful compilation from successful execution. Use owned sample processes.
