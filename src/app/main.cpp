/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/app/main.cpp
 * @license MIT
 */

#include "app/Application.hpp"
#include <filesystem>
#include <iostream>
int main(int argc,char** argv){
 std::filesystem::path project=std::filesystem::absolute(argv[0]).parent_path()/"StarterWorld";
 bool play=false;
 for(int i=1;i<argc;++i){std::string arg=argv[i];
  if(arg=="--play"){
   std::cerr<<"Use the separate AuvyronPlayer executable for gameplay.\n";return 2;
  }
  else if(arg=="--help"||arg=="-h"){
   std::cout<<"AuvyronEngine [project_folder]  (editor only; game uses AuvyronPlayer)\n";return 0;
  }else project=arg;
 }
 auv::Application app;
 if(!app.init(project,play)){app.shutdown();return 1;}
 int result=app.run();app.shutdown();return result;
}
