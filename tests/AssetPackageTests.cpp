/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    tests/AssetPackageTests.cpp
 * @license MIT
 */

#include "core/AssetPackage.hpp"
#include "core/AssetStore.hpp"
#include "core/Project.hpp"
#include "core/Scene.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

// Unlike assert(), TEST_REQUIRE executes its expression in Release builds.
// CMake/CTest build tests with -DNDEBUG / /DNDEBUG for Release.
#define TEST_REQUIRE(expr) do { \
    if (!(expr)) { \
        std::cerr << "[packed_assets] FAILED: " << #expr \
                  << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        return EXIT_FAILURE; \
    } \
} while (false)
int main(){
 namespace fs=std::filesystem;
 const fs::path root=fs::temp_directory_path()/"auvyron_pack_test_project";
 fs::remove_all(root);
 auv::Project project;TEST_REQUIRE(project.create(root,"Pack Test"));
 auv::Scene scene;scene.create("Cube").kind=auv::ComponentKind::PointLight;
 TEST_REQUIRE(scene.save(project.scenePath()));
 fs::create_directories(root/"assets"/"models");
 const std::string script="local S={}\nfunction S.on_start(id) Engine.log('Test') end\nreturn S\n";
 std::ofstream(root/"assets/scripts/sample.lua",std::ios::binary)<<script;
 std::ofstream(root/"assets/models/sample.obj",std::ios::binary)<<"v 0 0 0\n";
 std::vector<fs::path> entries={"project.auvproj","scenes/main.auvscene",
                                "assets/scripts/sample.lua","assets/models/sample.obj"};
 std::string error;const auto packageFile=root.parent_path()/"auvyron_pack_test.auvyronasset";
 TEST_REQUIRE(auv::AssetPackage::create(packageFile,"MyTestGame",root,entries,error));
 auto loaded=std::make_shared<auv::AssetPackage>();TEST_REQUIRE(loaded->open(packageFile,error));
 TEST_REQUIRE(loaded->gameName()=="MyTestGame");TEST_REQUIRE(loaded->fileCount()==4);
 auv::AssetStore store(root);store.usePackage(loaded);
 std::string data;TEST_REQUIRE(store.readText("assets/scripts/sample.lua",data));TEST_REQUIRE(data==script);
 TEST_REQUIRE(store.readText("project.auvproj",data));
 auv::Project reopened;TEST_REQUIRE(reopened.loadText(data,root));TEST_REQUIRE(reopened.name=="Pack Test");
 TEST_REQUIRE(store.readText(reopened.scene,data));
 auv::Scene restored;TEST_REQUIRE(restored.loadText(data));
 TEST_REQUIRE(restored.entities().size()==1);
 TEST_REQUIRE(restored.entities()[0].kind==auv::ComponentKind::PointLight);
 TEST_REQUIRE(!store.readText("../assets/scripts/sample.lua",data));
 TEST_REQUIRE(!loaded->contains("../../Windows/system32"));
 TEST_REQUIRE(!auv::safeAssetPath("../escape"));
 {std::fstream io(packageFile,std::ios::binary|std::ios::in|std::ios::out);
  TEST_REQUIRE(io.is_open());
  io.seekg(-1,std::ios::end);char last=0;io.get(last);
  TEST_REQUIRE(bool(io));
  last^=0x40;io.seekp(-1,std::ios::end);io.put(last);
  TEST_REQUIRE(bool(io));}
 auv::AssetPackage corrupt;std::string bad;
 TEST_REQUIRE(!corrupt.open(packageFile,bad));TEST_REQUIRE(bad.find("Corrupt")!=std::string::npos);
 fs::remove_all(root);fs::remove(packageFile);
 std::cout<<"Auvyron asset package, integrity and backward-compatible scene tests passed\n";
 return EXIT_SUCCESS;
}
