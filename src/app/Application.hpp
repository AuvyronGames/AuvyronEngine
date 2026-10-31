/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/Application.hpp
 * @license MIT
 */

#pragma once
#include "app/Editor.hpp"
#include <filesystem>
struct GLFWwindow;
namespace auv {
class Application {
public:
 bool init(const std::filesystem::path& project,bool playerOnly);
 int run();
 void shutdown();
private:
 GLFWwindow* window_=nullptr;
 Editor editor_;
 bool imgui_=false,playerOnly_=false;
};
}
