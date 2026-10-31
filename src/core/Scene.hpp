/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/Scene.hpp
 * @license MIT
 */

#pragma once
#include "core/Math.hpp"
#include <cstdint>
#include <array>
#include <filesystem>
#include <string>
#include <vector>
namespace auv {
using EntityId=std::uint64_t;
enum class Shape { Cube, Plane, Sphere, OBJ };
enum class ComponentKind { Mesh, DirectionalLight, PointLight, SpotLight, Camera, Fire, Explosion };
struct Entity {
 EntityId id=0;
 std::string name="Entity";
 Vec3 position{},rotation{},scale{1,1,1},color{0.6f,0.8f,0.95f};
 Shape shape=Shape::Cube;
 std::string mesh;
 std::string script;
 bool visible=true, physics=false, staticBody=false;
 Vec3 velocity{};
 ComponentKind kind=ComponentKind::Mesh;
 Vec3 lightColor{1.f,0.95f,0.84f};
 float intensity=1.0f,range=12.f,spotAngle=35.f;
 float effectAge=0.f,effectDuration=1.2f;
 bool grounded=false;
};
struct SkyboxSettings {
 bool enabled=true;
 bool useCubemap=false;
 Vec3 zenith{0.075f,0.19f,0.40f};
 Vec3 horizon{0.51f,0.68f,0.84f};
 // OpenGL cubemap order: +X, -X, +Y, -Y, +Z, -Z.
 std::array<std::string,6> faces{};
};
struct GameCameraSettings {
 Vec3 target{0.f,0.8f,0.f};
 float yaw=40.f,pitch=24.f,distance=14.f;
 bool allowOrbit=true;
 // Set by Lua at runtime. Kept out of the saved editor camera configuration.
 EntityId activeEntity=0;
 bool firstPerson=false;
 Vec3 eyePosition{};
};
class Scene {
public:
 SkyboxSettings skybox;
 GameCameraSettings gameCamera;
 Entity& create(std::string name="Entity", Shape shape=Shape::Cube);
 Entity* find(EntityId id);
 const Entity* find(EntityId id)const;
 Entity* findName(const std::string& name);
 bool remove(EntityId id);
 void clear();
 bool save(const std::filesystem::path& file)const;
 bool load(const std::filesystem::path& file);
 bool loadText(const std::string& content);
 void stepPhysics(float dt);
 const std::vector<Entity>& entities()const{return entities_;}
 std::vector<Entity>& entities(){return entities_;}
 const std::string& error()const{return error_;}
private:
 std::vector<Entity> entities_;
 EntityId nextId_=1;
 mutable std::string error_;
};
}
