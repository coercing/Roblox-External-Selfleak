# Roblox-External-Selfleak

This is an external, usermode cheat for **Roblox** (Windows x64), written in C++. It attaches to `RobloxPlayerBeta.exe` via direct process memory access using raw syscalls, then renders a transparent DirectX 11 overlay on top of the game window. All interaction with the game happens purely through memory reads and writes — no code is injected into the Roblox process.

> [!WARNING]
> **Read the disclaimer at the bottom of this document before using or distributing this software.**

---

## Features

### Aimbot
- **Mouse aim** — moves the physical cursor toward the target using `SendInput`
- **Camera aim** — directly writes the Roblox camera rotation matrix, bypassing mouse movement entirely
- **FOV circle** — configurable circular field-of-view limit with optional spinning/fill animation
- **Sticky aim** — locks onto the last acquired target by player name until they die or leave
- **Bone selection** — head, HumanoidRootPart, upper/lower torso, all limbs, closest-to-cursor, or random
- **Knocked check** — skips targets whose `BodyEffects/K.O` value is true
- **Visibility check** — raycasts from the camera to the target bone via the `WallCheck` OBB system; skips occluded targets
- **Shake/jitter** — adds configurable X/Y/Z noise to the aim position
- **Prediction** — applies a linear lead offset to compensate for target movement
- **Smoothing** — configurable smooth factor for both camera and mouse aim modes
- **Keybind + mode** — hold or toggle, any key

### Silent Aim
Spoofs the frame position of the fired projectile so it hits the target even when crosshairs are off.
- **Frame position spoof** — writes `FramePositionOffsetX/Y` on the active UI frame
- **Mouse spoof** — redirects mouse coordinates to the target bone screen position
- **Raycast redirect** — redirects the shot raycast direction toward the selected bone
- **Gun-based FOV** — separate FOV values for Double-Barrel, Tactical Shotgun, and Revolver (Phantom Forces–specific)
- **Sticky aim, knocked check, visibility check, keybind + mode** — same as aimbot

### ESP / Visuals
- **Bounding box** — flat, corner-bracket, or round-corner style; configurable colour, outline, and thickness
- **Box fill** — solid or gradient fill with animated rotation
- **Skeleton** — draws limb connections between all cached body parts
- **Head dot** — circle marker at head position
- **Snap lines** — lines from the bottom/top/center of screen to each player
- **Health bar** — vertical or horizontal bar, colour-shifts green → yellow → red
- **Health text** — numeric HP display
- **Name tag** — display name or username, with outline rendering
- **Distance** — shows metres to target
- **Rig type** — R6 / R15 indicator
- **Tool name** — shows currently held weapon/tool
- **Look direction** — arrow indicating which way the player is facing
- **Chams** — semi-transparent filled mesh overlay per-player (fetches `.mesh` data from Roblox asset delivery)
- **Hit effects** — hitmarker, ghost skeleton flash, or ghost chams on damage
- **Shot tracers** — line, beam, or dotted trail from the shot origin to impact
- **Hit notifications** — on-screen text pop when a hit registers
- **Render distance** — global cap on how far away targets are drawn
- **Radar** — configurable mini-map showing enemy dots and optional names/distances

### World / Lighting
- **Fog override** — sets fog end distance and colour; restores originals when disabled
- **Exposure override** — sets `ExposureCompensation` on the `Lighting` service
- **FOV override** — sets camera field of view

### Movement
- **Fly** — four modes: Velocity, CFrame, Anchored, and Multi-part; camera-relative WASD + Space/Ctrl movement; configurable speed; optional stealth/bypass pulse to avoid anti-fly detection
- **Speed** — overwrites `WalkSpeed` on the Humanoid
- **Jump power** — overwrites `JumpPower` on the Humanoid
- **Spinbot** — Y-spin, jitter, backwards, or random rotation modes
- **Bhop macro** — IOIO or mouse-wheel jump macro

### Config System
- Binary config format with magic header `FNGX` and version field
- Saves and loads all feature toggles, colours, keybinds, and numeric values
- Config is XOR-encrypted on disk (via `xorstr`/`oxorany` protection helpers)

### Misc / QoL
- **Team check** — skips players on the same team
- **Friend check** — skips players marked as friends
- **Client check** — excludes your own character from the cache
- **Streamproof** — sets the overlay window style to exclude it from screen capture
- **Console toggle** — show/hide the debug console window at runtime
- **Watermark** — HUD overlay showing player count and current game name
- **Explorer** — in-menu Roblox instance explorer for browsing the DataModel tree
- **Anti-cheat detection** — on startup, warns if Vanguard, EAC, BattlEye, FACEIT, or Ricochet are running
- **Performance mode** — slider that trades visual fidelity for higher render throughput

### Driver / Memory
- Reads and writes process memory via raw `syscall` stubs (`NtReadVirtualMemory` syscall 63, `NtWriteVirtualMemory` syscall 58) implemented in MASM (`luck.asm`), bypassing the standard Win32 API
- Process watchdog — polls for `RobloxPlayerBeta.exe` every second and automatically re-attaches if Roblox restarts or switches servers
- Game-change detection — monitors `PlaceId` and re-resolves all global pointers when the player joins a new place
- Offset auto-fetcher — downloads and parses current offsets at startup to stay up to date after Roblox updates

