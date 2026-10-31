/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/Editor.cpp
 * @license MIT
 */

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "app/Editor.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <ImGuizmo.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
namespace auv {
namespace {
constexpr auto fixed=ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize;
const ImVec4 accent{0.32f,0.77f,0.91f,1};
std::string lower(std::string s){for(char& c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
std::string generic(const std::filesystem::path& p){return p.generic_string();}
std::string shapeLabel(Shape sh){switch(sh){case Shape::Cube:return "Cube";case Shape::Plane:return "Plane";case Shape::Sphere:return "Sphere";default:return "OBJ mesh";}}
void section(const char* name){ImGui::Spacing();ImGui::SeparatorText(name);}
void tooltip(const char* name){if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))ImGui::SetTooltip("%s",name);}
void helpLabel(const char* text){ImGui::TextDisabled("%s",text);}
void propertyLabel(const char* text){ImGui::AlignTextToFramePadding();ImGui::TextDisabled("%s",text);ImGui::SameLine(102.f);ImGui::SetNextItemWidth(-1);}
void actionIcon(ImDrawList* d,ImVec2 c,int kind,ImU32 color){
 auto line=[&](float x1,float y1,float x2,float y2){d->AddLine({c.x+x1,c.y+y1},{c.x+x2,c.y+y2},color,1.8f);};
 if(kind==0){ // Move: axis arrows
  line(-7,6,7,-8);line(7,-8,1,-7);line(7,-8,6,-2);line(-6,1,-6,-8);line(-6,-8,-9,-4);
 }else if(kind==1){d->AddCircle(c,7,color,28,1.8f);line(4,-7,9,-7);line(9,-7,9,-2);}
 else if(kind==2){d->AddRect({c.x-7,c.y-7},{c.x+7,c.y+7},color,0,0,1.7f);line(-2,2,7,-7);}
 else if(kind==3){d->AddRect({c.x-7,c.y-8},{c.x+7,c.y+8},color,1,0,1.6f);line(-4,-2,-1,1);line(-1,1,5,-5);}
 else if(kind==4){d->AddTriangleFilled({c.x-5,c.y-7},{c.x+8,c.y},{c.x-5,c.y+7},color);}
 else if(kind==5){d->AddRectFilled({c.x-6,c.y-6},{c.x+6,c.y+6},color,1);}
 else if(kind==6){d->AddRect({c.x-7,c.y-6},{c.x+7,c.y+7},color,1,0,1.5f);line(-2,-9,0,-9);line(0,-9,5,-3);}
 else if(kind==7){d->AddLine({c.x-7,c.y},{c.x+7,c.y},color,1.8f);d->AddLine({c.x,c.y-7},{c.x,c.y+7},color,1.8f);}
}
bool iconButton(const char* id,const char* tip,int kind,bool active=false,ImVec2 size={34,30}){
 ImGui::PushID(id);
 const ImVec2 pos=ImGui::GetCursorScreenPos();
 bool pressed=ImGui::InvisibleButton("##button",size);
 auto* d=ImGui::GetWindowDrawList();
 const bool hover=ImGui::IsItemHovered();
 ImU32 bg=ImGui::GetColorU32(active?ImVec4(.16f,.36f,.43f,1):hover?ImVec4(.16f,.23f,.28f,1):ImVec4(.09f,.12f,.16f,1));
 d->AddRectFilled(pos,{pos.x+size.x,pos.y+size.y},bg,5);
 if(active)d->AddRect(pos,{pos.x+size.x,pos.y+size.y},IM_COL32(77,186,217,220),5,0,1);
 actionIcon(d,{pos.x+size.x*.5f,pos.y+size.y*.5f},kind,active?IM_COL32(107,225,249,255):IM_COL32(197,209,222,255));
 tooltip(tip);
 ImGui::PopID();return pressed;
}
void beginFixed(const char* name,ImVec2 pos,ImVec2 size,ImGuiWindowFlags extras=0){
 ImGui::SetNextWindowPos(pos);ImGui::SetNextWindowSize(size);ImGui::Begin(name,nullptr,fixed|extras);
}
bool childPath(const std::filesystem::path& root,const std::filesystem::path& candidate){
 std::error_code ec;
 const auto a=std::filesystem::weakly_canonical(root,ec);if(ec)return false;
 const auto b=std::filesystem::weakly_canonical(candidate,ec);if(ec)return false;
 auto ai=a.begin(),bi=b.begin();
 for(;ai!=a.end();++ai,++bi)if(bi==b.end()||*ai!=*bi)return false;
 return true;
}
std::vector<std::filesystem::directory_entry> sortedEntries(const std::filesystem::path& folder){
 std::vector<std::filesystem::directory_entry> r;std::error_code ec;
 for(std::filesystem::directory_iterator it(folder,ec),end;!ec&&it!=end;it.increment(ec))r.push_back(*it);
 std::sort(r.begin(),r.end(),[](const auto& a,const auto& b){
  if(a.is_directory()!=b.is_directory())return a.is_directory();
  return lower(a.path().filename().string())<lower(b.path().filename().string());
 });return r;
}
std::string safeFileName(std::string s){
 if(s.empty())return "Game";
 for(char& c:s)if(!std::isalnum(static_cast<unsigned char>(c))&&c!='_'&&c!='-')c='_';
 return s;
}
}
bool Editor::init(const std::filesystem::path& folder){
 if(!renderer_.init()){pushMessage("OpenGL ERROR: "+renderer_.error());return false;}
 if(!project_.load(folder)){
  if(!project_.create(folder,"StarterWorld")){pushMessage("Could not create project");return false;}
  auto& ground=scene_.create("Ground",Shape::Plane);ground.scale={30,1,30};ground.color={.24f,.36f,.28f};
  auto& cube=scene_.create("Demo Cube");cube.position.y=1;cube.script="assets/scripts/example.lua";
  scene_.save(project_.scenePath());
 }else if(!scene_.load(project_.scenePath())){
  scene_.clear();auto& cube=scene_.create("Cube");cube.position.y=.5f;
  scene_.save(project_.scenePath());
 }
 std::snprintf(projectPath_,sizeof(projectPath_),"%s",project_.root.parent_path().string().c_str());
 std::snprintf(buildPath_,sizeof(buildPath_),"%s",(project_.root.parent_path()/"Builds").string().c_str());
 std::snprintf(gameName_,sizeof(gameName_),"%s",safeFileName(project_.name).c_str());
 auto language=TextEditor::LanguageDefinition::Lua();
 for(const auto* api:{"Engine","log","get_position","set_position","translate","rotate","set_rotation",
  "get_rotation","set_scale","set_color","set_visible","find_entity","spawn","destroy","key_down","time",
  "on_start","on_update","on_stop","set_camera_target","set_camera_angles","set_camera_distance","get_camera_target",
  "mouse_delta","capture_mouse","mouse_captured","set_fps_camera",
  "set_velocity","get_velocity","is_grounded","set_gravity","set_light",
  "spawn_fire","spawn_explosion","raycast","ui_text","ui_rect","ui_button",
  "ui_mouse_visible","set_paused","is_paused","quit_game","mouse_down","get_camera_angles"}){
  TextEditor::Identifier identifier;identifier.mDeclaration=std::string("Auvyron API: ")+api;
  language.mIdentifiers.insert_or_assign(api,identifier);
 }
 scriptEditor_.SetLanguageDefinition(language);
 scriptEditor_.SetPalette(TextEditor::GetDarkPalette());
 scriptEditor_.SetTabSize(4);
 scanAssets();lua_.setKeyCallback([](const std::string&){return false;});
 pushMessage("Project opened: "+project_.name);return true;
}
void Editor::shutdown(){if(playing_)stop();renderer_.shutdown();}
void Editor::pushMessage(std::string m){message_=std::move(m);lua_.log("[Editor] "+message_);}
void Editor::scanAssets(){
 assets_.clear();std::error_code ec;auto base=project_.root/"assets";
 std::filesystem::create_directories(base,ec);
 for(std::filesystem::recursive_directory_iterator it(base,ec),end;!ec&&it!=end;it.increment(ec))
  if(it->is_regular_file())assets_.push_back(std::filesystem::relative(it->path(),project_.root,ec));
 std::sort(assets_.begin(),assets_.end());
}
void Editor::save(){
 if(playing_){pushMessage("Stop Play Mode before saving");return;}
 if(scriptDirty_)saveScript();
 if(scene_.save(project_.scenePath())&&project_.save()){
  dirty_=false;pushMessage("Scene and project saved");
 }else pushMessage("SAVE FAILED: "+scene_.error());
}
void Editor::addEntity(Shape shape){
 if(playing_)return;
 auto& e=scene_.create(shapeLabel(shape),shape);
 e.position.y=shape==Shape::Plane?0.f:.5f;
 if(shape==Shape::Plane)e.scale={10,1,10};
 selected_=e.id;dirty_=true;
}
void Editor::addComponent(ComponentKind kind){
 if(playing_)return;
 const char* names[]={"Mesh","Directional Light","Point Light","Spotlight","Camera","Fire","Explosion"};
 auto& e=scene_.create(names[int(kind)],kind==ComponentKind::Fire||kind==ComponentKind::Explosion?Shape::Sphere:Shape::Cube);
 e.kind=kind;e.position={0,kind==ComponentKind::Camera?2.f:3.f,0};e.scale={1,1,1};
 if(kind==ComponentKind::Fire){e.color={1.f,.35f,.05f};e.scale={.8f,1.6f,.8f};}
 else if(kind==ComponentKind::Explosion){e.color={1.f,.5f,.05f};e.effectDuration=1.2f;}
 selected_=e.id;showSceneSettings_=false;dirty_=true;
}
void Editor::applyTheme(){
 auto& style=ImGui::GetStyle();
 if(lightTheme_){
  ImGui::StyleColorsLight();
  auto& c=style.Colors;
  c[ImGuiCol_WindowBg]={.94f,.96f,.98f,1};
  c[ImGuiCol_TitleBgActive]={.86f,.9f,.95f,1};
  c[ImGuiCol_Button]={.79f,.87f,.94f,1};
  c[ImGuiCol_Header]={.74f,.85f,.94f,.76f};
 }else{
  ImGui::StyleColorsDark();
  auto& c=style.Colors;
  c[ImGuiCol_WindowBg]={.052f,.067f,.087f,1};
  c[ImGuiCol_TitleBgActive]={.065f,.095f,.128f,1};
  c[ImGuiCol_Button]={.11f,.18f,.24f,1};
  c[ImGuiCol_Header]={.12f,.24f,.30f,.86f};
 }
 scriptEditor_.SetPalette(lightTheme_?TextEditor::GetLightPalette():TextEditor::GetDarkPalette());
 style.WindowRounding=6.f;style.FrameRounding=5.f;style.ChildRounding=5.f;
 style.WindowPadding={11,10};style.FramePadding={8,6};style.ItemSpacing={8,7};
}
void Editor::start(){
 if(playing_)return;
 if(scriptDirty_)saveScript();
 snapshot_=scene_;editorCameraSnapshot_=camera_;playing_=true;
 if(!lua_.start(scene_,project_.root))pushMessage("Lua errors detected: open Console");
 else pushMessage("Play Mode: Lua scripts running");
}
void Editor::stop(){
 if(!playing_)return;
 if(activeWindow_)mouse_.capture(activeWindow_,false);
 lua_.stop();scene_=snapshot_;camera_=editorCameraSnapshot_;playing_=false;
 pushMessage("Play Mode stopped; scene restored");
}
void Editor::createNewProject(){
 if(playing_)stop();
 auto directory=std::filesystem::path(projectPath_)/projectName_;
 Project created;if(!created.create(directory,projectName_)){pushMessage("Could not create project");return;}
 project_=std::move(created);scene_.clear();selected_=0;
 auto& ground=scene_.create("Ground",Shape::Plane);ground.scale={20,1,20};ground.color={.28f,.35f,.32f};
 auto& cube=scene_.create("Cube");cube.position.y=.5f;cube.script="assets/scripts/example.lua";
 if(std::filesystem::is_regular_file(project_.root/"assets/skyboxes/Daylight/right.png")){
  static constexpr const char* names[]={"right","left","top","bottom","front","back"};
  scene_.skybox.useCubemap=true;
  for(int i=0;i<6;++i)scene_.skybox.faces[std::size_t(i)]=
    std::string("assets/skyboxes/Daylight/")+names[i]+".png";
 }
 selectedAssetFolder_="assets";openScriptPath_.clear();scriptDirty_=false;
 save();scanAssets();showProject_=false;
 std::snprintf(gameName_,sizeof(gameName_),"%s",safeFileName(project_.name).c_str());
}
void Editor::topBar(bool playerOnly){
 if(playerOnly)return;
 const auto& io=ImGui::GetIO();
 beginFixed("##AuvyronToolbar",{0,0},{io.DisplaySize.x,toolbarHeight_},ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_MenuBar);
 if(ImGui::BeginMenuBar()){
  ImGui::TextColored(accent,"AUVYRON ENGINE");ImGui::SameLine();
  ImGui::TextDisabled("v0.4.2 ALPHA");ImGui::SameLine(225.f);
  if(ImGui::BeginMenu("Project")){
   if(ImGui::MenuItem("Project Manager"))showProject_=true;
   if(ImGui::MenuItem("Save Scene","Ctrl+S"))save();
   if(ImGui::MenuItem("Build Game","Ctrl+B"))showBuild_=true;
   ImGui::EndMenu();
  }ImGui::SameLine();
  if(ImGui::BeginMenu("Components")){
   if(ImGui::MenuItem("Cube"))addEntity(Shape::Cube);
   if(ImGui::MenuItem("Sphere"))addEntity(Shape::Sphere);
   if(ImGui::MenuItem("Plane"))addEntity(Shape::Plane);
   ImGui::Separator();
   if(ImGui::MenuItem("Directional Light"))addComponent(ComponentKind::DirectionalLight);
   if(ImGui::MenuItem("Point Light"))addComponent(ComponentKind::PointLight);
   if(ImGui::MenuItem("Spotlight"))addComponent(ComponentKind::SpotLight);
   if(ImGui::MenuItem("Camera"))addComponent(ComponentKind::Camera);
   if(ImGui::MenuItem("GUI Controller (Lua)")){
    addComponent(ComponentKind::Mesh);
    if(auto* e=scene_.find(selected_)){e->name="UI Controller";e->visible=false;
     e->script="assets/scripts/main_menu.lua";dirty_=true;}
   }
   ImGui::Separator();
   if(ImGui::MenuItem("Fire"))addComponent(ComponentKind::Fire);
   if(ImGui::MenuItem("Explosion"))addComponent(ComponentKind::Explosion);
   if(ImGui::MenuItem("World / Skybox")){selected_=0;showSceneSettings_=true;}
   ImGui::EndMenu();
  }ImGui::SameLine();
  if(ImGui::BeginMenu("Asset Manager")){
   if(ImGui::MenuItem("Refresh Assets"))scanAssets();
   if(ImGui::MenuItem("Open asset browser"))ImGui::SetWindowFocus("Project / Assets");
   ImGui::TextDisabled("Always visible in the bottom dock");
   ImGui::EndMenu();
  }ImGui::SameLine();
  if(ImGui::MenuItem("Credits"))showCredits_=true;
  ImGui::SameLine();
  if(ImGui::BeginMenu("Settings")){
   if(ImGui::MenuItem("Dark theme",nullptr,!lightTheme_)){lightTheme_=false;applyTheme();}
   if(ImGui::MenuItem("Light theme",nullptr,lightTheme_)){lightTheme_=true;applyTheme();}
   ImGui::Separator();ImGui::TextDisabled("OpenGL 3.3 | Lua 5.4");
   ImGui::EndMenu();
  }
  ImGui::SameLine();
  ImGui::TextDisabled("  |  %s%s",project_.name.c_str(),dirty_?" *":"");
  if(io.DisplaySize.x>1100.f){ImGui::SameLine(io.DisplaySize.x-154.f);
   if(ImGui::SmallButton("Build Game"))showBuild_=true;}
  ImGui::EndMenuBar();
 }
 ImGui::Spacing();
 if(iconButton("save","Save project (Ctrl+S)",3))save();ImGui::SameLine();
 if(iconButton("cube","Create cube",7))addEntity(Shape::Cube);ImGui::SameLine();
 ImGui::Dummy({6,1});ImGui::SameLine();
 if(iconButton("move","Move (W)",0,gizmo_==GizmoTool::Move))gizmo_=GizmoTool::Move;ImGui::SameLine();
 if(iconButton("rotate","Rotate (E)",1,gizmo_==GizmoTool::Rotate))gizmo_=GizmoTool::Rotate;ImGui::SameLine();
 if(iconButton("scale","Scale (R)",2,gizmo_==GizmoTool::Scale))gizmo_=GizmoTool::Scale;ImGui::SameLine();
 if(ImGui::SmallButton(localSpace_?"LOCAL":"WORLD"))localSpace_=!localSpace_;ImGui::SameLine();
 ImGui::Checkbox("Snap",&snap_);ImGui::SameLine();
 if(snap_){ImGui::SetNextItemWidth(55);
  float* step=gizmo_==GizmoTool::Move?&moveSnap_:gizmo_==GizmoTool::Rotate?&rotateSnap_:&scaleSnap_;
  ImGui::DragFloat("##snap",step,.05f,.01f,180.f,"%.2f");ImGui::SameLine();}
 if(io.DisplaySize.x>825.f){ImGui::SameLine(io.DisplaySize.x-270.f);
  if(!playing_){if(iconButton("play","Play scene",4))start();}
  else if(iconButton("stop","Stop and restore scene",5,true))stop();
  ImGui::SameLine();ImGui::TextDisabled("%s",playing_?"PLAY MODE":"SCENE EDITOR");}
 ImGui::End();
 if(showCredits_){
  ImGui::OpenPopup("Credits | AuvyronEngine");
  if(ImGui::BeginPopupModal("Credits | AuvyronEngine",&showCredits_,ImGuiWindowFlags_AlwaysAutoResize)){
   ImGui::TextColored(accent,"AuvyronEngine 0.4.2 Alpha");ImGui::Separator();
   ImGui::TextUnformatted("Auvyron Games");
   ImGui::TextDisabled("Author: D4NTE");
   ImGui::TextWrapped("C++20 / OpenGL / Lua 5.4 / Dear ImGui");
   ImGui::TextWrapped("Licensed under MIT. Third-party components retain their own licenses.");
   if(ImGui::Button("Close"))showCredits_=false;
   ImGui::EndPopup();
  }
 }
}
void Editor::hierarchy(){
 const auto& io=ImGui::GetIO();
 beginFixed("Hierarchy",{0,toolbarHeight_},{leftWidth_,(std::max)(90.f,io.DisplaySize.y-toolbarHeight_-bottomHeight_)});
 ImGui::TextDisabled("SCENE");ImGui::SameLine();ImGui::TextDisabled("%zu objects",scene_.entities().size());
 ImGui::SameLine(leftWidth_-44.f);
 if(ImGui::SmallButton("+"))ImGui::OpenPopup("CreateEntity");
 if(ImGui::BeginPopup("CreateEntity")){
  if(ImGui::MenuItem("Cube"))addEntity(Shape::Cube);
  if(ImGui::MenuItem("Sphere"))addEntity(Shape::Sphere);
  if(ImGui::MenuItem("Plane"))addEntity(Shape::Plane);
  ImGui::Separator();
  if(ImGui::MenuItem("Directional Light"))addComponent(ComponentKind::DirectionalLight);
  if(ImGui::MenuItem("Point Light"))addComponent(ComponentKind::PointLight);
  if(ImGui::MenuItem("Spotlight"))addComponent(ComponentKind::SpotLight);
  if(ImGui::MenuItem("Camera"))addComponent(ComponentKind::Camera);
  if(ImGui::MenuItem("Fire"))addComponent(ComponentKind::Fire);
  if(ImGui::MenuItem("Explosion"))addComponent(ComponentKind::Explosion);
  ImGui::EndPopup();
 }
 ImGui::Separator();
 if(ImGui::Selectable("[ World / Skybox ]",showSceneSettings_)){showSceneSettings_=true;selected_=0;}
 ImGui::Separator();
 for(const auto& entity:scene_.entities()){
  ImGui::PushID(static_cast<int>(entity.id));
  ImGui::TextColored(entity.visible?ImVec4(.43f,.78f,.87f,1):ImVec4(.35f,.39f,.44f,1),"%s",entity.visible?"[]":"--");ImGui::SameLine();
  if(ImGui::Selectable(entity.name.c_str(),selected_==entity.id)){selected_=entity.id;showSceneSettings_=false;}
  ImGui::PopID();
 }
 if(selected_&&!playing_){
  ImGui::Separator();if(ImGui::SmallButton("Delete selected")){scene_.remove(selected_);selected_=0;dirty_=true;}
 }
 ImGui::End();
}
void Editor::inspector(){
 const auto& io=ImGui::GetIO();
 beginFixed("Inspector",{io.DisplaySize.x-rightWidth_,toolbarHeight_},
  {rightWidth_,(std::max)(90.f,io.DisplaySize.y-toolbarHeight_-bottomHeight_)});
 auto* e=scene_.find(selected_);
 if(!e){sceneSettings();ImGui::End();return;}
 ImGui::PushID(static_cast<int>(e->id));
 ImGui::TextColored(accent,"ENTITY");ImGui::SameLine();
 ImGui::TextDisabled("#%llu",static_cast<unsigned long long>(e->id));
 ImGui::SetNextItemWidth(-55);
 if(ImGui::InputText("##entityname",&e->name))dirty_=true;
 ImGui::SameLine();if(ImGui::Checkbox("##visible",&e->visible))dirty_=true;tooltip("Visible in scene");
 ImGui::BeginDisabled(playing_);
 if(ImGui::CollapsingHeader("Transform",ImGuiTreeNodeFlags_DefaultOpen)){
  propertyLabel("Position");if(ImGui::DragFloat3("##pos",&e->position.x,.06f))dirty_=true;
  propertyLabel("Rotation");if(ImGui::DragFloat3("##rot",&e->rotation.x,.35f))dirty_=true;
  propertyLabel("Scale");if(ImGui::DragFloat3("##scale",&e->scale.x,.03f,.001f,1000.f))dirty_=true;
  if(ImGui::SmallButton("Reset Transform")){
   e->position={};e->rotation={};e->scale={1,1,1};dirty_=true;
  }
 }
 if(e->kind>=ComponentKind::DirectionalLight&&e->kind<=ComponentKind::SpotLight){
  if(ImGui::CollapsingHeader("Light Component",ImGuiTreeNodeFlags_DefaultOpen)){
   int kind=int(e->kind)-1;
   if(ImGui::Combo("Type",&kind,"Directional\0Point\0Spotlight\0")){e->kind=ComponentKind(kind+1);dirty_=true;}
   if(ImGui::ColorEdit3("Light color",&e->lightColor.x))dirty_=true;
   if(ImGui::DragFloat("Intensity",&e->intensity,.04f,0.f,20.f))dirty_=true;
   if(e->kind!=ComponentKind::DirectionalLight&&ImGui::DragFloat("Range",&e->range,.1f,.1f,1000.f))dirty_=true;
   if(e->kind==ComponentKind::SpotLight&&ImGui::SliderFloat("Cone angle",&e->spotAngle,5.f,85.f))dirty_=true;
   helpLabel("Alpha renderer supports one active scene light.");
  }
 }
 if(e->kind==ComponentKind::Camera&&ImGui::CollapsingHeader("Game Camera",ImGuiTreeNodeFlags_DefaultOpen)){
  if(ImGui::Button("Set as game camera")){
   scene_.gameCamera.target=e->position;
   scene_.gameCamera.yaw=e->rotation.y;
   scene_.gameCamera.pitch=e->rotation.x;
   scene_.gameCamera.distance=6.f;scene_.gameCamera.activeEntity=e->id;dirty_=true;
  }
 }
 if((e->kind==ComponentKind::Fire||e->kind==ComponentKind::Explosion)&&
    ImGui::CollapsingHeader("Visual Effect",ImGuiTreeNodeFlags_DefaultOpen)){
  if(e->kind==ComponentKind::Explosion&&ImGui::DragFloat("Duration",&e->effectDuration,.01f,.05f,30.f))dirty_=true;
  helpLabel("Prototype sphere-based VFX; particles are planned.");
 }
 if(e->kind==ComponentKind::Mesh&&ImGui::CollapsingHeader("Mesh Renderer",ImGuiTreeNodeFlags_DefaultOpen)){
  int type=int(e->shape);const char* shapes[]={"Cube","Plane","Sphere","OBJ Mesh"};
  propertyLabel("Mesh");if(ImGui::Combo("##shape",&type,shapes,4)){e->shape=Shape(type);dirty_=true;}
  if(e->shape==Shape::OBJ){propertyLabel("OBJ path");if(ImGui::InputText("##obj",&e->mesh))dirty_=true;}
  propertyLabel("Albedo");if(ImGui::ColorEdit3("##albedo",&e->color.x,ImGuiColorEditFlags_NoInputs))dirty_=true;
 }
 if(ImGui::CollapsingHeader("Physics")){
  if(ImGui::Checkbox("Gravity",&e->physics))dirty_=true;
  if(ImGui::Checkbox("Static",&e->staticBody))dirty_=true;
  propertyLabel("Velocity");if(ImGui::DragFloat3("##velocity",&e->velocity.x,.05f))dirty_=true;
  helpLabel("Gravity, floor and simple AABB platforms. No full rigid-body solver.");
 }
 if(ImGui::CollapsingHeader("Lua Script",ImGuiTreeNodeFlags_DefaultOpen)){
  propertyLabel("Script");if(ImGui::InputText("##script",&e->script))dirty_=true;
  if(ImGui::SmallButton("Open script")&&!e->script.empty())openScript(e->script);
  ImGui::SameLine();
  if(ImGui::SmallButton("New Lua")){
   e->script="assets/scripts/entity_"+std::to_string(e->id)+".lua";
   const auto path=project_.root/e->script;std::filesystem::create_directories(path.parent_path());
   if(!std::filesystem::exists(path)){
    std::ofstream file(path);file<<"local Script = {}\n\nfunction Script.on_start(id)\n    Engine.log('Ready')\nend\n\nfunction Script.on_update(id, dt)\nend\n\nreturn Script\n";
   }
   openScript(e->script);scanAssets();dirty_=true;
  }
  helpLabel("Lifecycle: on_start, on_update, on_stop");
 }
 ImGui::EndDisabled();
 ImGui::PopID();
 if(showSceneSettings_)sceneSettings();
 ImGui::End();
}
void Editor::sceneSettings(){
 ImGui::TextColored(accent,"WORLD SETTINGS");
 ImGui::Separator();
 if(ImGui::CollapsingHeader("Skybox / Environment",ImGuiTreeNodeFlags_DefaultOpen)){
  ImGui::BeginDisabled(playing_);
  if(ImGui::Checkbox("Enable skybox",&scene_.skybox.enabled))dirty_=true;
  int mode=scene_.skybox.useCubemap?1:0;
  if(ImGui::Combo("Mode",&mode,"Gradient\0Six textures (cubemap)\0")){
   scene_.skybox.useCubemap=mode==1;dirty_=true;
  }
  if(ImGui::ColorEdit3("Zenith",&scene_.skybox.zenith.x))dirty_=true;
  if(ImGui::ColorEdit3("Horizon",&scene_.skybox.horizon.x))dirty_=true;
  if(scene_.skybox.useCubemap){
   ImGui::TextDisabled("Six square PNG/JPG images, project-relative paths.");
   static constexpr const char* faces[]={"Right (+X)","Left (-X)","Top (+Y)",
                                         "Bottom (-Y)","Front (+Z)","Back (-Z)"};
   for(int i=0;i<6;++i){
    ImGui::PushID(i);
    if(ImGui::InputText(faces[i],&scene_.skybox.faces[std::size_t(i)]))dirty_=true;
    if(ImGui::BeginDragDropTarget()){
     if(const ImGuiPayload* payload=ImGui::AcceptDragDropPayload("AUV_ASSET")){
      const std::filesystem::path asset(static_cast<const char*>(payload->Data));
      const auto ext=lower(asset.extension().string());
      if(ext==".png"||ext==".jpg"||ext==".jpeg"){
       scene_.skybox.faces[std::size_t(i)]=generic(asset);dirty_=true;
      }
     }
     ImGui::EndDragDropTarget();
    }
    ImGui::PopID();
   }
   if(std::filesystem::is_regular_file(project_.root/"assets/skyboxes/Daylight/right.png") &&
      ImGui::SmallButton("Use included Daylight skybox")){
    static constexpr const char* stems[]={"right","left","top","bottom","front","back"};
    for(int i=0;i<6;++i)scene_.skybox.faces[std::size_t(i)]=
      std::string("assets/skyboxes/Daylight/")+stems[i]+".png";
    dirty_=true;
   }
   if(!std::filesystem::is_regular_file(project_.root/"assets/skyboxes/Daylight/right.png"))
    ImGui::TextDisabled("Drop 6 PNG/JPG images into assets/skyboxes/ to use a cubemap.");
   if(!renderer_.error().empty())ImGui::TextWrapped("Skybox renderer: %s",renderer_.error().c_str());
  }
  ImGui::EndDisabled();
 }
 if(ImGui::CollapsingHeader("Game Camera",ImGuiTreeNodeFlags_DefaultOpen)){
  ImGui::BeginDisabled(playing_);
  if(ImGui::DragFloat3("Target",&scene_.gameCamera.target.x,0.05f))dirty_=true;
  if(ImGui::DragFloat("Yaw",&scene_.gameCamera.yaw,0.2f))dirty_=true;
  if(ImGui::DragFloat("Pitch",&scene_.gameCamera.pitch,0.2f,-85.f,85.f))dirty_=true;
  if(ImGui::DragFloat("Distance",&scene_.gameCamera.distance,0.1f,0.2f,200.f))dirty_=true;
  if(ImGui::Checkbox("Allow orbit mouse controls",&scene_.gameCamera.allowOrbit))dirty_=true;
  if(scene_.gameCamera.activeEntity!=0){
   ImGui::TextDisabled("Active camera component: #%llu",static_cast<unsigned long long>(scene_.gameCamera.activeEntity));
   if(ImGui::SmallButton("Clear camera component")){scene_.gameCamera.activeEntity=0;dirty_=true;}
  }
  if(ImGui::Button("Use current editor view")){
   scene_.gameCamera.activeEntity=0;
   scene_.gameCamera.target=camera_.target;scene_.gameCamera.yaw=camera_.yaw;
   scene_.gameCamera.pitch=camera_.pitch;scene_.gameCamera.distance=camera_.distance;dirty_=true;
  }
  if(ImGui::Button("Preview game camera")){
   camera_.target=scene_.gameCamera.target;camera_.yaw=scene_.gameCamera.yaw;
   camera_.pitch=scene_.gameCamera.pitch;camera_.distance=scene_.gameCamera.distance;
  }
  ImGui::EndDisabled();
 }
}
void Editor::sceneViewport(GLFWwindow* window,float dt,bool playerOnly){
 const auto& io=ImGui::GetIO();
 const float x=playerOnly?0:leftWidth_,y=playerOnly?0:toolbarHeight_;
 const float width=(std::max)(100.f,playerOnly?io.DisplaySize.x:io.DisplaySize.x-leftWidth_-rightWidth_);
 const float height=(std::max)(100.f,playerOnly?io.DisplaySize.y:io.DisplaySize.y-toolbarHeight_-bottomHeight_);
 ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{4,4});
 beginFixed(playerOnly?"##GameRuntime":"Scene",{x,y},{width,height},
  (playerOnly?ImGuiWindowFlags_NoTitleBar:0)|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
 ImVec2 size=ImGui::GetContentRegionAvail();
 const int w=(std::max)(16,int(size.x)),h=(std::max)(16,int(size.y));renderer_.resize(w,h);
 if(playing_){
  // ESC releases the locked cursor without stopping Play. Click the viewport
  // to regain FPS control. Stop is then accessible from the toolbar.
  const bool esc=glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS;
  if(esc&&!escapePreviouslyDown_)mouse_.capture(window,false);
  escapePreviouslyDown_=esc;
  lua_.setMouseCaptureCallback([this,window](bool enabled){mouse_.capture(window,enabled);});
  lua_.setMouseCapturedCallback([this](){return mouse_.captured();});
 lua_.setMouseButtonCallback([window](const std::string& button){
  if(button=="LEFT")return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
  if(button=="RIGHT")return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
  return false;
 });
  mouse_.update(window);
  lua_.setMouseDelta(mouse_.dx(),mouse_.dy());
  lua_.setKeyCallback([window](const std::string& key){
   int code=GLFW_KEY_UNKNOWN;
   if(key.size()==1){const char c=char(std::toupper(static_cast<unsigned char>(key[0])));
    if(c>='A'&&c<='Z')code=GLFW_KEY_A+(c-'A');
    if(c>='0'&&c<='9')code=GLFW_KEY_0+(c-'0');}
   if(key=="SPACE")code=GLFW_KEY_SPACE;
   if(key=="LEFT_SHIFT")code=GLFW_KEY_LEFT_SHIFT;
   if(key=="LEFT_CONTROL"||key=="CTRL")code=GLFW_KEY_LEFT_CONTROL;
   if(key=="ESCAPE")code=GLFW_KEY_ESCAPE;
   return code!=GLFW_KEY_UNKNOWN&&glfwGetKey(window,code)==GLFW_PRESS;
  });
  const ImVec2 viewportOrigin=ImGui::GetCursorScreenPos();
  const bool uiClick=ImGui::IsMouseClicked(ImGuiMouseButton_Left)&&
    io.MousePos.x>=viewportOrigin.x&&io.MousePos.x<=viewportOrigin.x+size.x&&
    io.MousePos.y>=viewportOrigin.y&&io.MousePos.y<=viewportOrigin.y+size.y;
  lua_.ui().beginFrame((io.MousePos.x-viewportOrigin.x)/float(std::max(1,w)),
                       (io.MousePos.y-viewportOrigin.y)/float(std::max(1,h)),uiClick);
  lua_.update(dt);if(!lua_.paused())scene_.stepPhysics(dt);
  const auto& game=scene_.gameCamera;
  camera_.firstPerson=game.firstPerson;
  camera_.position=game.eyePosition;
  camera_.target=game.target;
  camera_.yaw=game.yaw;
  camera_.pitch=game.pitch;
  camera_.distance=game.distance;
  if(!game.firstPerson && game.activeEntity){
   if(const auto* e=scene_.find(game.activeEntity);e&&e->kind==ComponentKind::Camera){
    camera_.firstPerson=true;camera_.position=e->position;
    camera_.yaw=e->rotation.y;camera_.pitch=e->rotation.x;
   }
  }
 }
 renderer_.draw(scene_,camera_,playing_?0:selected_,project_.root,playing_);
 if(playing_)renderer_.drawGameUI(lua_.ui(),false);
 const ImVec2 origin=ImGui::GetCursorScreenPos();
 ImGui::Image((ImTextureID)(intptr_t)renderer_.texture(),size,{0,1},{1,0});
 const bool hovered=ImGui::IsItemHovered();
 if(playing_ && hovered && !mouse_.captured() && !escapePreviouslyDown_ &&
    ImGui::IsMouseClicked(ImGuiMouseButton_Left))mouse_.capture(window,true);
 if(!playerOnly&&!playing_){
  if(ImGui::BeginDragDropTarget()){
   if(const ImGuiPayload* payload=ImGui::AcceptDragDropPayload("AUV_ASSET")){
    const std::string rel(static_cast<const char*>(payload->Data));
    const std::filesystem::path path(rel);
    if(!playing_&&path.extension()==".obj"){
     addEntity(Shape::OBJ);if(auto* e=scene_.find(selected_)){e->mesh=rel;e->name=path.stem().string();}
    }else if(path.extension()==".lua"&&selected_){
     if(auto* e=scene_.find(selected_)){e->script=rel;dirty_=true;}
    }
   }
   ImGui::EndDragDropTarget();
  }
 }
 if(hovered&&!playing_){
  if(ImGui::IsMouseDragging(ImGuiMouseButton_Right))camera_.orbit(io.MouseDelta.x,io.MouseDelta.y);
  if(ImGui::IsMouseDragging(ImGuiMouseButton_Middle))camera_.pan(io.MouseDelta.x,io.MouseDelta.y);
  if(io.MouseWheel!=0)camera_.zoom(io.MouseWheel);
 }
 if(!playerOnly&&!playing_){
  // ImGuizmo is rendered on top of the OpenGL scene and consumes axis interactions.
  ImGuizmo::BeginFrame();
  auto* target=scene_.find(selected_);
  if(target&&target->visible){
   ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
   ImGuizmo::SetRect(origin.x,origin.y,size.x,size.y);
   ImGuizmo::SetOrthographic(false);
   ImGuizmo::Enable(true);
   const auto view=camera_.view();const auto proj=camera_.projection(float(w)/float(h));
   auto model=modelMatrix(target->position,target->rotation,target->scale);
   const auto op=gizmo_==GizmoTool::Move?ImGuizmo::TRANSLATE:
     gizmo_==GizmoTool::Rotate?ImGuizmo::ROTATE:ImGuizmo::SCALE;
   const auto space=localSpace_?ImGuizmo::LOCAL:ImGuizmo::WORLD;
   const float gridStep=gizmo_==GizmoTool::Move?moveSnap_:
    gizmo_==GizmoTool::Rotate?rotateSnap_:scaleSnap_;
   const float snap[3]={gridStep,gridStep,gridStep};
   ImGuizmo::Manipulate(view.m,proj.m,op,space,model.m,nullptr,snap_?snap:nullptr);
   if(ImGuizmo::IsUsing()){
    float pos[3]{},rot[3]{},scale[3]{};
    ImGuizmo::DecomposeMatrixToComponents(model.m,pos,rot,scale);
    target->position={pos[0],pos[1],pos[2]};
    target->rotation={rot[0],rot[1],rot[2]};
    target->scale={(std::max)(.001f,scale[0]),(std::max)(.001f,scale[1]),(std::max)(.001f,scale[2])};
    dirty_=true;
   }
  }
  if(hovered&&!io.WantTextInput){
   if(ImGui::IsKeyPressed(ImGuiKey_W,false))gizmo_=GizmoTool::Move;
   if(ImGui::IsKeyPressed(ImGuiKey_E,false))gizmo_=GizmoTool::Rotate;
   if(ImGui::IsKeyPressed(ImGuiKey_R,false))gizmo_=GizmoTool::Scale;
   if(ImGui::IsKeyPressed(ImGuiKey_Q,false))localSpace_=!localSpace_;
   if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)&&!ImGuizmo::IsOver()&&!ImGuizmo::IsUsing()){
    const ImVec2 m=io.MousePos;float nearest=900.f;EntityId found=0;
    for(const auto& e:scene_.entities())if(e.visible){float px=0,py=0;
     if(renderer_.project(e.position,camera_,px,py)){
      const float dx=px-(m.x-origin.x),dy=py-(m.y-origin.y),d=dx*dx+dy*dy;
      if(d<nearest){nearest=d;found=e.id;}
     }
    }
    selected_=found;
   }
  }

 }
 ImGui::GetWindowDrawList()->AddRectFilled({origin.x+9,origin.y+8},
  {origin.x+174,origin.y+30},IM_COL32(19,28,38,200),4);
 ImGui::SetCursorScreenPos({origin.x+18,origin.y+11});
 ImGui::TextColored(accent,"%s",playing_?"GAME VIEW":"SCENE VIEW");
 if(!playerOnly){
  ImGui::SetCursorScreenPos({origin.x+12,origin.y+size.y-25});
  ImGui::TextDisabled("W Move  /  E Rotate  /  R Scale   |   RMB Orbit   MMB Pan   Wheel Zoom");
 }else{
  if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)glfwSetWindowShouldClose(window,1);
 }
 ImGui::End();ImGui::PopStyleVar();
}
void Editor::openScript(const std::filesystem::path& relative){
 auto root=project_.root;
 auto file=root/relative;
 if(!childPath(root,file)||lower(file.extension().string())!=".lua"){
  pushMessage("Only Lua files inside this project can be opened");return;
 }
 if(scriptDirty_&&file!=openScriptPath_)saveScript();
 std::ifstream in(file,std::ios::binary);
 if(!in){pushMessage("Cannot read script: "+relative.string());return;}
 std::ostringstream buffer;buffer<<in.rdbuf();
 openScriptPath_=file;scriptEditor_.SetText(buffer.str());
 scriptDirty_=false;refreshLuaDiagnostics();
}
void Editor::refreshLuaDiagnostics(){
 if(openScriptPath_.empty()){scriptDiagnostics_.clear();scriptEditor_.SetErrorMarkers({});return;}
 scriptDiagnostics_=diagnoseLua(scriptEditor_.GetText());
 TextEditor::ErrorMarkers markers;
 for(const auto& diag:scriptDiagnostics_){
  std::string label=(diag.severity==DiagnosticSeverity::Error?"Error: ":"Warning: ")+diag.message;
  auto it=markers.find(diag.line);
  if(it==markers.end())markers[diag.line]=label;else it->second+="\n"+label;
 }
 scriptEditor_.SetErrorMarkers(markers);
 diagnosticsClock_=0.f;
}
void Editor::saveScript(){
 if(openScriptPath_.empty())return;
 if(!childPath(project_.root,openScriptPath_)){pushMessage("Unsafe script path");return;}
 std::ofstream out(openScriptPath_,std::ios::binary|std::ios::trunc);
 out<<scriptEditor_.GetText();
 if(out){scriptDirty_=false;refreshLuaDiagnostics();scanAssets();pushMessage("Saved Lua: "+openScriptPath_.filename().string());}
 else pushMessage("Failed to save Lua file");
}
void Editor::assetTree(const std::filesystem::path& folder){
 const auto path=project_.root/folder;
 const bool selected=folder==selectedAssetFolder_;
 const auto name=folder=="assets"?std::string("Assets"):folder.filename().string();
 ImGuiTreeNodeFlags nodeFlags=ImGuiTreeNodeFlags_OpenOnArrow|ImGuiTreeNodeFlags_SpanAvailWidth|
  (selected?ImGuiTreeNodeFlags_Selected:0);
 if(folder=="assets")nodeFlags|=ImGuiTreeNodeFlags_DefaultOpen;
 const bool opened=ImGui::TreeNodeEx(generic(folder).c_str(),nodeFlags,"%s",name.c_str());
 if(ImGui::IsItemClicked())selectedAssetFolder_=folder;
 if(opened){
  for(const auto& item:sortedEntries(path)){
   if(item.is_directory())assetTree(std::filesystem::relative(item.path(),project_.root));
  }
  ImGui::TreePop();
 }
}
void Editor::assetPanel(){
 ImGui::TextDisabled("PROJECT ASSETS");ImGui::SameLine();
 if(ImGui::SmallButton("Refresh"))scanAssets();ImGui::SameLine();
 if(ImGui::SmallButton("+ Folder")){
  const auto folder=project_.root/selectedAssetFolder_;int n=1;std::filesystem::path path;
  do{path=folder/("NewFolder"+std::to_string(n++));}while(std::filesystem::exists(path));
  std::filesystem::create_directories(path);
  selectedAssetFolder_=std::filesystem::relative(path,project_.root);scanAssets();
 }
 ImGui::SameLine();
 if(ImGui::SmallButton("+ Lua")){
  auto folder=project_.root/selectedAssetFolder_;
  std::filesystem::create_directories(folder);
  int n=1;std::filesystem::path path;
  do{path=folder/("script_"+std::to_string(n++)+".lua");}while(std::filesystem::exists(path));
  std::ofstream file(path);
  file<<"local Script = {}\n\nfunction Script.on_start(id)\n    Engine.log('Started')\nend\n\nfunction Script.on_update(id, dt)\nend\n\nreturn Script\n";
  file.close();scanAssets();openScript(std::filesystem::relative(path,project_.root));
 }
 ImGui::SetNextItemWidth(-1);ImGui::InputTextWithHint("##assetsearch","Search assets...",&assetSearch_);
 const float available=(std::max)(75.f,ImGui::GetContentRegionAvail().y);
 ImGui::BeginChild("##assetsTree",{(std::max)(110.f,ImGui::GetContentRegionAvail().x*.36f),available},true);
 assetTree("assets");ImGui::EndChild();ImGui::SameLine();
 ImGui::BeginChild("##assetFiles",{0,available},true);
 const auto chosenFolder=project_.root/selectedAssetFolder_;
 for(const auto& item:sortedEntries(chosenFolder)){
  if(item.is_directory())continue;
  const auto relative=std::filesystem::relative(item.path(),project_.root);
  const auto label=item.path().filename().string();
  if(!assetSearch_.empty()&&lower(label).find(lower(assetSearch_))==std::string::npos)continue;
  ImGui::PushID(generic(relative).c_str());
  ImGui::TextColored(item.path().extension()==".lua"?ImVec4(.41f,.75f,.92f,1):ImVec4(.64f,.7f,.73f,1),"%s",
    item.path().extension()==".lua"?"LUA":"FILE");ImGui::SameLine();
  if(ImGui::Selectable(label.c_str(),selectedAsset_==relative)){
   selectedAsset_=relative;
   if(item.path().extension()==".lua")openScript(relative);
   if(item.path().extension()==".obj"&&ImGui::IsMouseDoubleClicked(0)){
    addEntity(Shape::OBJ);if(auto* e=scene_.find(selected_)){e->mesh=generic(relative);e->name=item.path().stem().string();}
   }
  }
  if(ImGui::BeginDragDropSource()){
   const auto data=generic(relative);
   ImGui::SetDragDropPayload("AUV_ASSET",data.c_str(),data.size()+1);
   ImGui::Text("%s",label.c_str());ImGui::EndDragDropSource();
  }
  ImGui::PopID();
 }
 ImGui::EndChild();
}
void Editor::scriptPanel(){
 if(openScriptPath_.empty()){
  ImGui::TextDisabled("LUA EDITOR");ImGui::Separator();
  ImGui::TextWrapped("Select a .lua file in Assets, or attach one in the Inspector.");
  return;
 }
 ImGui::TextColored(accent,"LUA");ImGui::SameLine();
 ImGui::Text("%s%s",openScriptPath_.filename().string().c_str(),scriptDirty_?" *":"");
 ImGui::SameLine();if(ImGui::SmallButton("Save##Lua"))saveScript();
 ImGui::SameLine();if(ImGui::SmallButton("Check"))refreshLuaDiagnostics();
 int errors=0,warnings=0;
 for(const auto& diag:scriptDiagnostics_)if(diag.severity==DiagnosticSeverity::Error)++errors;else ++warnings;
 ImGui::SameLine();ImGui::TextColored(errors?ImVec4(.99f,.43f,.43f,1):ImVec4(.46f,.79f,.59f,1),
  "%d errors  %d warnings",errors,warnings);
 const float diagnosticsArea=scriptDiagnostics_.empty()?24.f:54.f;
 scriptEditor_.Render("##LuaSource",{0,(std::max)(40.f,ImGui::GetContentRegionAvail().y-diagnosticsArea)},true);
 if(scriptEditor_.IsTextChanged()){scriptDirty_=true;diagnosticsClock_=0.01f;}
 if(diagnosticsClock_>0.f){diagnosticsClock_+=ImGui::GetIO().DeltaTime;
  if(diagnosticsClock_>.45f)refreshLuaDiagnostics();
 }
 if(scriptDiagnostics_.empty())ImGui::TextColored({.46f,.81f,.6f,1},"Lua 5.4: no detected errors");
 else{
  for(std::size_t i=0;i<scriptDiagnostics_.size()&&i<2;++i){
   auto& diag=scriptDiagnostics_[i];
   ImGui::TextColored(diag.severity==DiagnosticSeverity::Error?ImVec4(.99f,.49f,.48f,1):ImVec4(.99f,.77f,.40f,1),
    "Line %d: %s",diag.line,diag.message.c_str());
  }
 }
}
void Editor::consolePanel(){
 ImGui::TextDisabled("CONSOLE");ImGui::SameLine();if(ImGui::SmallButton("Clear"))lua_.clearLog();
 ImGui::Separator();ImGui::BeginChild("##consoleLog",{0,0},false);
 for(const auto& line:lua_.logs()){
  const bool error=line.find("ERROR")!=std::string::npos||line.find("[Lua]")!=std::string::npos;
  ImGui::PushStyleColor(ImGuiCol_Text,error?ImVec4(.98f,.52f,.49f,1):ImVec4(.68f,.75f,.78f,1));
  ImGui::TextWrapped("%s",line.c_str());ImGui::PopStyleColor();
 }
 if(ImGui::GetScrollY()>=(ImGui::GetScrollMaxY()-25.f))ImGui::SetScrollHereY(1.f);
 ImGui::EndChild();
}
void Editor::bottomBar(){
 const auto& io=ImGui::GetIO();
 const float w=io.DisplaySize.x;
 const float a=w*.30f,b=w*.43f;
 const float h=(std::max)(150.f,(std::min)(bottomHeight_,io.DisplaySize.y*.46f));
 beginFixed("Project / Assets",{0,io.DisplaySize.y-h},{a,h});assetPanel();ImGui::End();
 beginFixed("Lua Script Editor",{a,io.DisplaySize.y-h},{b,h});scriptPanel();ImGui::End();
 beginFixed("Console",{a+b,io.DisplaySize.y-h},{(std::max)(120.f,w-a-b),h});consolePanel();ImGui::End();
}
void Editor::projectPanel(){
 if(!showProject_)return;
 ImGui::OpenPopup("Project Manager");
 if(ImGui::BeginPopupModal("Project Manager",&showProject_,ImGuiWindowFlags_AlwaysAutoResize)){
  ImGui::TextColored(accent,"CURRENT PROJECT");
  ImGui::Text("%s",project_.name.c_str());
  ImGui::TextWrapped("%s",project_.root.string().c_str());ImGui::Separator();
  ImGui::TextDisabled("CREATE NEW PROJECT");
  ImGui::InputText("Parent directory",projectPath_,sizeof(projectPath_));
  ImGui::InputText("Project name",projectName_,sizeof(projectName_));
  if(ImGui::Button("Create and open"))createNewProject();
  ImGui::Separator();ImGui::TextDisabled("OPEN AN EXISTING PROJECT");
  ImGui::InputText("Folder or manifest",existingPath_,sizeof(existingPath_));
  if(ImGui::Button("Open project")){
   Project candidate;Scene loaded;
   if(!candidate.load(existingPath_))pushMessage("Cannot load project.auvproj");
   else if(!loaded.load(candidate.scenePath()))pushMessage("Scene load failed: "+loaded.error());
   else{
    if(playing_)stop();
    if(scriptDirty_)saveScript();
    project_=std::move(candidate);scene_=std::move(loaded);selected_=0;
    dirty_=false;openScriptPath_.clear();scriptDirty_=false;
    selectedAssetFolder_="assets";scanAssets();showProject_=false;
    std::snprintf(gameName_,sizeof(gameName_),"%s",safeFileName(project_.name).c_str());
    pushMessage("Opened: "+project_.name);
   }
  }
  ImGui::SameLine();if(ImGui::Button("Close"))showProject_=false;
  ImGui::EndPopup();
 }
}
void Editor::buildGame(){
 if(playing_){pushMessage("Stop Play Mode before building");return;}
 const auto outputParent=std::filesystem::path(buildPath_);
 const std::string game=safeFileName(gameName_);
 if(!std::filesystem::exists(project_.root/"project.auvproj")){pushMessage("Project manifest not found");return;}
 if(childPath(project_.root,outputParent)){pushMessage("Output directory must be outside the project");return;}
 if(!std::filesystem::exists(outputParent)){
  std::error_code ec;std::filesystem::create_directories(outputParent,ec);
  if(ec){pushMessage("Cannot create output directory: "+ec.message());return;}
 }
 // Refresh asset list and validate all resources before packaging.
 scanAssets();
 if(scriptDirty_)saveScript();
 if(scene_.skybox.enabled&&scene_.skybox.useCubemap){
  for(const auto& face:scene_.skybox.faces){
   const auto absolute=project_.root/face;
   if(face.empty()||!childPath(project_.root,absolute)||
      !std::filesystem::is_regular_file(absolute)){
    pushMessage("Build failed: missing or unsafe skybox face: "+face);return;
   }
   auto relative=std::filesystem::path(face).lexically_normal();
   if(relative.empty()||relative.begin()->string()!="assets"){
    pushMessage("Build failed: skybox textures must be inside assets/: "+face);return;
   }
  }
 }
 // A bad Lua file must not silently become part of a release.
 for(const auto& asset:assets_)if(lower(asset.extension().string())==".lua"){
  std::ifstream in(project_.root/asset,std::ios::binary);
  if(!in){pushMessage("Build failed: cannot read "+generic(asset));return;}
  std::ostringstream content;content<<in.rdbuf();
  for(const auto& diagnostic:diagnoseLua(content.str())){
   if(diagnostic.severity==DiagnosticSeverity::Error){
    pushMessage("Build failed: "+generic(asset)+" line "+std::to_string(diagnostic.line)+": "+diagnostic.message);
    openScript(asset);return;
   }
  }
 }
 save();
 if(dirty_){pushMessage("Build cancelled: scene could not be saved");return;}
 std::error_code ec;
 #ifdef _WIN32
 const auto destination=outputParent/(game+"_Windows");
#else
 const auto destination=outputParent/(game+"_Linux");
#endif
 if(childPath(project_.root,destination)){pushMessage("Output cannot be inside the project");return;}
 std::filesystem::create_directories(destination,ec);
 if(ec){pushMessage("Build directory error: "+ec.message());return;}
 std::vector<std::filesystem::path> packageFiles;
 packageFiles.push_back("project.auvproj");packageFiles.push_back(project_.scene);
 packageFiles.insert(packageFiles.end(),assets_.begin(),assets_.end());
 std::string packageError;
 const auto archive=destination/(game+".auvyronasset");
 if(!AssetPackage::create(archive,game,project_.root,packageFiles,packageError)){
  pushMessage("Asset packing failed: "+packageError);return;
 }
 std::filesystem::path executable;
#ifdef _WIN32
 wchar_t module[32768]{};
 if(GetModuleFileNameW(nullptr,module,32768)>0)
  executable=std::filesystem::path(module).parent_path()/"AuvyronPlayer.exe";
#elif defined(__linux__)
 executable=std::filesystem::read_symlink("/proc/self/exe",ec).parent_path()/"AuvyronPlayer";
#elif defined(__APPLE__)
 executable=std::filesystem::current_path()/"AuvyronPlayer";
#endif
 if(executable.empty()||!std::filesystem::is_regular_file(executable)){
  std::filesystem::remove(archive,ec);
  pushMessage("Build failed: missing standalone AuvyronPlayer. Rebuild both targets.");return;
 }
#ifdef _WIN32
 const auto gameExe=destination/(game+".exe");
#else
 const auto gameExe=destination/game;
#endif
 std::filesystem::copy_file(executable,gameExe,std::filesystem::copy_options::overwrite_existing,ec);
 if(ec){std::filesystem::remove(archive,ec);pushMessage("Failed to copy player executable");return;}
 // Remove obsolete plaintext data from earlier alpha builds in this output folder.
 // Only the old, known export directory is deleted; other user files are untouched.
 ec.clear();std::filesystem::remove_all(destination/"Data",ec);
 if(ec){pushMessage("Warning: remove legacy Data/ manually; "+ec.message());return;}
 std::filesystem::remove(destination/"Play_Game.bat",ec);
 std::filesystem::remove(destination/"BUILD_INFO.txt",ec);
 pushMessage("Build successful: "+gameExe.string()+" + "+archive.filename().string());
 showBuild_=false;
}
void Editor::buildPanel(){

 if(!showBuild_)return;
 ImGui::OpenPopup("Build Game");
 if(ImGui::BeginPopupModal("Build Game",&showBuild_,ImGuiWindowFlags_AlwaysAutoResize)){
  ImGui::TextColored(accent,"AUVYRON  /  GAME BUILD");ImGui::Spacing();
  ImGui::TextDisabled("Target platform");ImGui::SameLine();
#ifdef _WIN32
  ImGui::TextUnformatted("Windows x64 (native)");
#else
  ImGui::TextUnformatted("Current platform (native)");
#endif
  ImGui::SetNextItemWidth(360);ImGui::InputText("Game executable",gameName_,sizeof(gameName_));
  ImGui::SetNextItemWidth(360);ImGui::InputText("Output parent folder",buildPath_,sizeof(buildPath_));
  ImGui::TextWrapped("Build Game produces <Game>.exe plus <Game>.auvyronasset."
                     " Lua, models, textures and scenes are packed (checksum + light obfuscation).");
  ImGui::Separator();
  if(ImGui::Button("Build Game",{140,34}))buildGame();
  ImGui::SameLine();if(ImGui::Button("Cancel",{110,34}))showBuild_=false;
  #ifdef _WIN32
  ImGui::TextDisabled("Output: <folder>/<name>_Windows/  (EXE + package)");
#else
  ImGui::TextDisabled("Output: <folder>/<name>_Linux/  (EXE + package)");
#endif
  ImGui::EndPopup();
 }
}
void Editor::draw(GLFWwindow* window,float dt,bool playerOnly){
 activeWindow_=window;
 // Must be ready before on_start, which can request mouse capture.
 lua_.setMouseCaptureCallback([this,window](bool enabled){mouse_.capture(window,enabled);});
 lua_.setMouseVisibleCallback([this,window](bool visible){mouse_.capture(window,!visible);});
 lua_.setMouseCapturedCallback([this](){return mouse_.captured();});
 lua_.setMouseButtonCallback([window](const std::string& button){
  if(button=="LEFT")return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
  if(button=="RIGHT")return glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
  return false;
 });
 const auto& io=ImGui::GetIO();
 if(!playerOnly&&io.KeyCtrl&&ImGui::IsKeyPressed(ImGuiKey_S,false))save();
 if(!playerOnly&&io.KeyCtrl&&ImGui::IsKeyPressed(ImGuiKey_L,false))saveScript();
 if(!playerOnly&&io.KeyCtrl&&ImGui::IsKeyPressed(ImGuiKey_B,false))showBuild_=true;
 if(playerOnly&&!playing_)start();
 // Responsive widths: the scene always retains useful space on smaller screens.
 if(!playerOnly){
  leftWidth_=(std::min)(270.f,(std::max)(165.f,io.DisplaySize.x*.19f));
  rightWidth_=(std::min)(335.f,(std::max)(185.f,io.DisplaySize.x*.23f));
  bottomHeight_=(std::min)(340.f,(std::max)(220.f,io.DisplaySize.y*.36f));
 }
 topBar(playerOnly);
 if(!playerOnly){hierarchy();inspector();bottomBar();projectPanel();buildPanel();}
 sceneViewport(window,dt,playerOnly);
}
}
