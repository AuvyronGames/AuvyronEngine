/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/Player.cpp
 * @license MIT
 */

// AuvyronPlayer - standalone OpenGL/Lua application. No Dear ImGui or editor code.
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "core/Project.hpp"
#include "core/AssetPackage.hpp"
#include "core/AssetStore.hpp"
#include <memory>
#include "core/Scene.hpp"
#include "core/LuaRuntime.hpp"
#include "render/Renderer.hpp"
#include "render/GL.hpp"
#include "app/MouseCapture.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace {
std::filesystem::path gameDirectory(){
#ifdef _WIN32
 wchar_t path[32768]{};
 if(GetModuleFileNameW(nullptr,path,32768))return std::filesystem::path(path).parent_path();
#elif defined(__linux__)
 std::error_code ec;
 auto binary=std::filesystem::read_symlink("/proc/self/exe",ec);
 if(!ec)return binary.parent_path();
#endif
 return std::filesystem::current_path();
}
void writeFailure(const std::filesystem::path& home,const std::string& message){
 std::cerr << "[AuvyronPlayer] " << message << '\n';
 std::ofstream file(home/"AuvyronPlayer_error.log",std::ios::app);
 if(file)file << message << '\n';
#ifdef _WIN32
 MessageBoxA(nullptr,message.c_str(),"AuvyronPlayer",MB_OK|MB_ICONERROR);
#endif
}
int glfwKey(const std::string& name){
 std::string key;
 for(unsigned char c:name)key+=char(std::toupper(c));
 if(key.size()==1){
  if(key[0]>='A'&&key[0]<='Z')return GLFW_KEY_A+(key[0]-'A');
  if(key[0]>='0'&&key[0]<='9')return GLFW_KEY_0+(key[0]-'0');
 }
 if(key=="SPACE")return GLFW_KEY_SPACE;
 if(key=="LEFT_SHIFT"||key=="SHIFT")return GLFW_KEY_LEFT_SHIFT;
 if(key=="LEFT_CONTROL"||key=="CTRL")return GLFW_KEY_LEFT_CONTROL;
 if(key=="ESCAPE")return GLFW_KEY_ESCAPE;
 if(key=="UP")return GLFW_KEY_UP;
 if(key=="DOWN")return GLFW_KEY_DOWN;
 if(key=="LEFT")return GLFW_KEY_LEFT;
 if(key=="RIGHT")return GLFW_KEY_RIGHT;
 return GLFW_KEY_UNKNOWN;
}
struct PlayerInput {
 bool rightDown=false;
 bool middleDown=false;
 bool cursorValid=false;
 double x=0,y=0,scroll=0;
};
int runPlayer(){
 const auto home=gameDirectory();
 auv::Project project;
 auv::Scene scene;
 std::shared_ptr<auv::AssetPackage> package;
 auto store=std::make_shared<auv::AssetStore>(home);
 std::error_code packageError;
 std::filesystem::path packageFile;
 for(const auto& item:std::filesystem::directory_iterator(home,packageError)){
  if(item.is_regular_file()&&item.path().extension()==".auvyronasset"){
   if(!packageFile.empty()){writeFailure(home,"Multiple .auvyronasset packages found");return 1;}
   packageFile=item.path();
  }
 }
 if(packageFile.empty()){writeFailure(home,"Game package (.auvyronasset) is missing");return 1;}
 package=std::make_shared<auv::AssetPackage>();std::string error;
 if(!package->open(packageFile,error)){
  writeFailure(home,"Cannot open asset package: "+error);return 1;
 }
 store->usePackage(package);
 std::string manifest,sceneText;
 if(!store->readText("project.auvproj",manifest)||!project.loadText(manifest,home)){
  writeFailure(home,"Missing or invalid project manifest inside asset package");return 1;
 }
 if(!store->readText(project.scene,sceneText)||!scene.loadText(sceneText)){
  writeFailure(home,"Missing or invalid scene in asset package: "+scene.error());return 1;
 }
 if(!glfwInit()){
  writeFailure(home,"GLFW initialization failed. Check graphics driver.");return 1;
 }
 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
 glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
 glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
 GLFWwindow* window=glfwCreateWindow(1280,720,project.name.c_str(),nullptr,nullptr);
 if(!window){glfwTerminate();writeFailure(home,"Cannot create OpenGL 3.3 window.");return 1;}
 glfwMakeContextCurrent(window);glfwSwapInterval(1);
 if(!auv::glx.load()){
  glfwDestroyWindow(window);glfwTerminate();writeFailure(home,"OpenGL 3.3 functions are unavailable.");return 1;
 }
 auv::Renderer renderer;
 renderer.setResources(store);
 if(!renderer.init()){
  auto error=renderer.error();renderer.shutdown();glfwDestroyWindow(window);glfwTerminate();
  writeFailure(home,"Renderer initialization failed: "+error);return 1;
 }
 auv::LuaRuntime scripts;
 auv::MouseCapture mouse;
 scripts.setMouseCaptureCallback([&](bool enabled){mouse.capture(window,enabled);});
 scripts.setMouseCapturedCallback([&](){return mouse.captured();});
 scripts.setMouseButtonCallback([window](const std::string& button){
  if(button=="LEFT")return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
  if(button=="RIGHT")return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
  return false;
 });
 scripts.setMouseVisibleCallback([window,&mouse](bool visible){
  if(visible){mouse.capture(window,false);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);}
  else mouse.capture(window,true);
 });
 scripts.setQuitCallback([window](){glfwSetWindowShouldClose(window,1);});
 scripts.setKeyCallback([window](const std::string& key){
  const int code=glfwKey(key);
  return code!=GLFW_KEY_UNKNOWN&&glfwGetKey(window,code)==GLFW_PRESS;
 });
 const bool scriptsOkay=scripts.start(scene,store);
 (void)scriptsOkay;
 auv::Camera camera;
 PlayerInput input;
 glfwSetWindowUserPointer(window,&input);
 glfwSetScrollCallback(window,[](GLFWwindow* w,double,double y){
  auto* in=static_cast<PlayerInput*>(glfwGetWindowUserPointer(w));in->scroll+=y;
 });
 auto updateCamera=[&](){
  camera.target=scene.gameCamera.target;
  camera.yaw=scene.gameCamera.yaw;
  camera.pitch=scene.gameCamera.pitch;
  camera.distance=scene.gameCamera.distance;
  camera.firstPerson=scene.gameCamera.firstPerson;
  camera.position=scene.gameCamera.eyePosition;
  if(!scene.gameCamera.firstPerson && scene.gameCamera.activeEntity){
   if(const auto* e=scene.find(scene.gameCamera.activeEntity);e&&e->kind==auv::ComponentKind::Camera){
    camera.firstPerson=true;camera.position=e->position;
    camera.yaw=e->rotation.y;camera.pitch=e->rotation.x;
   }
  }
 };
 updateCamera();
 double previous=glfwGetTime();
 bool previousEsc=false,previousClick=false;
 while(!glfwWindowShouldClose(window)){
  glfwPollEvents();
  const bool escape=glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS;
  if(escape&&!previousEsc){
   if(mouse.captured())mouse.capture(window,false);
   else if(scripts.ui().elements().empty()&&!scripts.paused())glfwSetWindowShouldClose(window,1);
  }
  previousEsc=escape;
  const bool clicked=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
  if(clicked&&!previousClick&&!mouse.captured()&&!escape &&
     scripts.ui().elements().empty()&&!scripts.paused())mouse.capture(window,true);
  const bool clickPressed=clicked&&!previousClick;
  previousClick=clicked;
  const double now=glfwGetTime();
  const float dt=std::clamp(float(now-previous),0.f,0.1f);
  previous=now;
  // Script changes to the camera are applied before optional runtime orbit input.
  mouse.update(window);
  scripts.setMouseDelta(mouse.dx(),mouse.dy());
  int windowWidth=0,windowHeight=0;
  glfwGetWindowSize(window,&windowWidth,&windowHeight);
  double mx=0,my=0;glfwGetCursorPos(window,&mx,&my);
  scripts.ui().beginFrame(windowWidth>0?float(mx/windowWidth):0.f,
                         windowHeight>0?float(my/windowHeight):0.f,clickPressed);
  scripts.update(dt);
  if(!scripts.paused())scene.stepPhysics(dt);
  updateCamera();
  double x,y;glfwGetCursorPos(window,&x,&y);
  const bool right=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
  const bool middle=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_MIDDLE)==GLFW_PRESS;
  if(input.cursorValid && !mouse.captured() && !camera.firstPerson && scene.gameCamera.allowOrbit){
   const float dx=float(x-input.x),dy=float(y-input.y);
   if(right&&input.rightDown)camera.orbit(dx,dy);
   if(middle&&input.middleDown)camera.pan(dx,dy);
   if(input.scroll!=0)camera.zoom(float(input.scroll));
   scene.gameCamera.target=camera.target;
   scene.gameCamera.yaw=camera.yaw;
   scene.gameCamera.pitch=camera.pitch;
   scene.gameCamera.distance=camera.distance;
  }
  input.cursorValid=true;input.x=x;input.y=y;input.rightDown=right;input.middleDown=middle;
  input.scroll=0;
  int width=0,height=0;glfwGetFramebufferSize(window,&width,&height);
  if(width>0&&height>0){
   renderer.resize(width,height);
   renderer.draw(scene,camera,0,project.root,true);
   renderer.drawGameUI(scripts.ui(),true);
   glfwSwapBuffers(window);
  }
 }
 scripts.stop();mouse.capture(window,false);renderer.shutdown();glfwDestroyWindow(window);glfwTerminate();
 return 0;
}
}
#ifdef _WIN32
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){return runPlayer();}
#else
int main(){return runPlayer();}
#endif
