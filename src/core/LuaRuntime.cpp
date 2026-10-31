/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/LuaRuntime.cpp
 * @license MIT
 */

#include "core/LuaRuntime.hpp"
#include <lua.hpp>
#include <algorithm>
#include <cmath>
#include <sstream>
namespace auv {
namespace {
LuaRuntime* context(lua_State* L){return static_cast<LuaRuntime*>(lua_touserdata(L,lua_upvalueindex(1)));}
Entity* entityArg(lua_State* L,LuaRuntime* rt){
 auto id=static_cast<EntityId>(luaL_checkinteger(L,1));
 return rt->scene()?rt->scene()->find(id):nullptr;
}
int apiLog(lua_State* L){auto* rt=context(L);rt->log(luaL_checkstring(L,1));return 0;}
int apiGetPos(lua_State* L){auto* e=entityArg(L,context(L));if(!e){lua_pushnil(L);return 1;}
 lua_pushnumber(L,e->position.x);lua_pushnumber(L,e->position.y);lua_pushnumber(L,e->position.z);return 3;}
int apiSetPos(lua_State* L){auto* e=entityArg(L,context(L));if(e){e->position={float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};}return 0;}
int apiTranslate(lua_State* L){auto* e=entityArg(L,context(L));if(e){e->position+=Vec3{float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};}return 0;}
int apiRotate(lua_State* L){auto* e=entityArg(L,context(L));if(e){e->rotation+=Vec3{float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};}return 0;}
int apiSetRotation(lua_State* L){auto* e=entityArg(L,context(L));if(e){e->rotation={float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};}return 0;}
int apiSetScale(lua_State* L){auto* e=entityArg(L,context(L));if(e){e->scale={float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};}return 0;}
int apiGetRotation(lua_State* L){auto* e=entityArg(L,context(L));if(!e){lua_pushnil(L);return 1;}
 lua_pushnumber(L,e->rotation.x);lua_pushnumber(L,e->rotation.y);lua_pushnumber(L,e->rotation.z);return 3;}
int apiSetColor(lua_State* L){auto* e=entityArg(L,context(L));if(e){e->color={float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};}return 0;}
int apiSetVisible(lua_State* L){auto* e=entityArg(L,context(L));if(e)e->visible=lua_toboolean(L,2)!=0;return 0;}
int apiFind(lua_State* L){auto* rt=context(L);auto* e=rt->scene()->findName(luaL_checkstring(L,1));if(e)lua_pushinteger(L,static_cast<lua_Integer>(e->id));else lua_pushnil(L);return 1;}
int apiSpawn(lua_State* L){auto* rt=context(L);std::string name=luaL_checkstring(L,1);
 int sh=int(luaL_optinteger(L,2,0));sh=std::clamp(sh,0,3);
 auto& e=rt->scene()->create(name,Shape(sh));lua_pushinteger(L,static_cast<lua_Integer>(e.id));return 1;}
int apiDestroy(lua_State* L){auto* rt=context(L);lua_pushboolean(L,rt->scene()->remove(static_cast<EntityId>(luaL_checkinteger(L,1))));return 1;}
int apiKey(lua_State* L){lua_pushboolean(L,context(L)->keyDown(luaL_checkstring(L,1)));return 1;}
int apiSetCameraTarget(lua_State* L){auto* rt=context(L);
 if(rt->scene())rt->scene()->gameCamera.target={float(luaL_checknumber(L,1)),float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3))};
 return 0;
}
int apiSetCameraAngles(lua_State* L){auto* rt=context(L);
 if(rt->scene()){
  rt->scene()->gameCamera.yaw=float(luaL_checknumber(L,1));
  rt->scene()->gameCamera.pitch=std::clamp(float(luaL_checknumber(L,2)),-85.f,85.f);
 }
 return 0;
}
int apiSetCameraDistance(lua_State* L){auto* rt=context(L);
 if(rt->scene())rt->scene()->gameCamera.distance=std::clamp(float(luaL_checknumber(L,1)),.2f,400.f);
 return 0;
}
int apiGetCameraTarget(lua_State* L){auto* rt=context(L);
 if(!rt->scene()){lua_pushnil(L);return 1;}
 const auto v=rt->scene()->gameCamera.target;
 lua_pushnumber(L,v.x);lua_pushnumber(L,v.y);lua_pushnumber(L,v.z);return 3;
}
int apiMouseDown(lua_State* L){lua_pushboolean(L,context(L)->mouseDown(luaL_checkstring(L,1)));return 1;}
int apiGetCameraAngles(lua_State* L){auto* rt=context(L);
 if(!rt->scene()){lua_pushnil(L);return 1;}
 lua_pushnumber(L,rt->scene()->gameCamera.yaw);
 lua_pushnumber(L,rt->scene()->gameCamera.pitch);return 2;}
