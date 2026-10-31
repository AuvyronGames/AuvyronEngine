/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/LuaDiagnostics.cpp
 * @license MIT
 */

#include "core/LuaDiagnostics.hpp"
#include <lua.hpp>
#include <algorithm>
#include <cctype>
#include <set>
#include <regex>
#include <string>
#include <vector>
namespace auv {
namespace {
const std::set<std::string> validFunctions={
 "log","get_position","set_position","translate","rotate","set_rotation",
 "get_rotation","set_scale","set_color","set_visible","find_entity",
 "spawn","destroy","key_down","time",
 "set_camera_target","set_camera_angles","set_camera_distance","get_camera_target",
 "mouse_delta","capture_mouse","mouse_captured","set_fps_camera",
 "set_velocity","get_velocity","is_grounded","set_gravity","set_light","spawn_fire",
 "spawn_explosion","raycast","ui_text","ui_rect","ui_button","ui_mouse_visible",
 "set_paused","is_paused","quit_game","mouse_down","get_camera_angles"
};
std::string maskNonCode(std::string_view source){
 std::string clean(source);
 enum State { Code, Single, Double, LineComment, LongString, LongComment } state=Code;
 auto blank=[&](size_t i){if(clean[i]!='\n')clean[i]=' ';};
 for(size_t i=0;i<source.size();++i){
  char c=source[i],next=i+1<source.size()?source[i+1]:'\0';
  if(state==Code){
   if(c=='-'&&next=='-'){
    blank(i);blank(++i);
    if(i+1<source.size()&&source[i+1]=='['&&i+2<source.size()&&source[i+2]=='['){
     blank(++i);blank(++i);state=LongComment;
    }else state=LineComment;
   }else if(c=='\''||c=='"'){blank(i);state=c=='\''?Single:Double;}
   else if(c=='['&&next=='['){blank(i);blank(++i);state=LongString;}
  }else if(state==LineComment){blank(i);if(c=='\n')state=Code;}
  else if(state==Single||state==Double){
   if(c=='\\'&&next!='\0'){blank(i);blank(++i);}
   else{blank(i);if(c==(state==Single?'\'':'"'))state=Code;}
  }else{
   if(c==']'&&next==']'){blank(i);blank(++i);state=Code;}
   else blank(i);
  }
 }
 return clean;
}
int lineAt(const std::string& text,size_t at){return 1+int(std::count(text.begin(),text.begin()+static_cast<std::ptrdiff_t>(at),'\n'));}
int errorLine(const std::string& message){
 // Lua compiles chunks named @source.lua, errors use source.lua:<line>: ...
 size_t first=message.find(':');
 while(first!=std::string::npos){
  size_t begin=first+1,end=begin;
  while(end<message.size()&&std::isdigit(static_cast<unsigned char>(message[end])))++end;
  if(end>begin&&end<message.size()&&message[end]==':'){
   try{return (std::max)(1,std::stoi(message.substr(begin,end-begin)));}catch(...){return 1;}
  }
  first=message.find(':',first+1);
 }
 return 1;
}
}
std::vector<LuaDiagnostic> diagnoseLua(std::string_view source){
 std::vector<LuaDiagnostic> out;
 lua_State* L=luaL_newstate();
 if(!L){out.push_back({1,DiagnosticSeverity::Error,"Could not initialize Lua parser"});return out;}
 const std::string sourceText(source);
 const int status=luaL_loadbufferx(L,sourceText.data(),sourceText.size(),"@script.lua","t");
 if(status!=LUA_OK){
  const char* error=lua_tostring(L,-1);std::string message=error?error:"Lua syntax error";
  out.push_back({errorLine(message),DiagnosticSeverity::Error,message});
 }
 lua_close(L);
 const auto code=maskNonCode(source);
 for(size_t p=0;(p=code.find("Engine",p))!=std::string::npos;++p){
  if(p>0&&(std::isalnum(static_cast<unsigned char>(code[p-1]))||code[p-1]=='_'))continue;
  size_t i=p+6;
  while(i<code.size()&&(code[i]==' '||code[i]=='\t'))++i;
  if(i>=code.size()||code[i]!='.')continue;
  ++i;while(i<code.size()&&(code[i]==' '||code[i]=='\t'))++i;
  const size_t start=i;
  while(i<code.size()&&(std::isalnum(static_cast<unsigned char>(code[i]))||code[i]=='_'))++i;
  if(start==i)continue;
  const std::string member=code.substr(start,i-start);
  while(i<code.size()&&(code[i]==' '||code[i]=='\t'))++i;
  if(i<code.size()&&code[i]=='('&&!validFunctions.count(member)){
   out.push_back({lineAt(code,p),DiagnosticSeverity::Warning,"Unknown Engine API: Engine."+member+"()"});
  }
 }
 // Detect lifecycle callbacks that look like engine events but will never be invoked.
 // Only function declarations are checked, not string literals or comments.
 const std::regex eventFunction(R"AU(\bfunction\s+[A-Za-z_][A-Za-z_0-9]*[.:](on_[A-Za-z_0-9]+)\s*\()AU");
 for(auto it=std::sregex_iterator(code.begin(),code.end(),eventFunction);
     it!=std::sregex_iterator();++it){
  const std::string event=(*it)[1].str();
  if(event!="on_start"&&event!="on_update"&&event!="on_stop")
   out.push_back({lineAt(code,static_cast<size_t>((*it).position())),DiagnosticSeverity::Warning,
     "Unknown lifecycle callback: "+event+" (supported: on_start, on_update, on_stop)"});
 }
 std::sort(out.begin(),out.end(),[](const auto& a,const auto& b){return a.line<b.line;});
 return out;
}
}
