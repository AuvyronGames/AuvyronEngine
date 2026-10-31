/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/render/Renderer.hpp
 * @license MIT
 */

#pragma once
#include "core/Scene.hpp"
#include "core/AssetStore.hpp"
#include "core/GameUI.hpp"
#include <memory>
#include <algorithm>
#include "render/GL.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
namespace auv {
struct Camera {
 Vec3 target{0,0.8f,0};
 bool firstPerson=false;
 Vec3 position{};
 float yaw=40,pitch=24,distance=14;
 Vec3 eye()const;
 Vec3 forward()const;
 Mat4 view()const;
 Mat4 projection(float aspect)const{return Mat4::perspective(55,aspect,0.05f,400.f);}
 void orbit(float dx,float dy);
 void zoom(float y){distance=std::clamp(distance-y*1.0f,1.5f,120.f);}
 void pan(float dx,float dy);
};
class Renderer {
public:
 bool init();
 void setResources(std::shared_ptr<AssetStore> store){resources_=std::move(store);}
 void drawGameUI(const GameUI& ui,bool runtime);
 void shutdown();
 bool resize(int w,int h);
 void draw(const Scene& scene,const Camera& camera,EntityId selected,const std::filesystem::path& projectRoot,bool runtime=false);
 GLuint texture()const{return color_;}
 int width()const{return width_;}
 int height()const{return height_;}
 // Project world position to viewport pixels for selection overlay.
 bool project(Vec3 world,const Camera& camera,float& x,float& y)const;
 const std::string& error()const{return error_;}
private:
 struct Vertex {float x,y,z,nx,ny,nz;};
 struct Mesh {GLuint vao=0,vbo=0;GLsizei count=0;};
 Mesh createMesh(const std::vector<Vertex>& vertices);
 void deleteMesh(Mesh& mesh);
 const Mesh* meshFor(const Entity& entity,const std::filesystem::path& projectRoot);
 Mesh loadOBJ(const std::filesystem::path& file);
 GLuint compile(GLenum kind,const char* text);
 void updateSkybox(const Scene& scene,const std::filesystem::path& root);
 void drawSkybox(const Scene& scene,const Camera& camera);
 GLuint fbo_=0,color_=0,depth_=0,shader_=0;
 GLuint uiShader_=0,uiVao_=0,uiVbo_=0;
 std::shared_ptr<AssetStore> resources_;
 GLuint skyShader_=0,skyVao_=0,skyTexture_=0;
 std::string skyTextureSignature_;
 bool skyLoaded_=false;
 Mesh cube_,plane_,sphere_,grid_;
 std::unordered_map<std::string,Mesh> obj_;
 int width_=0,height_=0;
 std::string error_;
};
}