int apiMouseDelta(lua_State* L){
 auto* rt=context(L);
 lua_pushnumber(L,rt->mouseDx());lua_pushnumber(L,rt->mouseDy());return 2;
}
int apiCaptureMouse(lua_State* L){context(L)->captureMouse(lua_toboolean(L,1)!=0);return 0;}
int apiMouseCaptured(lua_State* L){lua_pushboolean(L,context(L)->mouseCaptured());return 1;}
int apiFPSCamera(lua_State* L){
 auto* rt=context(L);
 if(auto* sc=rt->scene()){
  auto& cam=sc->gameCamera;
  cam.firstPerson=true;
  cam.eyePosition={float(luaL_checknumber(L,1)),float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3))};
  cam.yaw=float(luaL_checknumber(L,4));
  cam.pitch=std::clamp(float(luaL_checknumber(L,5)),-89.f,89.f);
  cam.allowOrbit=false;
 }
 return 0;
}
int apiSetVelocity(lua_State* L){auto* e=entityArg(L,context(L));if(e)e->velocity={float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3)),float(luaL_checknumber(L,4))};return 0;}
int apiGetVelocity(lua_State* L){auto* e=entityArg(L,context(L));if(!e){lua_pushnil(L);return 1;}
 lua_pushnumber(L,e->velocity.x);lua_pushnumber(L,e->velocity.y);lua_pushnumber(L,e->velocity.z);return 3;}
int apiGrounded(lua_State* L){auto* e=entityArg(L,context(L));lua_pushboolean(L,e&&e->grounded);return 1;}
int apiSetGravity(lua_State* L){auto* e=entityArg(L,context(L));if(e)e->physics=lua_toboolean(L,2)!=0;return 0;}
int apiLight(lua_State* L){auto* rt=context(L);int kind=int(luaL_checkinteger(L,2));
 if(auto* e=entityArg(L,rt)){e->kind=ComponentKind(std::clamp(kind,1,3));e->intensity=float(luaL_optnumber(L,3,1));}return 0;}
int apiFire(lua_State* L){auto* rt=context(L);auto& e=rt->scene()->create("Fire",Shape::Sphere);
 e.kind=ComponentKind::Fire;e.position={float(luaL_checknumber(L,1)),float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3))};
 e.scale={.8f,1.6f,.8f};e.color={1.f,.34f,.05f};lua_pushinteger(L,static_cast<lua_Integer>(e.id));return 1;}
int apiExplosion(lua_State* L){auto* rt=context(L);auto& e=rt->scene()->create("Explosion",Shape::Sphere);
 e.kind=ComponentKind::Explosion;e.position={float(luaL_checknumber(L,1)),float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3))};
 e.effectDuration=std::max(.05f,float(luaL_optnumber(L,4,1.2)));
 e.color={1.f,.45f,.09f};lua_pushinteger(L,static_cast<lua_Integer>(e.id));return 1;}
int apiUIText(lua_State* L){auto* rt=context(L);UIElement item;item.type=UIElement::Type::Text;
 item.text=luaL_checkstring(L,1);item.x=float(luaL_checknumber(L,2));item.y=float(luaL_checknumber(L,3));
 item.r=float(luaL_optnumber(L,4,1));item.g=float(luaL_optnumber(L,5,1));item.b=float(luaL_optnumber(L,6,1));rt->ui().add(std::move(item));return 0;}
int apiUIRect(lua_State* L){auto* rt=context(L);UIElement item;item.type=UIElement::Type::Rectangle;
 item.x=float(luaL_checknumber(L,1));item.y=float(luaL_checknumber(L,2));item.w=float(luaL_checknumber(L,3));item.h=float(luaL_checknumber(L,4));
 item.r=float(luaL_optnumber(L,5,.1));item.g=float(luaL_optnumber(L,6,.15));item.b=float(luaL_optnumber(L,7,.2));
 item.a=float(luaL_optnumber(L,8,.85));rt->ui().add(std::move(item));return 0;}
int apiUIButton(lua_State* L){auto* rt=context(L);const std::string text=luaL_checkstring(L,1);
 const float x=float(luaL_checknumber(L,2)),y=float(luaL_checknumber(L,3));
 const float w=float(luaL_checknumber(L,4)),h=float(luaL_checknumber(L,5));
 lua_pushboolean(L,rt->ui().button(text,x,y,w,h));return 1;}
