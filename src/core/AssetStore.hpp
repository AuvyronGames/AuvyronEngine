/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/AssetStore.hpp
 * @license MIT
 */

#pragma once
#include "core/AssetPackage.hpp"
#include <filesystem>
#include <memory>
#include <string>
namespace auv {
class AssetStore {
public:
 explicit AssetStore(std::filesystem::path root={}):root_(std::move(root)){}
 void usePackage(std::shared_ptr<AssetPackage> pack){package_=std::move(pack);}
 bool read(const std::filesystem::path& relative,AssetPackage::Bytes& out)const;
 bool readText(const std::filesystem::path& relative,std::string& out)const;
 const std::filesystem::path& root()const{return root_;}
 bool packed()const{return bool(package_);}
private:
 std::filesystem::path root_;
 std::shared_ptr<AssetPackage> package_;
};
}
