/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/AssetStore.cpp
 * @license MIT
 */

#include "core/AssetStore.hpp"
#include <fstream>
namespace auv {
bool AssetStore::read(const std::filesystem::path& rel,AssetPackage::Bytes& out)const{
 if(!safeAssetPath(rel))return false;
 if(package_)return package_->read(rel,out);
 std::error_code ec;
 const auto base=std::filesystem::weakly_canonical(root_,ec);if(ec)return false;
 const auto full=std::filesystem::weakly_canonical(base/rel,ec);if(ec)return false;
 auto a=base.begin(),b=full.begin();for(;a!=base.end();++a,++b)if(b==full.end()||*a!=*b)return false;
 const auto size=std::filesystem::file_size(full,ec);if(ec||size>256ull*1024*1024)return false;
 std::ifstream in(full,std::ios::binary);if(!in)return false;
 out.resize(static_cast<std::size_t>(size));return bool(in.read(reinterpret_cast<char*>(out.data()),static_cast<std::streamsize>(size)));
}
bool AssetStore::readText(const std::filesystem::path& rel,std::string& out)const{
 AssetPackage::Bytes bytes;if(!read(rel,bytes))return false;
 out.assign(reinterpret_cast<const char*>(bytes.data()),bytes.size());return true;
}
}
