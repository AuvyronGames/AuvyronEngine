# @project AuvyronEngine
# @author  D4NTE / Auvyron Games
# @file    CMakeLists.txt
# @license MIT

cmake_minimum_required(VERSION 3.21)
project(AuvyronEngine VERSION 0.4.2 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(WIN32)
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
include(FetchContent)

set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw)
FetchContent_Declare(imgui URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz)
FetchContent_MakeAvailable(imgui)
FetchContent_Declare(lua URL https://www.lua.org/ftp/lua-5.4.9.tar.gz
 URL_HASH SHA256=2335b6c582a52654f94612bf10d2f4672805d05329aa6568b1d8cd9e5c6fb8e6)
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
  FetchContent_Populate(lua)
endif()
set(LUA_FILES lapi lauxlib lbaselib lcode lcorolib lctype ldblib ldebug ldo ldump lfunc lgc linit liolib llex lmathlib lmem loadlib lobject lopcodes loslib lparser lstate lstring lstrlib ltable ltablib ltm lundump lutf8lib lvm lzio)
set(LUA_SOURCES "")
foreach(f ${LUA_FILES})
  list(APPEND LUA_SOURCES ${lua_SOURCE_DIR}/src/${f}.c)
endforeach()
add_library(auvyron_lua STATIC ${LUA_SOURCES})
target_include_directories(auvyron_lua PUBLIC ${lua_SOURCE_DIR}/src)
if(UNIX AND NOT APPLE)
  target_link_libraries(auvyron_lua PUBLIC m ${CMAKE_DL_LIBS})
  target_compile_definitions(auvyron_lua PRIVATE LUA_USE_LINUX)
elseif(APPLE)
  target_compile_definitions(auvyron_lua PRIVATE LUA_USE_MACOSX)
endif()

add_library(auvyron_core STATIC
  src/core/Scene.cpp src/core/LuaRuntime.cpp src/core/Project.cpp src/core/LuaDiagnostics.cpp
  src/core/AssetPackage.cpp src/core/AssetStore.cpp
)
target_include_directories(auvyron_core PUBLIC src)
target_link_libraries(auvyron_core PUBLIC auvyron_lua)

# stb_image is used only to decode user-supplied skybox PNG/JPG faces.
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master GIT_SHALLOW TRUE)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
  FetchContent_Populate(stb)
endif()

add_library(auvyron_render STATIC src/render/GL.cpp src/render/Renderer.cpp)
target_include_directories(auvyron_render PRIVATE ${stb_SOURCE_DIR})
target_include_directories(auvyron_render PUBLIC src)
target_link_libraries(auvyron_render PUBLIC glfw)
if(WIN32)
 target_link_libraries(auvyron_render PUBLIC opengl32)
else()
 find_package(OpenGL REQUIRED)
 target_link_libraries(auvyron_render PUBLIC OpenGL::GL)
endif()

add_library(auvyron_imgui STATIC
 ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
 ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
 ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
 ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
 ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(auvyron_imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${imgui_SOURCE_DIR}/misc/cpp)
target_link_libraries(auvyron_imgui PUBLIC glfw)
if(WIN32)
 target_link_libraries(auvyron_imgui PUBLIC opengl32)
else()
 target_link_libraries(auvyron_imgui PUBLIC OpenGL::GL)
endif()

# Editor-only tools: actual 3D transform handles and a Lua-aware code editor.
# Their repositories are fetched during CMake configure (internet required on first build).
FetchContent_Declare(imguizmo GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git GIT_TAG master GIT_SHALLOW TRUE)
FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
  FetchContent_Populate(imguizmo)
endif()
add_library(auvyron_gizmos STATIC ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp)
target_include_directories(auvyron_gizmos PUBLIC ${imguizmo_SOURCE_DIR}/src)
target_link_libraries(auvyron_gizmos PUBLIC auvyron_imgui)

FetchContent_Declare(imguicolortextedit GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit.git GIT_TAG master GIT_SHALLOW TRUE)
FetchContent_GetProperties(imguicolortextedit)
if(NOT imguicolortextedit_POPULATED)
  FetchContent_Populate(imguicolortextedit)
endif()
# Compatibility for obsolete Dear ImGui keyboard and tab-stop functions.
file(READ "${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp" _auvyron_text_editor_source)
string(REPLACE "ImGui::GetKeyIndex(" "(" _auvyron_text_editor_source "${_auvyron_text_editor_source}")
string(REPLACE "ImGui::PushAllowKeyboardFocus(" "ImGui::PushTabStop(" _auvyron_text_editor_source "${_auvyron_text_editor_source}")
string(REPLACE "ImGui::PopAllowKeyboardFocus()" "ImGui::PopTabStop()" _auvyron_text_editor_source "${_auvyron_text_editor_source}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/AuvyronLuaTextEditor.cpp" "${_auvyron_text_editor_source}")
add_library(auvyron_texteditor STATIC "${CMAKE_CURRENT_BINARY_DIR}/AuvyronLuaTextEditor.cpp")
target_include_directories(auvyron_texteditor PUBLIC ${imguicolortextedit_SOURCE_DIR})
target_link_libraries(auvyron_texteditor PUBLIC auvyron_imgui)

# Dedicated standalone game player. No Dear ImGui, no editor, no gizmos.
add_executable(AuvyronPlayer WIN32 src/app/Player.cpp)
target_include_directories(AuvyronPlayer PRIVATE src)
target_link_libraries(AuvyronPlayer PRIVATE auvyron_core auvyron_render)

add_executable(AuvyronEngine src/app/main.cpp src/app/Application.cpp src/app/Editor.cpp)
target_include_directories(AuvyronEngine PRIVATE src)
target_link_libraries(AuvyronEngine PRIVATE auvyron_core auvyron_render auvyron_imgui auvyron_gizmos auvyron_texteditor)
if(MSVC)
 target_compile_options(AuvyronEngine PRIVATE /W4 /utf-8)
 target_compile_options(AuvyronPlayer PRIVATE /W4 /utf-8)
else()
 target_compile_options(AuvyronEngine PRIVATE -Wall -Wextra -Wpedantic)
 target_compile_options(AuvyronPlayer PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_custom_command(TARGET AuvyronEngine POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/examples/StarterWorld
 $<TARGET_FILE_DIR:AuvyronEngine>/StarterWorld)
add_dependencies(AuvyronEngine AuvyronPlayer)

include(CTest)
if(BUILD_TESTING)
 add_executable(auvyron_tests tests/CoreTests.cpp)
 target_link_libraries(auvyron_tests PRIVATE auvyron_core)
 add_test(NAME core_and_lua COMMAND auvyron_tests)
 add_executable(auvyron_package_tests tests/AssetPackageTests.cpp)
 target_link_libraries(auvyron_package_tests PRIVATE auvyron_core)
 add_test(NAME packed_assets COMMAND auvyron_package_tests)
endif()
