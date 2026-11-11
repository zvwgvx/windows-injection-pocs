# Run record template

Copy this template to `YYYY-MM-DD-technique-NN.md` and fill in every field for one
run. Do not delete fields; write `not recorded` if something was not captured. A run
with empty or missing fields does not validate the technique.

## Identity

| Field | Value |
| --- | --- |
| Date (UTC) | |
| Build revision (git commit) | |
| Host OS and build | |
| Compiler and version | |
| Build type | |
| Technique | |

## Artifacts used

| Field | Value |
| --- | --- |
| Target executable (full path, size, SHA-256) | |
| Payload binary (full path, size, SHA-256) | |
| Injector binary (full path, size, SHA-256) | |
| Target source last modified | |
| Payload/source relationship to `pocs/01-classic-dll-injection/code` | |

## Procedure

1. Command lines used (target, injector, self-test):
2. Target PID reported by the target itself:
3. Timeline (UTC): target start, precondition check, injection, postcondition check, target stop:

## Evidence

| Check | Expected | Observed |
| --- | --- | --- |
| Precondition `--check-only` exit code | 3 (absent) | |
| Injection exit code | 0 | |
| Postcondition `--check-only` exit code | 0 (present) | |
| Duplicate injection exit code | 6 (refused) | |
| Target still alive after experiment | yes | |
| `module_before` / `modules_before` | absent / N | |
| `module_after` / `module_path_match` | present / true | |
| `payload_log_fresh` / `pid_match` / `base_match` | true / true / true | |

## Raw output

Paste the full stdout/stderr of the injector and the self-test. Do not summarise.

## Deviations and limitations

List anything that differed from the documented procedure (for example, an EDR block,
a Defender exclusion, a failed first attempt). State explicitly what the run does and
does not prove.
