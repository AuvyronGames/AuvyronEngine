/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/Editor.hpp
 * @license MIT
 */

#pragma once
#include "core/Scene.hpp"
#include "core/Project.hpp"
#include "core/LuaRuntime.hpp"
#include "core/LuaDiagnostics.hpp"
#include "core/AssetPackage.hpp"
#include "render/Renderer.hpp"
#include "app/MouseCapture.hpp"
#include <TextEditor.h>
#include <filesystem>
#include <string>
#include <vector>
struct GLFWwindow;
namespace auv {
class Editor {
public:
 bool init(const std::filesystem::path& folder);
 void draw(GLFWwindow* window,float dt,bool playerOnly);
 void shutdown();
 const std::string& title()const{return project_.name;}
 bool play()const{return playing_;}
private:
 enum class GizmoTool { Move, Rotate, Scale };
 void topBar(bool playerOnly);
 void hierarchy();
 void inspector();
 void sceneSettings();
 void sceneViewport(GLFWwindow* window,float dt,bool playerOnly);
 void bottomBar();
 void assetPanel();
 void assetTree(const std::filesystem::path& folder);
 void scriptPanel();
 void refreshLuaDiagnostics();
 void consolePanel();
 void projectPanel();
 void buildPanel();
 void save();
 void buildGame();
 void addEntity(Shape shape);
 void addComponent(ComponentKind kind);
 void applyTheme();
 void createNewProject();
 void openScript(const std::filesystem::path& relative);
 void saveScript();
 void scanAssets();
 void start();
 void stop();
 void pushMessage(std::string m);
 Project project_;
 Scene scene_,snapshot_;
 LuaRuntime lua_;
 Renderer renderer_;
 Camera camera_,editorCameraSnapshot_;
 MouseCapture mouse_;
 GLFWwindow* activeWindow_=nullptr;
 bool escapePreviouslyDown_=false;
 EntityId selected_=0;
 bool playing_=false,dirty_=false,showProject_=false,showBuild_=false,showSceneSettings_=true;
 bool showCredits_=false,showSettings_=false,lightTheme_=false;
 bool scriptDirty_=false,snap_=false,localSpace_=false;
 float moveSnap_=0.5f,rotateSnap_=15.f,scaleSnap_=0.25f;
 GizmoTool gizmo_=GizmoTool::Move;
 std::string assetSearch_,message_="Ready";
 std::vector<std::filesystem::path> assets_;
 std::filesystem::path selectedAssetFolder_="assets",selectedAsset_,openScriptPath_;
 TextEditor scriptEditor_;
 std::vector<LuaDiagnostic> scriptDiagnostics_;
 float diagnosticsClock_=0;
 char projectPath_[512]{},existingPath_[512]{},projectName_[128]="NewGame";
 char buildPath_[512]{},gameName_[128]{};
 float toolbarHeight_=92,bottomHeight_=304,leftWidth_=248,rightWidth_=318;
};
}
