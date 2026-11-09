# Experiment protocol

Each PoC is a standalone C++ source tree with its own CMake target, setup instructions and reproducible result.

## Record

- Technique and exact variant
- Windows edition/build, architecture and toolchain
- Target executable and its SHA-256
- Inputs, invocation and expected output
- Observed process, thread, module and memory state
- Limitations and failure conditions
- Optional forensic observations

## Reproducibility

Run experiments only against owned sample processes in an isolated VM. Record a baseline before the experiment, run one experiment at a time, collect evidence, and reset to a known state. Do not classify a PoC as implemented merely because its research directory exists.
