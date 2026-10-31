/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/GameUI.hpp
 * @license MIT
 */

#pragma once
#include <string>
#include <vector>
namespace auv {
struct UIElement {
 enum class Type { Rectangle, Text, Button } type=Type::Text;
 std::string text;
 float x=0,y=0,w=0,h=0;
 float r=1,g=1,b=1,a=1;
};
// Immediate-mode runtime UI. Coordinates in normalized screen space [0..1], top-left origin.
class GameUI {
public:
 void beginFrame(float mouseX,float mouseY,bool clicked){
  elements_.clear();x_=mouseX;y_=mouseY;click_=clicked;
 }
 void add(UIElement item){if(elements_.size()<512)elements_.push_back(std::move(item));}
 bool button(const std::string& text,float x,float y,float w,float h){
  UIElement item;item.type=UIElement::Type::Button;item.text=text;
  item.x=x;item.y=y;item.w=w;item.h=h;item.r=.14f;item.g=.27f;item.b=.36f;
  add(item);
  return click_&&x_>=x&&x_<=x+w&&y_>=y&&y_<=y+h;
 }
 const std::vector<UIElement>& elements()const{return elements_;}
private:
 std::vector<UIElement> elements_;
 float x_=0,y_=0;bool click_=false;
};
}
