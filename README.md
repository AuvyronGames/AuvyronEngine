<div align="center">

# AuvyronEngine
**Build worlds. Create experiences.**

![Status](https://img.shields.io/badge/Status-In%20Progress-orange)
![Version](https://img.shields.io/badge/Version-0.4.2--alpha-2785b8)
![C++](https://img.shields.io/badge/C%2B%2B-20-00599C)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3-blue)
![Lua](https://img.shields.io/badge/Lua-5.4-2C2D72)
![License](https://img.shields.io/badge/License-MIT-brightgreen)

An experimental open-source 3D game engine, scene editor and standalone game runtime developed by **Auvyron Games**.

</div>

> **ALPHA / IN PROGRESS** — APIs and project formats may change. Not production-ready. Windows native builds are the main target. End-to-end MSVC GUI builds must be verified on Windows.

## Features (v0.4.2 Alpha)

| Area | Status | Details |
|---|---|---|
| C++20 / OpenGL | Alpha | Native scene renderer, OBJ models, cubemap/gradient skybox |
| Scene editor | Alpha | Hierarchy, Inspector, gizmos, asset tree, console, project manager |
| Lua 5.4 | Alpha | Lifecycle callbacks, input, camera, entity APIs, editor syntax checks |
| Game builds | Alpha | Separate ImGui-free Player and `.auvyronasset` data package |
| UI | Prototype | Lua-defined rectangles, ASCII text, click buttons, menus and HUDs |
| Lighting | Prototype | Directional, point and spot components (currently one active rendered light) |
| Physics | Prototype | Gravity, jump velocities, floor/platform AABB handling |
| VFX | Prototype | Animated placeholder sphere effects for fire and explosion |
| Camera | Alpha | Editor camera, camera component and script-controlled FPS camera |
| Documentation | Planned | GitHub Pages site |

## Source organization

This repository contains the AuvyronEngine editor, game runtime, Lua examples and tests.
The public source distribution is self-contained at the project level: it has no bundled
code from previous private game projects. Third-party libraries are obtained through
CMake FetchContent and retain their own licenses.

All original C++ and Lua source files carry a common AuvyronEngine header.


## Building the editor

Requirements: **Windows 10/11 x64**, **Visual Studio 2022 with C++ workload**, **CMake**, **Git**, internet to fetch third-party dependencies on first configure.

```bat
Build_Clean_Windows.bat
Run_Editor.bat
```

`Build_Windows.bat` performs incremental builds. `Build_Clean_Windows.bat` removes *only the local build directory* and rebuilds everything. CMake also builds the separate `AuvyronPlayer` executable and tests.

## Editor layout

- **Project** — projects, save and Build Game.
- **Components** — meshes, directional/point/spot lights, camera, fire and explosion.
- **Asset Manager** — always-visible folder tree and asset list below the viewport.
- **Credits** — authors, license and dependencies.
- **Settings** — switch Dark or Light theme.
- **Hierarchy / Scene / Inspector** — simultaneously visible, with Move/Rotate/Scale gizmos.
- **Lua Script Editor / Console** — panels next to the asset browser.

Window title is `AuvyronEngine | <ProjectName>`.

## Standalone game packaging

In the editor, click **Project → Build Game**. The resulting directory contains:

```text
MyGame_Windows/
  MyGame.exe
  MyGame.auvyronasset
```

The asset package stores the project manifest, active scene, all files in `assets/`, including Lua scripts, textures and OBJ models. A standalone runtime reads the container **directly from memory**; it does not unpack the Lua files into a `Data` folder.

**Security disclaimer:** `.auvyronasset` uses per-file FNV-1a checksums and reversible XOR stream obfuscation seeded with the game name + asset path. This is **not secure encryption or DRM**. Players with technical knowledge can still recover code and textures. Lua source is still present in reversible form. For actual secrecy, implement signed builds plus authenticated encryption with an appropriately managed key, and move valuable game logic to a trusted server where applicable.

For the cleanest export, build to a fresh output folder. Older alpha builds may leave unrelated files behind; the exporter removes only a legacy `Data/` directory and known launcher files.

## Example Lua scripts

StarterWorld includes:

- `fps_controller.lua`: mouse-look, WASD, sprint with Shift, crouch with Ctrl, jump with Space.
- `main_menu.lua`: start and exit buttons.
- `pause_menu.lua`: Escape pause overlay (use a dedicated controller entity).
- `hud.lua`: health/ammo text and crosshair demo.
- `gun.lua`: left-click hitscan demo with a raycast; R to reload.
- `horror_flicker.lua`: flickering light.
- `haunted_door.lua`: E-key door animation.
- `ambient_fire.lua` and `explosion_demo.lua`: effects.

Each entity currently supports **one Lua script**. Add separate controller entities for HUD, menu, player and gameplay scripts. The GUI examples are lightweight prototypes, not production UI frameworks.

```lua
local Script = {}
function Script.on_start(id)
    Engine.log("Hello from AuvyronEngine!")
end
function Script.on_update(id, dt)
    Engine.ui_text("HELLO WORLD", 0.05, 0.05, 1, 1, 1)
end
return Script
```

## Lua API highlights

`Engine.get_position`, `Engine.set_position`, `Engine.translate`, `Engine.set_velocity`, `Engine.get_velocity`, `Engine.is_grounded`, `Engine.key_down`, `Engine.mouse_down`, `Engine.mouse_delta`, `Engine.set_fps_camera`, `Engine.raycast`, `Engine.set_light`, `Engine.spawn_fire`, `Engine.spawn_explosion`, `Engine.ui_text`, `Engine.ui_rect`, `Engine.ui_button`, `Engine.ui_mouse_visible`, `Engine.set_paused`, `Engine.quit_game`.

GUI coordinates are normalized to `0..1` from the **top-left**; redraw controls every `on_update`. The embedded font currently supports basic ASCII only.

## Known limitations / roadmap

- Complete modern rigid-body physics, colliders, advanced character controller and collision events.
- Multi-light rendering, shadows, real particle-based VFX and volumetric effects.
- Full visual GUI designer, bitmap fonts, international text and images.
- PBR materials, glTF, skeletal animation, audio and prefab workflows.
- Asset signing and optionally authenticated encryption (obfuscation is not enough).
- Automated Windows end-to-end editor/Player smoke testing, installers and deployment.
- GitHub Pages documentation (VitePress is planned).

## License

Core source code is distributed under the [MIT License](LICENSE). Third-party dependencies and any legacy code/assets may have separate terms. Review those licenses before redistributing game content.

*Made by Auvyron Games · Initial author: D4NTE*
