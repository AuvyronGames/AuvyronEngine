/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/AssetPackage.cpp
 * @license MIT
 */

#include "core/AssetPackage.hpp"
#include <algorithm>
#include <fstream>
#include <limits>
#include <set>
#include <system_error>
namespace auv {
namespace {
constexpr std::uint64_t maxFile=256ull*1024*1024;
constexpr std::uint64_t maxBundle=1024ull*1024*1024;
constexpr std::uint32_t maxEntries=40000;
std::uint64_t hashText(std::string_view s){
 std::uint64_t h=14695981039346656037ull;
 for(unsigned char c:s){h^=c;h*=1099511628211ull;}return h;
}
std::uint64_t streamSeed(std::string_view game,std::string_view relative){
 return hashText(std::string(game)+"|AuvyronEngine|"+std::string(relative))^0xACF9317D6EB4A0F5ull;
}
void scramble(AssetPackage::Bytes& data,std::uint64_t seed){
 // xorshift64* keystream; reversible obfuscation only, not secure encryption.
 if(seed==0)seed=1;
 for(auto& ch:data){seed^=seed>>12;seed^=seed<<25;seed^=seed>>27;
  ch^=static_cast<std::uint8_t>((seed*2685821657736338717ull)>>56);}
}
template<typename T> void writeInt(std::ofstream& out,T value){
 for(std::size_t i=0;i<sizeof(T);++i)out.put(char((std::uint64_t(value)>>(i*8))&255));
}
template<typename T> bool readInt(std::ifstream& in,T& value){
 std::uint64_t result=0;for(std::size_t i=0;i<sizeof(T);++i){int c=in.get();if(c==EOF)return false;
 result|=std::uint64_t(std::uint8_t(c))<<(i*8);}value=static_cast<T>(result);return true;
}
bool isChild(const std::filesystem::path& parent,const std::filesystem::path& child){
 auto a=parent.begin(),b=child.begin();for(;a!=parent.end();++a,++b)
  if(b==child.end()||*a!=*b)return false;
 return true;
}
}
bool safeAssetPath(const std::filesystem::path& p){
 if(p.empty()||p.is_absolute()||p.has_root_path())return false;
 auto norm=p.lexically_normal();if(norm.empty()||norm==".")return false;
 for(const auto& part:norm)if(part==".."||part==".")return false;
 return norm==p && p.generic_string().find('\\')==std::string::npos;
}
std::uint64_t AssetPackage::checksum(const Bytes& d){
 std::uint64_t h=14695981039346656037ull;
 for(auto c:d){h^=c;h*=1099511628211ull;}return h;
}
bool AssetPackage::create(const std::filesystem::path& file,const std::string& game,
 const std::filesystem::path& sourceRoot,const std::vector<std::filesystem::path>& files,std::string& error){
 if(game.empty()||game.size()>512||files.size()>maxEntries){error="Invalid game name or file count";return false;}
 std::error_code ec;auto base=std::filesystem::weakly_canonical(sourceRoot,ec);
 if(ec){error="Invalid source root";return false;}
 auto temp=file;temp+=".tmp";
 std::filesystem::create_directories(file.parent_path(),ec);
 if(ec){error=ec.message();return false;}
 std::ofstream out(temp,std::ios::binary|std::ios::trunc);
 if(!out){error="Could not create asset package";return false;}
 out.write("AUVPAK1\0",8);writeInt<std::uint16_t>(out,std::uint16_t(game.size()));
 out.write(game.data(),static_cast<std::streamsize>(game.size()));
 writeInt<std::uint32_t>(out,std::uint32_t(files.size()));
 std::set<std::string> names;std::uint64_t total=0;
 for(const auto& rel:files){
  if(!safeAssetPath(rel)){error="Unsafe asset path: "+rel.string();break;}
  auto name=rel.generic_string();if(name.size()>4096||!names.insert(name).second){error="Duplicate or long path: "+name;break;}
  auto path=std::filesystem::weakly_canonical(base/rel,ec);
  if(ec||!isChild(base,path)||!std::filesystem::is_regular_file(path)){error="Asset escapes project or is missing: "+name;break;}
  auto sz=std::filesystem::file_size(path,ec);
  if(ec||sz>maxFile||total+sz>maxBundle){error="Asset exceeds package size limit: "+name;break;}
  total+=sz;
  Bytes data(static_cast<std::size_t>(sz));std::ifstream in(path,std::ios::binary);
  if(!in||!in.read(reinterpret_cast<char*>(data.data()),static_cast<std::streamsize>(sz))){error="Cannot read: "+name;break;}
  auto digest=checksum(data);scramble(data,streamSeed(game,name));
  writeInt<std::uint16_t>(out,std::uint16_t(name.size()));out.write(name.data(),static_cast<std::streamsize>(name.size()));
  writeInt<std::uint64_t>(out,sz);writeInt<std::uint64_t>(out,digest);
  out.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));
  if(!out){error="Failed to write package: "+name;break;}
 }
 out.close();if(!error.empty()){std::filesystem::remove(temp,ec);return false;}
 std::filesystem::rename(temp,file,ec);
 if(ec){std::filesystem::remove(file,ec);ec.clear();std::filesystem::rename(temp,file,ec);}
 if(ec){error="Could not finalize package: "+ec.message();return false;}
 return true;
}
bool AssetPackage::open(const std::filesystem::path& file,std::string& error){
 files_.clear();game_.clear();std::ifstream in(file,std::ios::binary);
 if(!in){error="Package not found: "+file.string();return false;}
 char magic[8]{};in.read(magic,8);
 if(!in||std::string_view(magic,8)!=std::string_view("AUVPAK1\0",8)){error="Unsupported package header";return false;}
 std::uint16_t length=0;std::uint32_t count=0;
 if(!readInt(in,length)||!length||length>512){error="Invalid game name length";return false;}
 game_.resize(length);in.read(game_.data(),length);
 if(!in||!readInt(in,count)||count>maxEntries){error="Invalid package entry count";return false;}
 std::uint64_t total=0;
 for(std::uint32_t i=0;i<count;++i){
  std::uint16_t n=0;std::uint64_t size=0,check=0;
  if(!readInt(in,n)||n==0||n>4096){error="Invalid path length";return false;}
  std::string name(n,'\0');in.read(name.data(),n);
  if(!in||!safeAssetPath(std::filesystem::path(name))||!readInt(in,size)||!readInt(in,check)||size>maxFile||total+size>maxBundle){error="Invalid package entry";return false;}
  total+=size;Bytes data(static_cast<std::size_t>(size));
  in.read(reinterpret_cast<char*>(data.data()),static_cast<std::streamsize>(size));
  if(!in){error="Truncated package: "+name;return false;}
  scramble(data,streamSeed(game_,name));
  if(checksum(data)!=check){error="Corrupt or modified asset: "+name;return false;}
  if(!files_.emplace(name,std::move(data)).second){error="Duplicate package path";return false;}
 }
 if(in.peek()!=EOF){error="Unexpected trailing package data";return false;}
 return true;
}
bool AssetPackage::read(const std::filesystem::path& relative,Bytes& out)const{
 if(!safeAssetPath(relative))return false;
 auto it=files_.find(relative.generic_string());if(it==files_.end())return false;
 out=it->second;return true;
}
bool AssetPackage::contains(const std::filesystem::path& relative)const{
 return safeAssetPath(relative)&&files_.find(relative.generic_string())!=files_.end();
}
}
