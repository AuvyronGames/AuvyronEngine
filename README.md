<div align="center">

# AuvyronEngine

An open-source, experimental **3D game engine** built with **C++20**, **OpenGL** and **Lua**.

![Development Status](https://img.shields.io/badge/status-IN%20PROGRESS-orange?style=for-the-badge)
![Version](https://img.shields.io/badge/version-0.4.2--alpha-5865F2?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)

![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3-5586A4?style=flat-square&logo=opengl&logoColor=white)
![Lua](https://img.shields.io/badge/Lua-5.4-2C2D72?style=flat-square&logo=lua&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows-0078D4?style=flat-square&logo=windows&logoColor=white)
![CMake](https://img.shields.io/badge/build-CMake-064F8C?style=flat-square&logo=cmake&logoColor=white)

**[Features](#-features) · [Getting started](#-getting-started) · [Lua scripting](#-lua-scripting) · [Roadmap](#-roadmap) · [Documentation](#-documentation)**

</div>

> [!WARNING]
> **Early alpha — actively in development.** AuvyronEngine is a learning and development project, not yet a production-ready engine. APIs, project formats and editor features may change between versions. Windows builds and exported games should be tested before use.

## ✨ About

**AuvyronEngine** is a lightweight, C++-based 3D engine focused on an accessible editor workflow and Lua-powered gameplay. Its goal is to let developers assemble scenes, configure world settings, attach scripts and export a playable application — without requiring C++ for every gameplay feature.

The project includes two separate applications:

| Application | Purpose |
| --- | --- |
| **AuvyronEngine** | Scene editor, hierarchy, inspector, asset browser and Lua development tools |
| **AuvyronPlayer** | Standalone 3D game runtime without the editor UI |

## 🚀 Features

### Editor & workflow
- Scene Hierarchy, viewport, Inspector and bottom asset browser with a folder tree.
- **Move / Rotate / Scale** gizmos powered by ImGuizmo.
- Scene and project saving via `.auvscene` and `.auvproj` files.
- Integrated Lua code editing, syntax highlighting and basic diagnostics.
- **Build Game** workflow that packages a standalone player with project assets.

### 3D & runtime
- OpenGL 3.3 renderer with basic 3D primitives and OBJ support.
- Basic directional lighting and scene transforms.
- **Skybox** options: gradient sky or six-face cubemap textures.
- Configurable game camera, including an example first-person controller.
- Basic gravity/ground behavior (not a full rigid-body physics engine).

### Lua scripting
- Embedded **Lua 5.4** scripting runtime.
- Script lifecycle callbacks such as `on_start`, `on_update` and `on_stop`.
- Engine bindings for entities, transforms, input, logging and camera control.
- FPS input API including mouse capture and mouse movement.

## 🛠️ Getting started

### Requirements
- Windows 10/11 (64-bit)
- Visual Studio 2022 with **Desktop development with C++**
- CMake and Git available on your `PATH`
- Internet connection for the initial dependency download
- A graphics driver supporting OpenGL 3.3

### Build and run

1. Clone or download this repository.
2. Open the project folder.
3. Run **`Build_Clean_Windows.bat`** for a fresh build, or **`Build_Windows.bat`** for a normal build.
4. After a successful build, run **`Run_Editor.bat`** to open the example project.
5. Open `examples/StarterWorld` and press **Play** to test the scene.

The Visual Studio Release builds are expected at:

```text
build/Release/AuvyronEngine.exe
build/Release/AuvyronPlayer.exe
```

> [!NOTE]
> CMake fetches third-party dependencies during the first configuration. If compilation fails, check [`BUILD_NOTES.md`](BUILD_NOTES.md) and the complete compiler output.

## 🧩 Lua scripting

Attach a Lua script to a scene entity to define its gameplay behavior:

```lua
local Script = {}

function Script.on_start(id)
    Engine.log("Hello from AuvyronEngine!")
end

function Script.on_update(id, dt)
    Engine.rotate(id, 0, 30 * dt, 0)
end

function Script.on_stop(id)
    Engine.log("Script stopped")
end

return Script
```

For first-person controls, see `examples/StarterWorld/assets/scripts/fps_controller.lua` and [`FPS_CONTROLS.md`](FPS_CONTROLS.md).

## 📦 Building a game

**Build Game** exports the scene, scripts and required assets together with the standalone player. The exported executable runs the game **without Scene Hierarchy, Inspector, editor gizmos or other editor panels**.

```text
MyGame_Windows/
├── MyGame.exe
└── Data/
    ├── project.auvproj
    ├── scenes/
    └── assets/
```

The export is an early-stage Windows runtime package, **not** a platform installer or a production release pipeline. More information: [`BUILD_GAME_GUIDE.md`](BUILD_GAME_GUIDE.md).

## 🗺️ Roadmap

| Feature | Status |
| --- | --- |
| C++20 core and basic OpenGL renderer | ✅ Implemented in source |
| Scene editor, hierarchy and inspector | ✅ Implemented in source |
| Lua 5.4 runtime and gameplay scripting | ✅ Implemented in source |
| Transform gizmos and Lua editor | ✅ Implemented in source |
| Standalone game player and Build Game | ✅ Initial implementation |
| Skybox and FPS camera controls | ✅ Initial implementation |
| Full physics and collision events | 🔜 Planned |
| PBR materials, shadows and advanced lighting | 🔜 Planned |
| glTF import and skeletal animation | 🔜 Planned |
| Audio system and in-game UI | 🔜 Planned |
| More export platforms and automated releases | 🔜 Planned |
| Public documentation website | 🔜 Planned |

See [`ROADMAP.md`](ROADMAP.md) for more technical milestones.

## 📚 Documentation

**Full documentation is coming soon.** We plan to publish a dedicated **GitHub Pages** website with:

- Installation and build instructions
- Editor and project workflow guides
- Complete Lua API reference, including callbacks and events
- Rendering, scene and asset documentation
- Tutorials, example projects and troubleshooting

Until then, the repository includes getting-started notes and build guides. **There is no live documentation URL yet.**

## 🤝 Contributing

AuvyronEngine is in active development. Bug reports, reproducible build issues, ideas and pull requests are welcome. For code changes, please describe the problem, the proposed fix and how you tested it.

## 📄 License

The newly developed AuvyronEngine source is distributed under the **MIT License**. See [`LICENSE`](LICENSE).

Third-party dependencies retain their own licenses.

---

<div align="center">

**AuvyronEngine Alpha**  
*Open source. Built for creators. Still evolving.*

</div>
