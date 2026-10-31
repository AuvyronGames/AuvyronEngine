/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/Scene.cpp
 * @license MIT
 */

#include "core/Scene.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
namespace auv {
Entity& Scene::create(std::string name,Shape s){Entity e;e.id=nextId_++;e.name=std::move(name);e.shape=s;entities_.push_back(std::move(e));return entities_.back();}
Entity* Scene::find(EntityId id){auto it=std::find_if(entities_.begin(),entities_.end(),[=](auto& e){return e.id==id;});return it==entities_.end()?nullptr:&*it;}
const Entity* Scene::find(EntityId id)const{auto it=std::find_if(entities_.begin(),entities_.end(),[=](auto& e){return e.id==id;});return it==entities_.end()?nullptr:&*it;}
Entity* Scene::findName(const std::string& name){auto it=std::find_if(entities_.begin(),entities_.end(),[&](auto& e){return e.name==name;});return it==entities_.end()?nullptr:&*it;}
bool Scene::remove(EntityId id){
 auto n=entities_.size();std::erase_if(entities_,[=](const Entity& e){return e.id==id;});
 if(gameCamera.activeEntity==id)gameCamera.activeEntity=0;
 return n!=entities_.size();
}
void Scene::clear(){entities_.clear();nextId_=1;skybox={};gameCamera={};}
static bool goodString(const std::string& s){return s.size()<16384 && s.find('\0')==std::string::npos;}
bool Scene::save(const std::filesystem::path& file)const{
 try{
  if(file.has_parent_path())std::filesystem::create_directories(file.parent_path());
  auto tmp=file;tmp+=".tmp";std::ofstream o(tmp,std::ios::binary|std::ios::trunc);
  if(!o){error_="Cannot open scene for writing";return false;}
  o<<"AUVSCENE 3\n"<<entities_.size()<<'\n'<<std::setprecision(9);
  for(auto& e:entities_){
   if(!goodString(e.name)||!goodString(e.script)||!goodString(e.mesh)){error_="Invalid string";return false;}
   o<<e.id<<' '<<int(e.shape)<<' '<<e.visible<<' '<<e.physics<<' '<<e.staticBody<<'\n';
   o<<std::quoted(e.name)<<' '<<std::quoted(e.mesh)<<' '<<std::quoted(e.script)<<'\n';
   for(Vec3 v:{e.position,e.rotation,e.scale,e.color,e.velocity})o<<v.x<<' '<<v.y<<' '<<v.z<<'\n';
   o<<"COMP "<<int(e.kind)<<' '<<e.lightColor.x<<' '<<e.lightColor.y<<' '<<e.lightColor.z
    <<' '<<e.intensity<<' '<<e.range<<' '<<e.spotAngle<<' '<<e.effectDuration<<'\n';
  }
  o<<"SKYBOX "<<skybox.enabled<<' '<<skybox.useCubemap<<'\n';
  for(Vec3 v:{skybox.zenith,skybox.horizon})o<<v.x<<' '<<v.y<<' '<<v.z<<'\n';
  for(const auto& face:skybox.faces){if(!goodString(face)){error_="Invalid skybox path";return false;}o<<std::quoted(face)<<'\n';}
  o<<"CAMERA "<<gameCamera.target.x<<' '<<gameCamera.target.y<<' '<<gameCamera.target.z<<' '
    <<gameCamera.yaw<<' '<<gameCamera.pitch<<' '<<gameCamera.distance<<' '<<gameCamera.allowOrbit<<' '<<gameCamera.activeEntity<<'\n';
  o.flush();if(!o){error_="Write failed";return false;}o.close();
  std::error_code ec;std::filesystem::rename(tmp,file,ec);
  if(ec){std::filesystem::remove(file,ec);ec.clear();std::filesystem::rename(tmp,file,ec);}
  if(ec){error_=ec.message();return false;}
  return true;
 }catch(const std::exception& ex){error_=ex.what();return false;}
}
bool Scene::load(const std::filesystem::path& file){
 std::ifstream in(file,std::ios::binary);if(!in){error_="Cannot open "+file.string();return false;}
 std::ostringstream contents;contents<<in.rdbuf();return loadText(contents.str());
}
bool Scene::loadText(const std::string& content){
 std::istringstream in(content);
 std::string magic;int version=0;std::size_t count=0;
 if(!(in>>magic>>version>>count)||magic!="AUVSCENE"||(version!=1&&version!=2&&version!=3)||count>100000){error_="Invalid scene header";return false;}
 std::vector<Entity> loaded;loaded.reserve(count);EntityId high=0;
 for(std::size_t i=0;i<count;++i){Entity e;int sh=0;
  if(!(in>>e.id>>sh>>e.visible>>e.physics>>e.staticBody)||sh<0||sh>3||e.id==0){error_="Invalid entity header";return false;}
  e.shape=Shape(sh);
  if(!(in>>std::quoted(e.name)>>std::quoted(e.mesh)>>std::quoted(e.script))||!goodString(e.name)||!goodString(e.mesh)||!goodString(e.script)){error_="Invalid entity strings";return false;}
  for(Vec3* v:{&e.position,&e.rotation,&e.scale,&e.color,&e.velocity})
   if(!(in>>v->x>>v->y>>v->z)||!std::isfinite(v->x)||!std::isfinite(v->y)||!std::isfinite(v->z)){error_="Invalid vector";return false;}
  if(version>=3){
   std::string token;int kind=0;
   if(!(in>>token>>kind>>e.lightColor.x>>e.lightColor.y>>e.lightColor.z
      >>e.intensity>>e.range>>e.spotAngle>>e.effectDuration)||token!="COMP"||kind<0||kind>6){
    error_="Invalid entity component";return false;
   }
   e.kind=ComponentKind(kind);
  }
  if(e.id<=high){error_="IDs must be ascending";return false;}
  high=e.id;loaded.push_back(std::move(e));
 }
 SkyboxSettings newSky;
 GameCameraSettings newCamera;
 if(version>=2){
  std::string keyword;
  if(!(in>>keyword)||keyword!="SKYBOX"||!(in>>newSky.enabled>>newSky.useCubemap)){
   error_="Invalid skybox header";return false;
  }
  for(Vec3* v:{&newSky.zenith,&newSky.horizon}){
   if(!(in>>v->x>>v->y>>v->z)||!std::isfinite(v->x)||!std::isfinite(v->y)||!std::isfinite(v->z)){
    error_="Invalid skybox color";return false;
   }
  }
  for(auto& face:newSky.faces)if(!(in>>std::quoted(face))||!goodString(face)){
   error_="Invalid skybox texture path";return false;
  }
  if(!(in>>keyword)||keyword!="CAMERA"||!(in>>newCamera.target.x>>newCamera.target.y>>newCamera.target.z
   >>newCamera.yaw>>newCamera.pitch>>newCamera.distance>>newCamera.allowOrbit)||
   !std::isfinite(newCamera.distance)||newCamera.distance<0.1f){
   error_="Invalid game camera";return false;
  }
  if(version>=3 && !(in>>newCamera.activeEntity)){
   error_="Invalid active game camera";return false;
  }
 }
 skybox=std::move(newSky);gameCamera=newCamera;
 entities_=std::move(loaded);nextId_=high+1;return true;
}
void Scene::stepPhysics(float dt){
 dt=std::clamp(dt,0.f,1.f/30.f);
 for(auto& e:entities_){
  if(e.kind==ComponentKind::Explosion){e.effectAge+=dt;
   if(e.effectAge>e.effectDuration)e.visible=false;}
  if(e.kind==ComponentKind::Fire)e.effectAge+=dt;
  if(!e.physics||e.staticBody)continue;
  e.grounded=false;
  const float halfY=std::max(0.01f,std::abs(e.scale.y)*0.5f);
  const float floorY=halfY;
  e.velocity.y-=9.81f*dt;
  e.position+=e.velocity*dt;
  if(e.position.y<floorY){e.position.y=floorY;if(e.velocity.y<0)e.velocity.y=0;e.grounded=true;}
  // First-pass static AABB collision for box/platform primitives.
  for(const auto& other:entities_){
   if(other.id==e.id||!other.staticBody||!other.visible||other.kind!=ComponentKind::Mesh)continue;
   const auto px=std::abs(e.position.x-other.position.x),pz=std::abs(e.position.z-other.position.z);
   const float hx=(std::abs(e.scale.x)+std::abs(other.scale.x))*.5f;
   const float hz=(std::abs(e.scale.z)+std::abs(other.scale.z))*.5f;
   if(px>=hx||pz>=hz)continue;
   const float top=other.shape==Shape::Plane?other.position.y:
    other.position.y+std::max(.01f,std::abs(other.scale.y))*.5f;
   const float bottom=e.position.y-halfY;
   if(e.velocity.y<=0&&bottom<=top&&bottom>=top-.25f){
    e.position.y=top+halfY;e.velocity.y=0;e.grounded=true;
   }
  }
 }
}
}