---

## Known Bugs

- **Silent aim visibility check causes FPS drops** — the `VisibilityCheck` path in Silent aim performs expensive raycasts inside the silent-aim thread loop. Leaving it enabled can cause noticeable frame-rate degradation. Workaround: disable `VisibilityCheck` in the Silent tab.

- **Ghost ESP data after Roblox restart** — before the process watchdog was added there was a known issue where player data from a previous session would remain in the cache and continue rendering after Roblox restarted. The watchdog now clears `Player_Cache` on re-attach, but a race between the cache clear and the visual render thread can still briefly show stale entries.

- **Fly multi-part mode may leave characters anchored** — if the cheat is forcibly closed while Multi-part fly is active, the anchor flag written to all character parts is not cleaned up, leaving the character frozen in place until Roblox is restarted.

- **Random bone selector loops on single-bone targets** — when `HitPart` is set to "random" and a target has only one valid bone in the cache, the random index always resolves to bone 0 instead of cycling, making the selection deterministic.

- **Re-attach timeout is silent** — if the watchdog fails to re-attach within its 30-second window it emits a warning and resets, but the UI continues to render with an empty cache showing no errors to the user.

- **Offset staleness** — all memory offsets are hard-coded for Roblox version `version-5cf2272675e145f5`. Any Roblox update that changes internal struct layouts will silently break reads, producing zero/garbage values with no crash or error message.

---

## Potential Problems

- **Ban risk** — Roblox's Hyperion/Byfron anti-cheat scans for anomalous memory access patterns, overlay windows, and known driver signatures. Direct syscall stubs reduce API-level detection surface but do not eliminate all detection vectors. Expect bans, especially on heavily moderated games.

- **Offset expiry** — Roblox updates frequently. The embedded offset set will go stale after any client patch, causing all features to silently malfunction until offsets are re-dumped and the binary is rebuilt.

- **Syscall numbers are OS-version dependent** — syscall IDs 63 and 58 for `NtReadVirtualMemory` / `NtWriteVirtualMemory` are correct for current Windows 11 builds but may differ on older or future Windows versions, causing crashes or memory corruption.

- **No handle-level protection** — the driver opens a plain `OpenProcess` handle. Any kernel-level anti-cheat or EDR that monitors `PROCESS_VM_READ` / `PROCESS_VM_WRITE` handle creation will see and potentially block or flag the cheat.

- **Mesh chams network requests** — the chams system fetches `.mesh` files from `assetdelivery.roblox.com` at runtime using `WinHTTP`. This produces outbound network traffic from the cheat process that could be flagged by a monitoring tool or corporate firewall.

- **Thread safety on Player_Cache** — `Globals::Player_Cache` is protected by `Cache::Mutex`, but the visuals and aimbot subsystems take snapshots without always acquiring the lock (notably in `AcquireTarget`), which can lead to reading a partially updated player struct under high load.

- **No ASLR/integrity hardening on the cheat binary itself** — the executable has no self-protection. Any anti-cheat with user-mode scanning can enumerate modules, scan memory, or detect the overlay window by its class name (`scare.lol`) or title.

- **Config encryption is weak** — the XOR-based config obfuscation protects against casual file inspection but is not cryptographically secure. A user can trivially recover the key and tamper with config values.

---

## Building

### Prerequisites
- Windows 10/11 (x64)
- Visual Studio 2022 with **Desktop development with C++**
- MASM assembler (included with VS by default)
- DirectX 11 SDK (bundled with the Windows SDK)

### Steps
1. Open `Coercing.sln` in Visual Studio.
2. Select the **Release | x64** configuration.
3. Build the solution (`Ctrl+Shift+B`).

The cheat must be run as **Administrator** so it can open a `PROCESS_VM_READ | PROCESS_VM_WRITE` handle to Roblox.

---

## Dependencies

| Library | Purpose |
| :--- | :--- |
| **Dear ImGui** | Overlay UI and menu rendering |
| **DirectX 11** | Swap-chain and render target for the overlay window |
| **FreeType** | Font rasterisation for UI text |
| **Clipper2** | Polygon clipping used in the overlay drawing layer |
| **Font Awesome (Solid)** | Icon glyphs in the menu |
| **Source Sans 3 SemiBold** | Primary menu font |
| **WinHTTP** | Fetches mesh asset data for chams |
| **oxorany / xorstr** | Compile-time string/value obfuscation |

---

## Disclaimer

> [!CAUTION]
> This software is provided strictly for **educational and research purposes** — to study how external process memory reading, DirectX overlay rendering, and game-engine internals work at a low level.
>
> Using this software in any online game, including Roblox, **violates the game's Terms of Service** and can result in permanent account bans, hardware bans, or other enforcement action. The authors and contributors accept no liability for bans, account losses, or any other consequences arising from use of this software.
>
> This software performs **direct process memory read/write operations** on a running application. Misuse on systems or processes you do not own or have explicit authorisation to test is illegal under the Computer Fraud and Abuse Act (CFAA), the UK Computer Misuse Act, and equivalent laws in most jurisdictions. **Do not use this software against any system you do not personally own.**
>
> The authors are not responsible for any damage, data loss, legal liability, or other harm resulting from use or misuse of this software. Use entirely at your own risk.
