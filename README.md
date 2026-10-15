# Inkwell Core — Live Matchmaking Instrumentation Framework

**Target Process**: `project8.exe` (Valve Deadlock)  
**Execution Mode**: **Pure Live / Secure Matchmaking** (Zero command-line flags required, no `-insecure`)  
**Architecture**: x64 Native C++20 (MSVC `14.51` / `v145`)  
**Deployment Suite**:
- `inkwelldeadlock.dll` (Internal Dynamic Instrumentation Module)
- Native compatibility with `Inkwell Loader Core` (`chs\Inkwell loader`) or external manual mappers.

---

## 1. How It Operates Without `-insecure`

Unlike research tools that require disabling anticheat via `-insecure`, **Inkwell Core** operates exactly like `CheatLock` and `Interium` in standard live sessions:

1. **No External Console Window**:
   - `AllocConsole()` and `freopen_s` have been replaced with internal session logging to `%TEMP%\inkwell_deadlock.log` and `OutputDebugStringA`.
   - Prevents spawning visible `conhost.exe` child processes that trip process-tree monitors.
2. **In-Game Module Synchronization Spinlock**:
   - Before querying interfaces or installing hooks, the module spin-waits for Deadlock's core modules to finish initialization:
     `client.dll`, `engine2.dll`, `schemasystem.dll`.
   - Eliminates early-injection crashes and null interface returns.
3. **Steam Overlay Hooking & Native DXGI Dual Pipeline**:
   - Detours `PresentOverlay` and `ResizeBuffersOverlay` via 14-byte atomic x64 trampolines, with automatic fallback to native D3D11 DXGI SwapChain detours.
   - Drawing occurs natively inside Steam's own presentation pipeline or directly via DXGI, fully compatible with normal fullscreen/borderless matchmaking.
4. **PE Header Erasure in Memory**:
   - Immediately following hook installation, `SecureZeroMemory` wipes the DOS and NT headers at the DLL's base address in memory (`0x1000` bytes).
   - Defeats naive in-memory PE header signature scanners.
5. **Invariant Angle Clamping**:
   - In `CreateMove`, pitch is clamped to `[-89.0°, 89.0°]` and yaw to `[-180.0°, 180.0°]` with polynomial smoothing to bypass server-side heuristic "frog" anomaly detection.

---

## 2. Injection & Runtime Usage

1. **Launch Deadlock normally via Steam** (no special launch options required).
2. Once the game reaches the main menu, inject `inkwelldeadlock.dll` into `project8.exe` via:
   - Your preferred injector / manual mapping loader, or
   - Standard thread hijacking vector targeting `project8.exe`.
3. The **Inkwell** Purple & Black HUD watermark (`#9447F2`), FOV circle, and combat ESP will activate natively in-game.

---

## 3. Interactive Keybinds & Runtime Controls

- **`[INSERT]` — Master Toggle**: Instantly toggles all Visuals (ESP, Boxes, Skeletons) and Aim Assist systems ON/OFF.
- **`[F9]` — Live Schema Auto-Dump**: Triggers the dynamic schema crawler to dump all live classes and offsets directly to `schema_dump_live.txt`.
- **`[END]` — Emergency Panic Unload**: Safely disables and removes all 14-byte MinHook detours in `gameoverlayrenderer64.dll`, restores thread execution, and calls `FreeLibraryAndExitThread()` to cleanly exit memory without crashing the game client.

---

## 4. Advanced Subsystems & Feature Suite (VenDeadlock & Marvel Rivals Parity)

1. **Dynamic Runtime Schema Auto-Updater**:
   - Intercepts `SchemaSystem_001` and crawls all in-memory type scopes (`client.dll`, `engine2.dll`, `server.dll`) dynamically.
   - Immune to Tuesday Valve patches; exports live dumps via `[F9]`.
2. **In-Flight Projectile & Threat Warning**:
   - Intercepts Bebop hooks and immobilizing projectiles in real time.
   - Computes distance and displays top-center warning banner (`! BEBOP HOOK DETECTED [XXm] !`).
3. **Triggerbot (Auto-Fire Assist)**:
   - Evaluates crosshair intersection against target bounding radii and pulses `IN_ATTACK` within `triggerbot_fov` deadzone.
4. **Recoil Pitch/Yaw Compensation (RCS)**:
   - Counteracts weapon climb and spread impulse kicks directly inside `CreateMove`.
5. **CQB Auto-Melee Strike**:
   - Automatically executes melee strikes (`IN_WEAPON1`) when an enemy pawn closes within striking range (< 3.5m).
6. **Combat Visuals & Dynamic Health Bars**:
   - Color-graded HP indicators (Emerald -> Amber -> Crimson) alongside hero bounding boxes with numeric health values (`Hero [XXX HP | XXm]`).
7. **Hero Name & Class Identification Mapping**:
   - Resolves internal schema class and designer tokens into clean hero titles (Bebop, Seven, Vindicta, Haze, Infernus, Dynamo, etc.).
8. **Off-Screen Perimeter Radar**:
   - Calculates 2D directional vectors for off-screen enemies and renders indicator diamonds around the screen perimeter with distance readouts.
9. **Snaplines (Tracer Lines)**:
   - Draws directional lines from bottom screen center to enemy player bases.
10. **Minimap Fog-of-War Disclosure**:
    - Discloses all hidden enemy pawns and neutral creep positions on the in-game radar.

---

## 5. Rebuilding

To rebuild the DLL after modifying features or settings:
```powershell
.\build.bat
```
Output artifact generated in project root:
- `Inkwell_x64.dll`
