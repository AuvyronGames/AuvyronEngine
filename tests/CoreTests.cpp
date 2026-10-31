/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    tests/CoreTests.cpp
 * @license MIT
 */

#ifdef NDEBUG
#undef NDEBUG
#endif
#include "core/Scene.hpp"
#include "core/LuaRuntime.hpp"
#include "core/Project.hpp"
#include "core/LuaDiagnostics.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <iostream>
int main(){
 namespace fs=std::filesystem;
 auto root=fs::temp_directory_path()/"auvyron_core_tests";
 fs::remove_all(root);
 auv::Project p;assert(p.create(root,"Test"));auv::Project loaded;assert(loaded.load(root));assert(loaded.name=="Test");
 auv::Scene s;auto& a=s.create("Cube");auto id=a.id;
 a.position={1,2,3};a.script="assets/scripts/example.lua";
 assert(s.save(p.scenePath()));auv::Scene restored;assert(restored.load(p.scenePath()));
 assert(restored.find(id));assert(restored.find(id)->position.y==2);
 auv::LuaRuntime lua;assert(lua.start(restored,root));assert(!lua.logs().empty());
 for(int n=0;n<20;++n)lua.update(0.01f);
 assert(std::fabs(restored.find(id)->rotation.y-7.0f)<0.01f);
 lua.stop();
 {
  std::ofstream cameraFile(root/"assets/scripts/camera.lua");
  cameraFile<<"local S={}\nfunction S.on_start(id)\n"
             "Engine.set_camera_target(5,6,7)\n"
             "Engine.set_camera_angles(32,10)\n"
             "Engine.set_camera_distance(9)\n"
             "end\nreturn S\n";
  cameraFile.close();
  auv::Scene cameraScene;
  cameraScene.create("CameraController").script="assets/scripts/camera.lua";
  auv::LuaRuntime camLua;
  assert(camLua.start(cameraScene,root));
  assert(cameraScene.gameCamera.target.x==5.f);
  assert(cameraScene.gameCamera.target.y==6.f);
  assert(cameraScene.gameCamera.distance==9.f);
  assert(cameraScene.gameCamera.yaw==32.f);
  camLua.stop();
 }
 {
  std::ofstream fpsFile(root/"assets/scripts/fps.lua");
  fpsFile << "local S={}\nfunction S.on_start(id)\n"
             "assert(Engine.api_version == '0.4.2-alpha')\n"
             "assert(type(Engine.capture_mouse) == 'function')\n"
             "assert(type(Engine.mouse_delta) == 'function')\n"
             "assert(type(Engine.mouse_captured) == 'function')\n"
             "Engine.capture_mouse(true)\n"
             "Engine.set_fps_camera(1,2,3,45,15)\n"
             "end\nfunction S.on_update(id,dt)\n"
             "local dx,dy=Engine.mouse_delta()\n"
             "Engine.translate(id,dx*dt,0,dy*dt)\n"
             "end\nreturn S\n";
  fpsFile.close();
  auv::Scene fpsScene;
  auto& fpsEntity=fpsScene.create("FPS");
  const auto fpsId=fpsEntity.id;
  fpsEntity.script="assets/scripts/fps.lua";
  auv::LuaRuntime fpsLua;
  bool requestedCapture=false;
  fpsLua.setMouseCaptureCallback([&](bool enabled){requestedCapture=enabled;});
  fpsLua.setMouseCapturedCallback([&](){return requestedCapture;});
  assert(fpsLua.start(fpsScene,root));
  assert(requestedCapture);
  assert(fpsScene.gameCamera.firstPerson);
  assert(fpsScene.gameCamera.eyePosition.x==1.f);
  assert(fpsScene.gameCamera.eyePosition.y==2.f);
  fpsLua.setMouseDelta(20,-10);
  fpsLua.update(0.05f);
  assert(std::fabs(fpsScene.find(fpsId)->position.x-1.f)<0.001f);
  assert(std::fabs(fpsScene.find(fpsId)->position.z+0.5f)<0.001f);
  assert(auv::diagnoseLua("Engine.mouse_delta()\nEngine.capture_mouse(true)\nEngine.set_fps_camera(0,0,0,0,0)\n").empty());
  fpsLua.stop();
 }
 {
  auto menuPath=root/"assets/scripts/menu_test.lua";
  std::ofstream menu(menuPath);
  menu<<"local S={}\nfunction S.on_update(id,dt)\n"
       " Engine.ui_rect(0.2,0.2,0.5,0.5)\n"
       " Engine.ui_text('Hello',0.3,0.3)\n"
       " if Engine.ui_button('PLAY',0.3,0.4,0.2,0.08) then Engine.log('UI pressed') end\n"
       "end\nreturn S\n";
  menu.close();
  auv::Scene uiScene;uiScene.create("UI").script="assets/scripts/menu_test.lua";
  auv::LuaRuntime runtime;assert(runtime.start(uiScene,root));
  runtime.ui().beginFrame(.35f,.44f,true);runtime.update(.016f);
  assert(runtime.ui().elements().size()==3);
  bool clicked=false;for(const auto& log:runtime.logs())if(log=="UI pressed")clicked=true;
  assert(clicked);runtime.stop();
 }

 auto& falling=restored.create("Falling");falling.position.y=5;falling.physics=true;
 for(int n=0;n<200;++n)restored.stepPhysics(0.02f);
 assert(falling.position.y>=0.49f && falling.position.y<=0.51f);
 {
  auto valid=auv::diagnoseLua("local S={}\nfunction S.on_start(id) Engine.log('Hello') end\nreturn S\n");
  assert(valid.empty());
  auto bad=auv::diagnoseLua("local x = (\nEngine.unknown_function()\n");
  assert(!bad.empty());
  bool syntaxError=false,unknownAPI=false;
  for(const auto& d:bad){
   if(d.severity==auv::DiagnosticSeverity::Error)syntaxError=true;
   if(d.message.find("unknown_function")!=std::string::npos)unknownAPI=true;
  }
  assert(syntaxError&&unknownAPI);
  auto comment=auv::diagnoseLua("-- Engine.no_such_function()\nlocal a = 'Engine.fake()'\n");
  assert(comment.empty());
  auto event=auv::diagnoseLua("local Script={}\nfunction Script.on_collide(id) end\nreturn Script\n");
  assert(event.size()==1 && event[0].severity==auv::DiagnosticSeverity::Warning);
  assert(event[0].line==2);
 }
 fs::remove_all(root);
 std::cout<<"Auvyron core, Lua and physics tests passed\n";
}
