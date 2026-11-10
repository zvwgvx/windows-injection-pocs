# 01 — DLL Injection

**Status: Planned. No remote DLL injector has been implemented.**

The target processes for this research are independent programs under [targets](../../targets/). They do not host a named event, signal handler, RPC endpoint, or any other custom mechanism for another process to request DLL loading.

The earlier cooperative signal-based demonstration has been removed. That demo was target-initiated DLL loading and was not genuine injection.

Normal loading demonstrations are maintained separately under [tools/dll-load-demo](../../tools/dll-load-demo/) and [tools/dll-host](../../tools/dll-host/). The marker DLL remains under [samples/marker-dll](../../samples/marker-dll/).

No files in this directory are currently described as an operational injector.
