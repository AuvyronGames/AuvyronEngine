/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/Application.cpp
 * @license MIT
 */

#include "app/Application.hpp"
#include "render/GL.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <algorithm>
namespace auv {
namespace {
void errorCallback(int code,const char* text){std::cerr<<"GLFW ["<<code<<"] "<<text<<'\n';}
void style(){
 ImGui::StyleColorsDark();auto& s=ImGui::GetStyle();
 s.WindowRounding=4;s.FrameRounding=4;s.GrabRounding=3;s.TabRounding=4;
 s.WindowPadding={11,10};s.FramePadding={7,5};s.ItemSpacing={7,7};s.ItemInnerSpacing={6,5};
 s.WindowBorderSize=1;s.ChildBorderSize=1;s.ScrollbarRounding=3;s.ScrollbarSize=11;
 auto& c=s.Colors;
 c[ImGuiCol_Text]={.85f,.90f,.95f,1};c[ImGuiCol_TextDisabled]={.47f,.54f,.60f,1};
 c[ImGuiCol_WindowBg]={.055f,.072f,.095f,1};
 c[ImGuiCol_ChildBg]={.067f,.087f,.113f,1};c[ImGuiCol_PopupBg]={.077f,.098f,.128f,.98f};
 c[ImGuiCol_MenuBarBg]={.050f,.066f,.085f,1};
 c[ImGuiCol_TitleBg]={.056f,.071f,.094f,1};
 c[ImGuiCol_TitleBgActive]={.073f,.105f,.139f,1};
 c[ImGuiCol_Border]={.15f,.20f,.255f,1};
 c[ImGuiCol_FrameBg]={.089f,.122f,.157f,1};
 c[ImGuiCol_FrameBgHovered]={.12f,.19f,.24f,1};
 c[ImGuiCol_FrameBgActive]={.13f,.26f,.31f,1};
 c[ImGuiCol_Button]={.115f,.18f,.22f,1};
 c[ImGuiCol_ButtonHovered]={.16f,.29f,.34f,1};
 c[ImGuiCol_ButtonActive]={.18f,.37f,.42f,1};
 c[ImGuiCol_Header]={.10f,.22f,.29f,1};
 c[ImGuiCol_HeaderHovered]={.15f,.30f,.37f,1};
 c[ImGuiCol_HeaderActive]={.19f,.34f,.41f,1};
 c[ImGuiCol_CheckMark]={.36f,.80f,.91f,1};
 c[ImGuiCol_SliderGrab]={.33f,.75f,.88f,1};
 c[ImGuiCol_Separator]={.14f,.20f,.255f,1};
 c[ImGuiCol_ResizeGrip]={.23f,.49f,.58f,.35f};
 c[ImGuiCol_ScrollbarBg]={.055f,.075f,.095f,1};
 c[ImGuiCol_ScrollbarGrab]={.22f,.30f,.36f,1};
}
} // namespace (anonymous helpers)

bool Application::init(const std::filesystem::path& path,bool playerOnly){
 playerOnly_=playerOnly;
 glfwSetErrorCallback(errorCallback);
 if(!glfwInit()){std::cerr<<"GLFW initialization failed\n";return false;}
 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
 glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
 #ifdef __APPLE__
 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
 #endif
 window_=glfwCreateWindow(1520,900,"AuvyronEngine | Loading...",nullptr,nullptr);
 if(!window_){std::cerr<<"Cannot create OpenGL 3.3 window\n";return false;}
 glfwMakeContextCurrent(window_);glfwSwapInterval(1);
 if(!glx.load()){std::cerr<<"Missing required OpenGL functions\n";return false;}
 IMGUI_CHECKVERSION();ImGui::CreateContext();
 auto& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
 style();
 if(!ImGui_ImplGlfw_InitForOpenGL(window_,true)||!ImGui_ImplOpenGL3_Init("#version 330")){
  std::cerr<<"ImGui initialization failed\n";return false;
 }
 imgui_=true;
 if(!editor_.init(path)){std::cerr<<"Editor initialization failed\n";return false;}
 return true;
}
int Application::run(){
 std::string lastTitle;
 double prev=glfwGetTime();
 while(!glfwWindowShouldClose(window_)){
  glfwPollEvents();
  double now=glfwGetTime();float dt=std::clamp(float(now-prev),0.f,0.1f);prev=now;
  ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();
  editor_.draw(window_,dt,playerOnly_);
  const std::string desired="AuvyronEngine | "+editor_.title();
  if(desired!=lastTitle){glfwSetWindowTitle(window_,desired.c_str());lastTitle=desired;}
  ImGui::Render();int w,h;glfwGetFramebufferSize(window_,&w,&h);
  glx.BindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,w,h);
  glClearColor(.03f,.04f,.05f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());glfwSwapBuffers(window_);
 }
 return 0;
}
void Application::shutdown(){
 if(window_)glfwMakeContextCurrent(window_);
 editor_.shutdown();
 if(imgui_){ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();imgui_=false;}
 if(window_){glfwDestroyWindow(window_);window_=nullptr;}
 glfwTerminate();
}
}
