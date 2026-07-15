---
name: fpsjanggi-multiplayer-regression
description: Build and triage the FPSJanggi Unreal Engine 5.8 multiplayer flow. Use when changing C++ or Blueprint integration that affects listen-server/client startup, Blue/Red assignment, placed board cameras, formation selection, authoritative piece spawning, replication, board movement, arena transitions, or when a screenshot disagrees with expected multiplayer behavior.
---

# FPSJanggi multiplayer regression

## Workflow

1. Read `AGENTS.md`, run `git status -sb`, and preserve unrelated working-tree changes.
2. Inspect the newest `Saved/Logs/FPSJanggi.log` before changing code. Compare server and client roles rather than diagnosing only from a screenshot.
3. Build both targets after C++ changes:

   ```powershell
   & 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' FPSJanggiEditor Win64 Development 'C:\gege\FPSJanggi.uproject' -WaitMutex -NoHotReload -NoUBA
   & 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' FPSJanggi Win64 Development 'C:\gege\FPSJanggi.uproject' -WaitMutex -NoHotReload -NoUBA
   ```

4. Run `scripts/run_standalone_lobby_smoke.ps1` for the front-end, then `scripts/run_two_player_smoke.ps1` for network play. Use `-Arena` when testing collision and return-camera behavior.
5. Require the following evidence before calling the multiplayer path healthy:
   - Blue uses the placed camera tagged `Blue`; Red uses the placed camera tagged `Red`.
   - Both sides report 32 synchronized pieces.
   - The server reports `BOARD_MATCH_STARTED` and the smoke move reports `moved=true`.
   - The client reports `ROLE_SimulatedProxy` board state without `BOARD_CAMERA_FAILED`.
6. Classify failures by layer: startup/asset loading, team assignment, camera selection, formation RPC, authoritative spawn, replication, movement rule, or arena state.
7. Update `Docs/SystemImplementation.md` when behavior or the manual test checklist changes.

## Guardrails

- Treat `nsh` as the canonical integration core and replace conflicting legacy Blueprint/C++ behavior when required.
- Do not fetch, copy, or reconstruct another developer's branch unless the user explicitly requests that specific integration.
- Use `UnrealEditor-Cmd.exe -game` for uncooked smoke tests. A Development Game build proves compilation; the standalone game executable needs packaged/cooked content for runtime validation.
- Do not commit, push, or open a PR unless explicitly requested.