int apiUIMouse(lua_State* L){context(L)->mouseVisible(lua_toboolean(L,1)!=0);return 0;}
int apiQuit(lua_State* L){context(L)->quitGame();return 0;}
int apiPause(lua_State* L){context(L)->setPause(lua_toboolean(L,1)!=0);return 0;}
int apiPaused(lua_State* L){lua_pushboolean(L,context(L)->paused());return 1;}
int apiRaycast(lua_State* L){auto* rt=context(L);
 const Vec3 origin{float(luaL_checknumber(L,1)),float(luaL_checknumber(L,2)),float(luaL_checknumber(L,3))};
 const Vec3 direction=norm(Vec3{float(luaL_checknumber(L,4)),float(luaL_checknumber(L,5)),float(luaL_checknumber(L,6))});
 const float maxDistance=std::clamp(float(luaL_optnumber(L,7,100.f)),.01f,10000.f);
 float closest=maxDistance;EntityId hit=0;
 for(const auto& e:rt->scene()->entities()){
  if(!e.visible||e.kind!=ComponentKind::Mesh)continue;
  const Vec3 min=e.position-e.scale*.5f,max=e.position+e.scale*.5f;
  float near=0,far=maxDistance;
  bool valid=true;
  for(int axis=0;axis<3;++axis){
   const float o=axis==0?origin.x:axis==1?origin.y:origin.z;
   const float d=axis==0?direction.x:axis==1?direction.y:direction.z;
   const float lo=axis==0?min.x:axis==1?min.y:min.z;
   const float hi=axis==0?max.x:axis==1?max.y:max.z;
   if(std::fabs(d)<1e-6f){if(o<lo||o>hi){valid=false;break;}}
   else{const float a=(lo-o)/d,b=(hi-o)/d;near=std::max(near,std::min(a,b));far=std::min(far,std::max(a,b));if(far<near){valid=false;break;}}
  }
  if(valid&&near<closest){closest=near;hit=e.id;}
 }
 if(hit){lua_pushinteger(L,static_cast<lua_Integer>(hit));lua_pushnumber(L,closest);return 2;}
 lua_pushnil(L);return 1;
}
int apiTime(lua_State* L){lua_pushnumber(L,context(L)->elapsed());return 1;}
void bind(lua_State* L,LuaRuntime* ctx,const char* name,lua_CFunction callback){lua_pushlightuserdata(L,ctx);lua_pushcclosure(L,callback,1);lua_setfield(L,-2,name);}
}
void LuaRuntime::log(std::string t){if(logs_.size()>=400)logs_.erase(logs_.begin());logs_.push_back(std::move(t));}
void LuaRuntime::registerAPI(){
 lua_newtable(state_);
 bind(state_,this,"log",apiLog);bind(state_,this,"get_position",apiGetPos);bind(state_,this,"set_position",apiSetPos);
 bind(state_,this,"translate",apiTranslate);bind(state_,this,"rotate",apiRotate);bind(state_,this,"set_rotation",apiSetRotation);
 bind(state_,this,"get_rotation",apiGetRotation);bind(state_,this,"set_scale",apiSetScale);
 bind(state_,this,"set_color",apiSetColor);bind(state_,this,"set_visible",apiSetVisible);
 bind(state_,this,"find_entity",apiFind);bind(state_,this,"spawn",apiSpawn);bind(state_,this,"destroy",apiDestroy);
 bind(state_,this,"key_down",apiKey);bind(state_,this,"time",apiTime);
 bind(state_,this,"set_camera_target",apiSetCameraTarget);
 bind(state_,this,"set_camera_angles",apiSetCameraAngles);
 bind(state_,this,"set_camera_distance",apiSetCameraDistance);
 bind(state_,this,"get_camera_target",apiGetCameraTarget);
 bind(state_,this,"mouse_delta",apiMouseDelta);
 bind(state_,this,"mouse_down",apiMouseDown);bind(state_,this,"get_camera_angles",apiGetCameraAngles);
 bind(state_,this,"capture_mouse",apiCaptureMouse);
 bind(state_,this,"mouse_captured",apiMouseCaptured);
 bind(state_,this,"set_fps_camera",apiFPSCamera);
 bind(state_,this,"set_velocity",apiSetVelocity);bind(state_,this,"get_velocity",apiGetVelocity);
 bind(state_,this,"is_grounded",apiGrounded);bind(state_,this,"set_gravity",apiSetGravity);
 bind(state_,this,"set_light",apiLight);bind(state_,this,"spawn_fire",apiFire);
 bind(state_,this,"spawn_explosion",apiExplosion);bind(state_,this,"raycast",apiRaycast);
 bind(state_,this,"ui_text",apiUIText);bind(state_,this,"ui_rect",apiUIRect);
 bind(state_,this,"ui_button",apiUIButton);bind(state_,this,"ui_mouse_visible",apiUIMouse);
 bind(state_,this,"quit_game",apiQuit);bind(state_,this,"set_paused",apiPause);bind(state_,this,"is_paused",apiPaused);
 lua_pushstring(state_,"0.4.2-alpha");
 lua_setfield(state_,-2,"api_version");
 lua_setglobal(state_,"Engine");
 // Scripts are local project code; remove direct filesystem and OS execution API.
 for(const char* global:{"io","os","debug","package","dofile","loadfile"}){lua_pushnil(state_);lua_setglobal(state_,global);}
}
bool LuaRuntime::call(int ref,const char* func,EntityId id,float dt,bool withDt){
 lua_rawgeti(state_,LUA_REGISTRYINDEX,ref);
 lua_getfield(state_,-1,func);
 if(!lua_isfunction(state_,-1)){lua_pop(state_,2);return true;}
 lua_pushinteger(state_,static_cast<lua_Integer>(id));
 if(withDt)lua_pushnumber(state_,dt);
 if(lua_pcall(state_,withDt?2:1,0,0)!=LUA_OK){log(std::string("[Lua] ")+lua_tostring(state_,-1));lua_pop(state_,2);return false;}
 lua_pop(state_,1);return true;
}
bool LuaRuntime::start(Scene& scene,const std::filesystem::path& root){
 return start(scene,std::make_shared<AssetStore>(std::filesystem::absolute(root).lexically_normal()));
}
bool LuaRuntime::start(Scene& scene,std::shared_ptr<AssetStore> resources){
 stop();scene_=&scene;resources_=std::move(resources);
 if(!resources_){log("Asset storage not available");return false;}
 root_=resources_->root();elapsed_=0;paused_=false;
 state_=luaL_newstate();if(!state_){log("Cannot create Lua state");return false;}
 luaL_openlibs(state_);registerAPI();
 // Report the compiled native API. This distinguishes a stale executable from
 // an up-to-date fps_controller.lua after replacing project files.
 log("[Lua] Auvyron Engine API: 0.4.2-alpha");
 bool ok=true;
 // Snapshot initial entities so scripts may safely spawn new entities during on_start.
 std::vector<std::pair<EntityId,std::string>> pending;
 for(const auto& e:scene.entities())if(!e.script.empty())pending.emplace_back(e.id,e.script);
 for(const auto& [id,script]:pending){
  std::string source;
  if(!resources_->readText(script,source)){
   log("[Lua] Script unavailable or unsafe: "+script);ok=false;continue;
  }
  if(luaL_loadbufferx(state_,source.data(),source.size(),("@"+script).c_str(),"t")!=LUA_OK){
   log(std::string("[Lua] ")+lua_tostring(state_,-1));lua_pop(state_,1);ok=false;continue;
  }
  if(lua_pcall(state_,0,1,0)!=LUA_OK){log(std::string("[Lua] ")+lua_tostring(state_,-1));lua_pop(state_,1);ok=false;continue;}
  if(!lua_istable(state_,-1)){log("[Lua] "+script+" must return a table");lua_pop(state_,1);ok=false;continue;}
  scripts_[id]=luaL_ref(state_,LUA_REGISTRYINDEX);
 }
 for(auto [id,ref]:scripts_)if(!call(ref,"on_start",id))ok=false;
 log(ok?"[Lua] Runtime started":"[Lua] Runtime started with script errors");
 return ok;
}
void LuaRuntime::update(float dt){
 if(!state_)return;
 dt=std::clamp(dt,0.f,0.1f);if(!paused_)elapsed_+=dt;
 for(auto [id,ref]:scripts_)if(scene_->find(id))call(ref,"on_update",id,dt,true);
}
void LuaRuntime::stop(){
 if(state_){for(auto [id,ref]:scripts_)if(scene_&&scene_->find(id))call(ref,"on_stop",id);
  for(auto [id,ref]:scripts_)luaL_unref(state_,LUA_REGISTRYINDEX,ref);
  lua_close(state_);state_=nullptr;
 }
 scripts_.clear();scene_=nullptr;resources_.reset();elapsed_=0;mouseDx_=mouseDy_=0;paused_=false;
}
}
