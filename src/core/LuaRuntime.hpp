/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/LuaRuntime.hpp
 * @license MIT
 */

#pragma once
#include "core/Scene.hpp"
#include "core/GameUI.hpp"
#include "core/AssetStore.hpp"
#include <memory>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
struct lua_State;
namespace auv {
class LuaRuntime {
public:
 LuaRuntime()=default;
 ~LuaRuntime(){stop();}
 LuaRuntime(const LuaRuntime&)=delete;
 LuaRuntime& operator=(const LuaRuntime&)=delete;
 bool start(Scene& scene,const std::filesystem::path& root);
 bool start(Scene& scene,std::shared_ptr<AssetStore> resources);
 void update(float dt);
 void stop();
 bool running()const{return state_!=nullptr;}
 void log(std::string text);
 const std::vector<std::string>& logs()const{return logs_;}
 void clearLog(){logs_.clear();}
 void setKeyCallback(std::function<bool(const std::string&)> f){keyDown_=std::move(f);}
 void setMouseButtonCallback(std::function<bool(const std::string&)> f){mouseButton_=std::move(f);}
 bool mouseDown(const std::string& button)const{return mouseButton_?mouseButton_(button):false;}
 void setMouseCaptureCallback(std::function<void(bool)> f){mouseCapture_=std::move(f);}
 void setMouseCapturedCallback(std::function<bool()> f){mouseCaptured_=std::move(f);}
 void setMouseDelta(float dx,float dy){mouseDx_=dx;mouseDy_=dy;}
 float mouseDx()const{return mouseDx_;}
 float mouseDy()const{return mouseDy_;}
 void captureMouse(bool enabled){if(mouseCapture_)mouseCapture_(enabled);}
 bool mouseCaptured()const{return mouseCaptured_?mouseCaptured_():false;}
 Scene* scene(){return scene_;}
 GameUI& ui(){return ui_;}
 void setQuitCallback(std::function<void()> f){quit_=std::move(f);}
 void quitGame(){if(quit_)quit_();}
 void setMouseVisibleCallback(std::function<void(bool)> f){mouseVisible_=std::move(f);}
 void mouseVisible(bool visible){if(mouseVisible_)mouseVisible_(visible);}
 void setPause(bool paused){paused_=paused;}
 bool paused()const{return paused_;}
 bool keyDown(const std::string& key){return keyDown_?keyDown_(key):false;}
 float elapsed()const{return elapsed_;}
private:
 void registerAPI();
 bool call(int reference,const char* func,EntityId entity,float dt=0,bool withDt=false);
 lua_State* state_=nullptr;
 Scene* scene_=nullptr;
 std::filesystem::path root_;
 std::shared_ptr<AssetStore> resources_;
 GameUI ui_;
 std::function<void(bool)> mouseVisible_;
 std::function<void()> quit_;
 bool paused_=false;
 std::unordered_map<EntityId,int> scripts_;
 std::vector<std::string> logs_;
 std::function<bool(const std::string&)> keyDown_;
 std::function<void(bool)> mouseCapture_;
 std::function<bool(const std::string&)> mouseButton_;
 std::function<bool()> mouseCaptured_;
 float mouseDx_=0,mouseDy_=0;
 float elapsed_=0;
};
}
