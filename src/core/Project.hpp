/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/Project.hpp
 * @license MIT
 */

#pragma once
#include <filesystem>
#include <string>
namespace auv {
struct Project {
 std::string name="Untitled";
 std::filesystem::path root;
 std::filesystem::path scene="scenes/main.auvscene";
 bool load(const std::filesystem::path& path);
 bool loadText(const std::string& content,const std::filesystem::path& virtualRoot);
 bool save()const;
 bool create(const std::filesystem::path& directory,std::string projectName);
 std::filesystem::path scenePath()const{return root/scene;}
 std::filesystem::path scriptsPath()const{return root/"assets"/"scripts";}
};
}
