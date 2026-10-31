/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/AssetPackage.hpp
 * @license MIT
 */

#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace auv {
// AUVPAK1 is a portable asset container with checksums and reversible obfuscation.
// Deliberately NOT cryptographic protection: the game name is publicly recoverable.
class AssetPackage {
public:
 using Bytes=std::vector<std::uint8_t>;
 static bool create(const std::filesystem::path& file,const std::string& game,
                    const std::filesystem::path& sourceRoot,
                    const std::vector<std::filesystem::path>& files,std::string& error);
 bool open(const std::filesystem::path& file,std::string& error);
 bool read(const std::filesystem::path& relative,Bytes& out)const;
 bool contains(const std::filesystem::path& relative)const;
 const std::string& gameName()const{return game_;}
 std::size_t fileCount()const{return files_.size();}
 static std::uint64_t checksum(const Bytes& data);
private:
 std::string game_;
 std::unordered_map<std::string,Bytes> files_;
};
// Uniform path validation for both packing and loading (no traversal, symlink escape).
bool safeAssetPath(const std::filesystem::path& relative);
}
