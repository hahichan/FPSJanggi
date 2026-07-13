# FPSJanggi integration policy

- Treat the current `nsh` implementation as the canonical integration core.
- When legacy Blueprint or C++ behavior conflicts with the core systems, replace or overwrite it as needed instead of preserving it merely because it originated from another developer.
- Build the core independently on `nsh`. Other developers will adapt their work to this branch after it is published.
- Do not fetch, copy, or reconstruct another developer's branch content unless the user explicitly requests that specific integration.
- Do not modify or push another developer's remote branch such as `kkw` unless the user explicitly requests it.
- Preserve unrelated user-authored changes in the working tree and do not commit, push, or open a PR unless explicitly requested.
- Build both `FPSJanggiEditor` and `FPSJanggi` Win64 Development targets after C++ system changes. For uncooked multiplayer smoke tests, use `UnrealEditor-Cmd.exe -game` rather than launching the unpackaged game executable directly.
