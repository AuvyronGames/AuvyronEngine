# AuvyronEngine FPS Controller v0.4.2 Alpha

Attach `assets/scripts/fps_controller.lua` to the entity used as the player. It requires the updated Lua APIs and basic Physics system.

| Input | Action |
|---|---|
| W A S D | Move relative to camera facing |
| Mouse | Rotate yaw and pitch |
| Left Shift | Sprint |
| Left Control | Crouch (smooth camera height) |
| Space | Jump while grounded |
| Escape | Release mouse in Play mode (can also be used by pause script) |

The controller enables gravity and hides the mesh during FPS view. Character collisions are experimental: floor and simple static platform AABB only. Player movement currently translates horizontally; wall blocking is not fully solved.

`Engine.mouse_delta()`, `Engine.capture_mouse(true)`, `Engine.get_velocity()`, `Engine.set_velocity()`, `Engine.is_grounded()` and `Engine.set_fps_camera()` are exposed by the engine's native Lua runtime. Rebuild all targets after replacing sources.
