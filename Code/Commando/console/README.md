# Dedicated server console

`renegadeserver` is a Windows console executable. Launch it from a terminal to
inherit that terminal, or redirect stdin/stdout/stderr using ordinary shell
redirection. It does not allocate or detach a console during a normal launch.
The game client, including its `--nodx` console, retains its existing behavior.

The existing `--slave` mode is the exception: the child selects a private Windows
console after argument parsing and before installing control handlers. This
works whether the master launched it through Win32 or SDL3. The private console
owns the slave's input; it does not compete for the master's command stream.

Commands are submitted with Enter. Basic backspace editing is supported, with
no prompt, completion, colors, or cursor-based status display. The existing
commands, disk log, and server-control forwarding remain in use. Output keeps
flowing while a command is being typed. `profile`, `quick_stats`/`qs`, and `stats`
report that the interactive display is unavailable.
Progress messages that previously overwrote a line now end with a newline.

Input accepts UTF-8 bytes, LF or CRLF, and at most 200 bytes per command (excluding
the delimiter). Oversized lines are rejected through their delimiter, never
executed as truncated commands. Empty/whitespace-only lines are ignored. EOF
submits one final nonempty line, then permanently disables input polling for
that host session. Closing or omitting stdin does **not** stop the server.
Redirected output contains UTF-8 bytes without input echo or terminal formatting.
Attached Windows terminals use Unicode console APIs. Windows argument conversion
is shared with the client wrapper, preserving quoting and UTF-8 argv; downstream
engine filesystem handling is outside this change.

Ctrl+C and Ctrl+Break request the existing orderly `quit` path, which disables
automatic restart. The callbacks only set a flag; the game thread handles the
request once. The portable host also handles SIGINT/SIGTERM on POSIX. Shutdown
restores host-modified console modes and signal/control handlers. Inherited
streams remain open. As before, closing a Windows console window is distinct
from Ctrl+C; no new service or session-shutdown behavior is introduced here.

## Standalone build and tests

The module needs a C++20 compiler and CMake 3.25+. Tests additionally need Python
3. It has no game, DirectX, SDL, or asset dependency:

```sh
cmake -S Code/Commando/console -B build/console -DCMAKE_BUILD_TYPE=Release
cmake --build build/console --config Release --parallel
ctest --test-dir build/console -C Release --output-on-failure
```

The standalone CI job runs this on Windows x64 and Linux. Unit and subprocess
tests cover fragmented input, UTF-8, editing, line limits, EOF, absent stdin,
continued ticks, redirection, handler restoration, and actual interrupts. Windows
control-event tests run in an isolated hidden console; they never signal the
terminal running CTest. They also exercise private-console ownership and Unicode
input/output. The regular Windows game builds check the linked PE subsystem:
GUI for `renegade`, console for `renegadeserver`.

**Linux validation covers this console module, not a runnable Linux server.**
Engine startup, dialogs and keypress waits in failure paths, movie fallbacks,
renderer/UI isolation, general exit codes, scripts, filesystem portability,
networking, and service installation remain separate work.

## Integration and manual smoke checks

`ConsoleHost` borrows standard handles and is owned by dedicated `main`.
`ConsoleModeClass` remains the game-facing facade. Each game-thread tick polls a
bounded input batch and dispatches completed lines through
`ConsoleFunctionManager::Parse_Input`. There is no input reader thread.
`cprintf` writes through the host and forwards to server control once; existing
callers retain responsibility for disk logging.

With an existing asset installation and LAN server configuration, verify both
Win32 and SDL3 builds:

1. Launch from a terminal with quoted `--ini`, `--gamedir`, and `--startserver`
   paths. Check `help`, `game_info`, profiling requests followed by `help`, and
   `quit`. Verify `renlog` output and that quit does not restart the server.
2. Repeat with stdout/stderr redirected. Type a partial command and check that
   normal output continues. Close stdin and confirm the server stays alive.
3. Restart and use Ctrl+C; check orderly cleanup and disabled automatic restart.
4. Launch a slave and confirm it has its own console, input goes to the intended
   process, and the master still receives its own commands.

Startup failure paths may still show dialogs or wait for a key. Those paths are
explicitly deferred; they are not evidence that the host's input polling blocks.
