/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/Project.cpp
 * @license MIT
 */

#include "core/Project.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
namespace auv {
bool Project::load(const std::filesystem::path& path){
 auto manifest=std::filesystem::is_directory(path)?path/"project.auvproj":path;
 std::ifstream in(manifest);std::string magic;
 std::string sceneString;
 if(!(in>>magic)||magic!="AUVPROJ1"||!(in>>std::quoted(name)>>std::quoted(sceneString)))return false;
 scene=sceneString;
 root=std::filesystem::absolute(manifest.parent_path());return true;
}
bool Project::loadText(const std::string& content,const std::filesystem::path& virtualRoot){
 std::istringstream in(content);std::string magic,sceneString;
 if(!(in>>magic)||magic!="AUVPROJ1"||!(in>>std::quoted(name)>>std::quoted(sceneString)))return false;
 scene=sceneString;
 if(scene.is_absolute()||scene.generic_string().find("..")!=std::string::npos)return false;
 root=virtualRoot;return true;
}
bool Project::save()const{
 std::filesystem::create_directories(root);
 std::ofstream out(root/"project.auvproj",std::ios::trunc);
 if(!out)return false;
 out<<"AUVPROJ1\n"<<std::quoted(name)<<" "<<std::quoted(scene.generic_string())<<"\n";
 return bool(out);
}
bool Project::create(const std::filesystem::path& directory,std::string projectName){
 root=std::filesystem::absolute(directory);name=std::move(projectName);scene="scenes/main.auvscene";
 std::filesystem::create_directories(root/"scenes");std::filesystem::create_directories(scriptsPath());
 // Include the optional skybox starter preset in newly created projects.
 const std::filesystem::path preset="assets/skyboxes/Daylight";
 for(const auto& templateRoot:{std::filesystem::current_path()/"examples/StarterWorld",
                               std::filesystem::current_path()/"StarterWorld"}){
  std::error_code ec;
  if(!std::filesystem::is_directory(templateRoot/preset,ec))continue;
  std::filesystem::create_directories(root/preset,ec);
  std::filesystem::copy(templateRoot/preset,root/preset,
   std::filesystem::copy_options::recursive|std::filesystem::copy_options::overwrite_existing,ec);
  const auto templates=templateRoot/"assets"/"scripts";
  if(std::filesystem::is_directory(templates,ec)){
   for(const auto& entry:std::filesystem::directory_iterator(templates,ec)){
    if(entry.is_regular_file()&&entry.path().extension()==".lua"){
     std::filesystem::copy_file(entry.path(),scriptsPath()/entry.path().filename(),
       std::filesystem::copy_options::skip_existing,ec);
    }
   }
  }
  break;
 }
 std::ofstream out(scriptsPath()/"example.lua");
 out<<"local M = {}\nfunction M.on_start(id)\n    Engine.log('Hello from Lua!')\nend\n"
       "function M.on_update(id, dt)\n    Engine.rotate(id, 0, 35 * dt, 0)\nend\nreturn M\n";
 return save();
}
}
