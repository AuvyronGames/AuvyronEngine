/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/MouseCapture.hpp
 * @license MIT
 */

#pragma once
#include <GLFW/glfw3.h>

namespace auv {
// Shared by the editor's Play viewport and standalone AuvyronPlayer.
// GLFW_CURSOR_DISABLED hides/confines the pointer and supplies virtual positions
// for unlimited FPS rotation. Raw motion is enabled only when supported.
class MouseCapture {
public:
 void capture(GLFWwindow* window,bool enable){
  if(!window || enabled_==enable)return;
  enabled_=enable;
  dx_=dy_=0.f;
  valid_=false;
  glfwSetInputMode(window,GLFW_CURSOR,enable?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);
  if(glfwRawMouseMotionSupported())
   glfwSetInputMode(window,GLFW_RAW_MOUSE_MOTION,enable?GLFW_TRUE:GLFW_FALSE);
 }
 void update(GLFWwindow* window){
  dx_=dy_=0.f;
  if(!window||!enabled_)return;
  if(glfwGetWindowAttrib(window,GLFW_FOCUSED)!=GLFW_TRUE){capture(window,false);return;}
  double x=0,y=0;
  glfwGetCursorPos(window,&x,&y);
  if(valid_){dx_=float(x-x_);dy_=float(y-y_);}
  x_=x;y_=y;valid_=true;
 }
 bool captured()const{return enabled_;}
 float dx()const{return dx_;}
 float dy()const{return dy_;}
private:
 bool enabled_=false,valid_=false;
 double x_=0,y_=0;
 float dx_=0,dy_=0;
};
}
