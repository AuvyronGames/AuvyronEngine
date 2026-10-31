# AuvyronEngine v0.4.2 Alpha — Build Game & Game UI

## Build Game

Build Game generates `GameName.exe` and `GameName.auvyronasset`.
The Player is a standalone OpenGL/Lua app and does not include Dear ImGui or the editor.
The package contains `project.auvproj`, the active scene, and all files under `assets/`.
When the game starts, `AuvyronPlayer` opens the package in memory, verifies per-file checksums, loads the scene and runs Lua text chunks without writing script source to disk.

The data format uses magic `AUVPAK1`, game-name-based reversible obfuscation and a per-file FNV-1a 64-bit checksum. This is an **anti-casual-inspection measure** only; it does not prevent reverse engineering. Package tampering is detectable for accidental corruption, not cryptographically authenticated.

## Lua GUI API

Functions called every frame in `on_update`:

```lua
Engine.ui_rect(x, y, width, height, r, g, b, alpha)
Engine.ui_text("TITLE", x, y, r, g, b)
local clicked = Engine.ui_button("START", x, y, width, height)
```

- Coordinates are normalized 0..1, top-left origin.
- Color components are 0..1.
- Widget layouts are currently manual. You may group scripts in controller entities.
- Rendered by the Player's simple OpenGL overlay (not ImGui).
- Font is basic ASCII and fixed-size; no support yet for font assets, images, multiline layout or Unicode.
- `Engine.ui_mouse_visible(true)` releases mouse capture; `false` recaptures it.
- `Engine.set_paused(true)` stops automatic physics and the runtime clock, but Lua callbacks still run so the pause menu can be interactive.
- Use `Engine.quit_game()` to close a game from its menu.

See `examples/StarterWorld/assets/scripts/main_menu.lua` and `pause_menu.lua`.

## FPS controls

`fps_controller.lua`: W/A/S/D movement relative to camera yaw, mouse look, Left Shift sprint, Left Ctrl crouch (camera height), Space jump. The script turns on basic gravity and hides the body mesh while in first-person mode.

## Lighting and VFX

Add via Components: Directional Light, Point Light, Spotlight, Camera, Fire, Explosion. Properties are stored in scene version 3. Old scene version 1/2 is loadable. **Renderer currently uses the first active light only.** Fire and explosion are animated placeholder meshes, not complete particle systems. Physics is basic floor/platform AABB handling only.

## Building on Windows

Run `Build_Clean_Windows.bat` with Visual Studio 2022 C++ toolchain installed. Make sure the separate `AuvyronPlayer.exe` gets built before using Build Game. Use a new output directory for releases to avoid unrelated stale files from previous builds.

## Security and release checklist

- Remove third-party assets you do not have the right to redistribute.
- Remember that all Lua scripts may be recovered by reverse engineers.
- Do not put passwords, paid API secrets or anti-cheat secrets in client-side Lua.
- Sign EXE releases, test the output on a clean Windows installation, and check the licenses of OpenGL-related dependencies.
