# TOOLS.md - Local Notes

Skills define _how_ tools work. This file is for _your_ specifics — the stuff that's unique to your setup.

## What Goes Here

Things like:

- Camera names and locations
- SSH hosts and aliases
- Preferred voices for TTS
- Speaker/room names
- Device nicknames
- Anything environment-specific

## Examples

```markdown
### Cameras

- living-room → Main area, 180° wide angle
- front-door → Entrance, motion-triggered

### SSH

- home-server → 192.168.1.100, user: admin

### TTS

- Preferred voice: "Nova" (warm, slightly British)
- Default speaker: Kitchen HomePod

### GitHub

- Account: Dadud
- gh CLI: authenticated via token
- SSH key: authenticated to GitHub
- Default protocol: SSH (`git@github.com`)
- Repo: `Dadud/OpenW3D` (origin), `w3dhub/OpenW3D` (upstream)
- Active branch: `bgfx-integration`
```

### Models

- **Main session:** kimi/kimi-code
- **Coder subagent:** minimax/MiniMax-M2.7-highspeed (efficient, fast)
- **SSH:** Authenticated to GitHub
- **Git protocol:** SSH (git@github.com)

### Coder Subagent Prompting (minimax/MiniMax-M2.7-highspeed)

This model is fast and capable for code tasks. Prompt it with:

1. **Concrete file paths** — always specify exact paths like `Code/ww3d2/dx8wrapper.h`
2. **Explicit constraints block** — what NOT to do is as important as what TO do
3. **Step-by-step breakdown** — numbered list of expected changes
4. **Verification requirement** — "build and confirm compiles" or "run tests"
5. **Context about the codebase** — mention relevant headers, build system, target platform

**Template for spawning coder subagents:**

```
Work in /tmp/openw3d-analyze on branch <branch>.

**Goal:** <one-line description>

**Files to modify:**
- path/to/file.cpp — <what to change>

**Constraints:**
- Do NOT change function signatures that are used by other modules
- Do NOT add new dependencies without asking
- Keep changes minimal and focused

**Steps:**
1. Read the current implementation
2. Apply the fix
3. Build to verify: `make <target>`
4. Report what changed and why

**Key context:** <relevant background about the codebase>
```

**Effective patterns:**
- Give it 1-3 files max per subagent task (prevents context overflow)
- Include the exact error message if fixing a compile failure
- Specify the build command to verify with
- Ask it to return a git diff summary, not just "it's fixed"

---

Add whatever helps you do your job. This is your cheat sheet.

## Related

- [Agent workspace](/concepts/agent-workspace)
