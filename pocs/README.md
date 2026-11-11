# PoC Catalogue and Metadata

The canonical [catalogue in the root README](../README.md#poc-catalogue) links to every technique, source set and verified proof.

Each PoC lives at `pocs/NN-short-name/`, with a two-digit stable ID, a technique README and a `poc.yaml` metadata file. IDs identify experiments, not technical categories. There is no hierarchy by payload type.

## Tags

`tags.payload`, `tags.mapping`, `tags.execution` and `tags.target` are arrays because one implementation can belong to multiple conceptual groups. Use `varies` only when a planned technique has no fixed implementation details; replace it with specific tags when source and execution evidence establish those details.

`status: implemented` requires an actual recorded Windows runtime test. For planned experiments, `proof: null`. Evidence records belong under [docs/runs](../docs/runs/README.md), and original captures must be retained.

The existing `poc01` executable and source filenames remain unchanged, along with the original [Technique 01 runtime record](../docs/runs/2026-11-11-technique-01.md).
