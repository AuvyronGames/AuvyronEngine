/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/render/Renderer.cpp
 * @license MIT
 */

#include "render/Renderer.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_EASY_FONT_IMPLEMENTATION
#include <stb_easy_font.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <climits>
namespace auv {
Vec3 Camera::eye()const{
 if(firstPerson)return position;
 const float a=rad(yaw),b=rad(pitch);
 return target+Vec3{std::sin(a)*std::cos(b),std::sin(b),std::cos(a)*std::cos(b)}*distance;
}
Vec3 Camera::forward()const{
 if(firstPerson){
  const float a=rad(yaw),b=rad(pitch);
  return {std::sin(a)*std::cos(b),std::sin(b),-std::cos(a)*std::cos(b)};
 }
 return norm(target-eye());
}
Mat4 Camera::view()const{
 if(firstPerson)return Mat4::lookAt(position,position+forward(),{0,1,0});
 return Mat4::lookAt(eye(),target,{0,1,0});
}
void Camera::orbit(float dx,float dy){yaw+=dx*0.28f;pitch=std::clamp(pitch+dy*0.28f,-85.f,85.f);}
void Camera::pan(float dx,float dy){auto forward=norm(target-eye());auto right=norm(cross(forward,{0,1,0}));auto up=norm(cross(right,forward));target+=right*(-dx*distance*0.0015f)+up*(dy*distance*0.0015f);}
GLuint Renderer::compile(GLenum kind,const char* text){
 GLuint s=glx.CreateShader(kind);glx.ShaderSource(s,1,&text,nullptr);glx.CompileShader(s);
 GLint ok=0;glx.GetShaderiv(s,GL_COMPILE_STATUS,&ok);
 if(!ok){char log[4096]{};glx.GetShaderInfoLog(s,4096,nullptr,log);error_=log;glx.DeleteShader(s);return 0;}return s;
}
Renderer::Mesh Renderer::createMesh(const std::vector<Vertex>& verts){
 Mesh out;if(verts.empty())return out;
 glx.GenVertexArrays(1,&out.vao);glx.GenBuffers(1,&out.vbo);glx.BindVertexArray(out.vao);
 glx.BindBuffer(GL_ARRAY_BUFFER,out.vbo);glx.BufferData(GL_ARRAY_BUFFER,GL::Size(verts.size()*sizeof(Vertex)),verts.data(),GL_STATIC_DRAW);
 glx.EnableVertexAttribArray(0);glx.VertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),nullptr);
 glx.EnableVertexAttribArray(1);glx.VertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<const void*>(3*sizeof(float)));
 glx.BindVertexArray(0);out.count=GLsizei(verts.size());return out;
}
void Renderer::deleteMesh(Mesh& mesh){if(mesh.vbo)glx.DeleteBuffers(1,&mesh.vbo);if(mesh.vao)glx.DeleteVertexArrays(1,&mesh.vao);mesh={};}
bool Renderer::init(){
 const char* vs=R"GLSL(#version 330 core
 layout(location=0)in vec3 aPos;
 layout(location=1)in vec3 aNormal;
 uniform mat4 uModel,uView,uProj;
 out vec3 vN,vP;
 void main(){vec4 p=uModel*vec4(aPos,1.0);vP=p.xyz;vN=mat3(transpose(inverse(uModel)))*aNormal;gl_Position=uProj*uView*p;}
 )GLSL";
 const char* fs=R"GLSL(#version 330 core
 in vec3 vN,vP;out vec4 frag;
 uniform vec3 uColor,uEye;
 uniform vec3 uLightColor,uLightPos,uLightDir;
 uniform float uLightIntensity,uLightRange,uSpotCos;
 uniform int uLightType,uUnlit;
 void main(){
 vec3 n=normalize(vN);
 vec3 fromLight=uLightPos-vP;
 float dist=length(fromLight);
 vec3 l=uLightType==0?normalize(-uLightDir):normalize(fromLight);
 float atten=uLightType==0?1.0:pow(clamp(1.0-dist/max(uLightRange,0.01),0.0,1.0),2.0);
 if(uLightType==2){float c=dot(normalize(-uLightDir),normalize(vP-uLightPos));
  atten*=smoothstep(uSpotCos,min(1.0,uSpotCos+0.09),c);}
 float diffuse=max(dot(n,l),0.0)*uLightIntensity*atten;
 vec3 h=normalize(l+normalize(uEye-vP));float spec=pow(max(dot(n,h),0.0),36.0)*0.22;
 vec3 c=uUnlit==1?uColor:uColor*(0.21+diffuse*uLightColor)+vec3(spec*atten);
 frag=vec4(pow(c,vec3(1.0/2.2)),1.0);
 })GLSL";
 GLuint v=compile(GL_VERTEX_SHADER,vs),f=compile(GL_FRAGMENT_SHADER,fs);
 if(!v||!f){if(v)glx.DeleteShader(v);if(f)glx.DeleteShader(f);return false;}
 shader_=glx.CreateProgram();glx.AttachShader(shader_,v);glx.AttachShader(shader_,f);glx.LinkProgram(shader_);
 glx.DeleteShader(v);glx.DeleteShader(f);GLint ok=0;glx.GetProgramiv(shader_,GL_LINK_STATUS,&ok);
 if(!ok){char log[4096]{};glx.GetProgramInfoLog(shader_,4096,nullptr,log);error_=log;return false;}

 const char* skyVS=R"GLSL(#version 330 core
 out vec2 uv;
 void main(){
  vec2 pos=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
  uv=pos;
  gl_Position=vec4(pos*2.0-1.0,0.9999,1.0);
 }
 )GLSL";
 const char* skyFS=R"GLSL(#version 330 core
 in vec2 uv;
 out vec4 frag;
 uniform vec3 uForward,uRight,uUp,uZenith,uHorizon;
 uniform float uAspect;
 uniform int uCubemap;
 uniform samplerCube uSky;
 void main(){
  vec2 p=uv*2.0-1.0;
  vec3 ray=normalize(uForward+uRight*p.x*uAspect*0.520567+uUp*p.y*0.520567);
  vec3 color=mix(uHorizon,uZenith,smoothstep(-0.15,0.88,ray.y));
  if(uCubemap==1)color=texture(uSky,ray).rgb;
  frag=vec4(color,1.0);
 }
 )GLSL";
 GLuint skyVert=compile(GL_VERTEX_SHADER,skyVS),skyFrag=compile(GL_FRAGMENT_SHADER,skyFS);
 if(!skyVert||!skyFrag){if(skyVert)glx.DeleteShader(skyVert);if(skyFrag)glx.DeleteShader(skyFrag);return false;}
 skyShader_=glx.CreateProgram();glx.AttachShader(skyShader_,skyVert);glx.AttachShader(skyShader_,skyFrag);glx.LinkProgram(skyShader_);
 glx.DeleteShader(skyVert);glx.DeleteShader(skyFrag);
 GLint skyOkay=0;glx.GetProgramiv(skyShader_,GL_LINK_STATUS,&skyOkay);
 if(!skyOkay){error_="Skybox GLSL shader link failed";return false;}

 const char* uiVS=R"GLSL(#version 330 core
 layout(location=0)in vec2 aPos;
 layout(location=1)in vec4 aColor;
 out vec4 vColor;
 void main(){gl_Position=vec4(aPos,0,1);vColor=aColor;}
 )GLSL";
 const char* uiFS=R"GLSL(#version 330 core
 in vec4 vColor;out vec4 frag;
 void main(){frag=vColor;}
 )GLSL";
 GLuint uv=compile(GL_VERTEX_SHADER,uiVS),uf=compile(GL_FRAGMENT_SHADER,uiFS);
 if(!uv||!uf)return false;
 uiShader_=glx.CreateProgram();glx.AttachShader(uiShader_,uv);glx.AttachShader(uiShader_,uf);
 glx.LinkProgram(uiShader_);glx.DeleteShader(uv);glx.DeleteShader(uf);
 GLint uiOkay=0;glx.GetProgramiv(uiShader_,GL_LINK_STATUS,&uiOkay);
 if(!uiOkay){error_="Runtime UI shader link failed";return false;}
 glx.GenVertexArrays(1,&uiVao_);glx.GenBuffers(1,&uiVbo_);
 glx.BindVertexArray(uiVao_);glx.BindBuffer(GL_ARRAY_BUFFER,uiVbo_);
 glx.EnableVertexAttribArray(0);glx.VertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),nullptr);
 glx.EnableVertexAttribArray(1);glx.VertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));
 glx.BindVertexArray(0);
 glx.GenVertexArrays(1,&skyVao_);
 std::vector<Vertex> cube;
 auto quad=[&](Vec3 a,Vec3 b,Vec3 c,Vec3 d,Vec3 n){for(auto p:{a,b,c,a,c,d})cube.push_back({p.x,p.y,p.z,n.x,n.y,n.z});};
 float s=0.5f;
 quad({-s,-s,s},{s,-s,s},{s,s,s},{-s,s,s},{0,0,1});
 quad({s,-s,-s},{-s,-s,-s},{-s,s,-s},{s,s,-s},{0,0,-1});
 quad({s,-s,s},{s,-s,-s},{s,s,-s},{s,s,s},{1,0,0});
 quad({-s,-s,-s},{-s,-s,s},{-s,s,s},{-s,s,-s},{-1,0,0});
 quad({-s,s,s},{s,s,s},{s,s,-s},{-s,s,-s},{0,1,0});
 quad({-s,-s,-s},{s,-s,-s},{s,-s,s},{-s,-s,s},{0,-1,0});
 cube_=createMesh(cube);
 std::vector<Vertex> plane={ {-0.5f,0,-0.5f,0,1,0},{-0.5f,0,0.5f,0,1,0},{0.5f,0,0.5f,0,1,0},
 {-0.5f,0,-0.5f,0,1,0},{0.5f,0,0.5f,0,1,0},{0.5f,0,-0.5f,0,1,0}};
 plane_=createMesh(plane);
 std::vector<Vertex> sphere;const int stacks=20,slices=32;
 auto pt=[&](int a,int b){float phi=3.14159265f*float(a)/stacks,theta=6.2831853f*float(b)/slices;
  return Vec3{std::sin(phi)*std::cos(theta),std::cos(phi),std::sin(phi)*std::sin(theta)};};
 for(int a=0;a<stacks;++a)for(int b=0;b<slices;++b){Vec3 p=pt(a,b),q=pt(a+1,b),r=pt(a+1,b+1),t=pt(a,b+1);
  for(auto x:{p,q,r,p,r,t})sphere.push_back({x.x*0.5f,x.y*0.5f,x.z*0.5f,x.x,x.y,x.z});}
 sphere_=createMesh(sphere);
 std::vector<Vertex> grid;
 for(int i=-25;i<=25;++i){float x=float(i);
  grid.push_back({x,0,-25,0,1,0});grid.push_back({x,0,25,0,1,0});
  grid.push_back({-25,0,x,0,1,0});grid.push_back({25,0,x,0,1,0});}
 grid_=createMesh(grid);
 return resize(1024,720);
}
bool Renderer::resize(int w,int h){
 w=std::clamp(w,16,4096);h=std::clamp(h,16,4096);
 if(width_==w && height_==h)return true;
 if(fbo_)glx.DeleteFramebuffers(1,&fbo_);
 if(depth_)glx.DeleteRenderbuffers(1,&depth_);
 if(color_)glDeleteTextures(1,&color_);
 glx.GenFramebuffers(1,&fbo_);glx.BindFramebuffer(GL_FRAMEBUFFER,fbo_);
 glGenTextures(1,&color_);glBindTexture(GL_TEXTURE_2D,color_);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 glx.FramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color_,0);
 glx.GenRenderbuffers(1,&depth_);glx.BindRenderbuffer(GL_RENDERBUFFER,depth_);
 glx.RenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,w,h);
 glx.FramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth_);
 bool ok=glx.CheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
 glx.BindFramebuffer(GL_FRAMEBUFFER,0);width_=w;height_=h;
 if(!ok)error_="OpenGL framebuffer incomplete";
 return ok;
}
Renderer::Mesh Renderer::loadOBJ(const std::filesystem::path& file){
 std::string content;
 if(resources_){if(!resources_->readText(file.lexically_relative(resources_->root()),content)){
  error_="OBJ unavailable in package: "+file.string();return {};}}
 else{std::ifstream input(file);if(!input){error_="OBJ missing: "+file.string();return {};}
  std::ostringstream ss;ss<<input.rdbuf();content=ss.str();}
 std::istringstream in(content);
 std::vector<Vec3> points,normals;std::vector<Vertex> verts;
 std::string line;
 while(std::getline(in,line)){
  std::istringstream ss(line);std::string cmd;ss>>cmd;
  if(cmd=="v"){Vec3 p;ss>>p.x>>p.y>>p.z;points.push_back(p);}
  else if(cmd=="vn"){Vec3 n;ss>>n.x>>n.y>>n.z;normals.push_back(n);}
  else if(cmd=="f"){
   std::vector<Vertex> face;std::string token;
   while(ss>>token){if(face.size()>128)break;
    int vi=0,ni=0;std::replace(token.begin(),token.end(),'/',' ');
    std::istringstream vertex(token);vertex>>vi;
    // Supports v//vn and v/vt/vn by reading the original face token.
    std::string original=token; // Fallback flat normal is computed below.
    (void)original;(void)ni;
    if(vi<0)vi=int(points.size())+vi+1;
    if(vi<=0||vi>int(points.size()))continue;
    auto p=points[vi-1];face.push_back({p.x,p.y,p.z,0,1,0});
   }
   if(face.size()<3)continue;
   for(std::size_t k=1;k+1<face.size();++k){
    Vertex tri[]={face[0],face[k],face[k+1]};
    Vec3 a{tri[0].x,tri[0].y,tri[0].z},b{tri[1].x,tri[1].y,tri[1].z},c{tri[2].x,tri[2].y,tri[2].z};
    Vec3 n=norm(cross(b-a,c-a));
    for(auto& v:tri){v.nx=n.x;v.ny=n.y;v.nz=n.z;verts.push_back(v);}
   }
  }
 }
 if(verts.empty())error_="OBJ has no triangles: "+file.string();
 return createMesh(verts);
}
const Renderer::Mesh* Renderer::meshFor(const Entity& e,const std::filesystem::path& projectRoot){
 if(e.shape==Shape::Cube)return &cube_;
 if(e.shape==Shape::Plane)return &plane_;
 if(e.shape==Shape::Sphere)return &sphere_;
 if(e.mesh.empty())return &cube_;
 auto path=std::filesystem::weakly_canonical(projectRoot/e.mesh);
 auto base=std::filesystem::weakly_canonical(projectRoot).generic_string()+"/";
 if(path.generic_string().rfind(base,0)!=0)return &cube_;
 auto key=path.generic_string();auto it=obj_.find(key);
 if(it==obj_.end())it=obj_.emplace(key,loadOBJ(path)).first;
 return it->second.count?&it->second:&cube_;
}
void Renderer::updateSkybox(const Scene& scene,const std::filesystem::path& root){
 std::string signature=scene.skybox.useCubemap?"cubemap:":"gradient:";
 for(const auto& face:scene.skybox.faces){
  signature+=face+"|";
  std::error_code ec;
  if(!face.empty()){
   auto mod=std::filesystem::last_write_time(root/face,ec);
   if(!ec)signature+=std::to_string(mod.time_since_epoch().count());
  }
  signature+=';';
 }
 if(signature==skyTextureSignature_)return;
 skyTextureSignature_=signature;skyLoaded_=false;
 if(skyTexture_){glDeleteTextures(1,&skyTexture_);skyTexture_=0;}
 if(!scene.skybox.useCubemap)return;
 std::array<std::vector<unsigned char>,6> pixels;
 int dimension=0;
 std::error_code ec;
 const auto base=std::filesystem::weakly_canonical(root,ec);
 if(ec)return;
 for(std::size_t i=0;i<scene.skybox.faces.size();++i){
  const auto& path=scene.skybox.faces[i];
  if(path.empty()){error_="Skybox: assign all 6 faces";return;}
  const auto rel=std::filesystem::path(path);
  if(rel.is_absolute()){error_="Skybox paths must be relative to project";return;}
  const auto absolute=std::filesystem::weakly_canonical(base/rel,ec);
  if(ec||absolute==base){error_="Skybox: invalid texture path";return;}
  auto b=base.begin(),a=absolute.begin();
  for(;b!=base.end();++b,++a)if(a==absolute.end()||*a!=*b){error_="Skybox path leaves project";return;}
  int w=0,h=0,channels=0;
  AssetPackage::Bytes packed;
  stbi_uc* bytes=nullptr;
  if(resources_){
   if(resources_->read(rel,packed)&&packed.size()<=static_cast<std::size_t>(INT_MAX))
    bytes=stbi_load_from_memory(packed.data(),static_cast<int>(packed.size()),&w,&h,&channels,3);
  }else bytes=stbi_load(absolute.string().c_str(),&w,&h,&channels,3);
  if(!bytes||w!=h||w<1||w>4096||(dimension&&dimension!=w)){
   error_="Skybox: faces must be same-size square PNG or JPG images";
   if(bytes)stbi_image_free(bytes);
   return;
  }
  dimension=w;pixels[i].assign(bytes,bytes+std::size_t(w)*std::size_t(h)*3);
  stbi_image_free(bytes);
 }
 glGenTextures(1,&skyTexture_);glBindTexture(GL_TEXTURE_CUBE_MAP,skyTexture_);
 glPixelStorei(GL_UNPACK_ALIGNMENT,1);
 for(int i=0;i<6;++i)glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB8,
  dimension,dimension,0,GL_RGB,GL_UNSIGNED_BYTE,pixels[i].data());
 glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
 glBindTexture(GL_TEXTURE_CUBE_MAP,0);
 skyLoaded_=true;error_.clear();
}
void Renderer::drawSkybox(const Scene& scene,const Camera& camera){
 if(!scene.skybox.enabled)return;
 const Vec3 forward=camera.forward();
 const Vec3 right=norm(cross(forward,{0,1,0}));
 const Vec3 up=norm(cross(right,forward));
 glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);
 glx.UseProgram(skyShader_);
 auto set=[&](const char* uniform,Vec3 v){glx.Uniform3f(glx.GetUniformLocation(skyShader_,uniform),v.x,v.y,v.z);};
 set("uForward",forward);set("uRight",right);set("uUp",up);
 set("uZenith",scene.skybox.zenith);set("uHorizon",scene.skybox.horizon);
 glx.Uniform1f(glx.GetUniformLocation(skyShader_,"uAspect"),float(width_)/float(height_));
 glx.Uniform1i(glx.GetUniformLocation(skyShader_,"uCubemap"),scene.skybox.useCubemap&&skyLoaded_?1:0);
 glx.ActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_CUBE_MAP,skyTexture_);
 glx.Uniform1i(glx.GetUniformLocation(skyShader_,"uSky"),0);
 glx.BindVertexArray(skyVao_);glDrawArrays(GL_TRIANGLES,0,3);
 glx.BindVertexArray(0);glBindTexture(GL_TEXTURE_CUBE_MAP,0);
 glx.UseProgram(0);glDepthMask(GL_TRUE);
}
void Renderer::draw(const Scene& scene,const Camera& camera,EntityId selected,const std::filesystem::path& root,bool runtime){
 if(!runtime&&!fbo_)return;
 glx.BindFramebuffer(GL_FRAMEBUFFER,runtime?0:fbo_);glViewport(0,0,width_,height_);
 glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glClearColor(0.055f,0.078f,0.103f,1);
 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 updateSkybox(scene,root);
 drawSkybox(scene,camera);
 glEnable(GL_DEPTH_TEST);
 glx.UseProgram(shader_);
 auto view=camera.view(),proj=camera.projection(float(width_)/height_);
 glx.UniformMatrix4fv(glx.GetUniformLocation(shader_,"uView"),1,GL_FALSE,view.m);
 glx.UniformMatrix4fv(glx.GetUniformLocation(shader_,"uProj"),1,GL_FALSE,proj.m);
 auto eye=camera.eye();glx.Uniform3f(glx.GetUniformLocation(shader_,"uEye"),eye.x,eye.y,eye.z);
 auto modelLoc=glx.GetUniformLocation(shader_,"uModel");auto colorLoc=glx.GetUniformLocation(shader_,"uColor");auto unlitLoc=glx.GetUniformLocation(shader_,"uUnlit");
 Mat4 identity=Mat4::identity();glx.UniformMatrix4fv(modelLoc,1,GL_FALSE,identity.m);
 glx.Uniform3f(colorLoc,0.17f,0.24f,0.30f);glx.Uniform1i(unlitLoc,1);
 if(!runtime){glx.BindVertexArray(grid_.vao);glDrawArrays(GL_LINES,0,grid_.count);}
 glx.Uniform1i(unlitLoc,0);
 const Entity* activeLight=nullptr;
 for(const auto& e:scene.entities())if(e.visible&&e.kind>=ComponentKind::DirectionalLight && e.kind<=ComponentKind::SpotLight){activeLight=&e;break;}
 const auto setLight=[&](const char* name,Vec3 v){glx.Uniform3f(glx.GetUniformLocation(shader_,name),v.x,v.y,v.z);};
 setLight("uLightColor",activeLight?activeLight->lightColor:Vec3{1,.96f,.89f});
 setLight("uLightPos",activeLight?activeLight->position:Vec3{0,10,0});
 setLight("uLightDir",activeLight?Vec3{std::sin(rad(activeLight->rotation.y)),-std::cos(rad(activeLight->rotation.x)),std::cos(rad(activeLight->rotation.y))}:Vec3{.5f,-1.f,-.7f});
 glx.Uniform1i(glx.GetUniformLocation(shader_,"uLightType"),activeLight?int(activeLight->kind)-1:0);
 glx.Uniform1f(glx.GetUniformLocation(shader_,"uLightIntensity"),activeLight?activeLight->intensity:1.f);
 glx.Uniform1f(glx.GetUniformLocation(shader_,"uLightRange"),activeLight?activeLight->range:100.f);
 glx.Uniform1f(glx.GetUniformLocation(shader_,"uSpotCos"),activeLight?std::cos(rad(activeLight->spotAngle)):.7f);
 for(auto& e:scene.entities()){
  if(!e.visible || (e.kind!=ComponentKind::Mesh && e.kind!=ComponentKind::Fire && e.kind!=ComponentKind::Explosion))continue;
  auto mesh=meshFor(e,root);if(!mesh||!mesh->count)continue;
  Vec3 meshScale=e.scale;
  if(e.kind==ComponentKind::Fire){const float pulse=1.f+.12f*std::sin(e.effectAge*14.f);meshScale=meshScale*pulse;}
  if(e.kind==ComponentKind::Explosion){const float t=std::clamp(e.effectAge/std::max(.01f,e.effectDuration),0.f,1.f);meshScale=meshScale*(.2f+4.f*t);}
  auto model=modelMatrix(e.position,e.rotation,meshScale);
  glx.UniformMatrix4fv(modelLoc,1,GL_FALSE,model.m);
  glx.Uniform3f(colorLoc,e.color.x,e.color.y,e.color.z);
  if(e.kind==ComponentKind::Fire||e.kind==ComponentKind::Explosion)glx.Uniform1i(unlitLoc,1);
  glx.BindVertexArray(mesh->vao);glDrawArrays(GL_TRIANGLES,0,mesh->count);
  if(e.kind==ComponentKind::Fire||e.kind==ComponentKind::Explosion)glx.Uniform1i(unlitLoc,0);
  if(!runtime&&selected==e.id){
   glDisable(GL_DEPTH_TEST);glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);glLineWidth(2);
   glx.Uniform1i(unlitLoc,1);glx.Uniform3f(colorLoc,0.35f,0.86f,0.94f);
   glDrawArrays(GL_TRIANGLES,0,mesh->count);
   glx.Uniform1i(unlitLoc,0);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glEnable(GL_DEPTH_TEST);
  }
 }
 glx.BindVertexArray(0);glx.UseProgram(0);glDisable(GL_CULL_FACE);
 glx.BindFramebuffer(GL_FRAMEBUFFER,0);
}
bool Renderer::project(Vec3 point,const Camera& camera,float& x,float& y)const{
 if(width_==0||height_==0)return false;
 Mat4 m=camera.projection(float(width_)/height_)*camera.view();
 float w=m.m[3]*point.x+m.m[7]*point.y+m.m[11]*point.z+m.m[15];if(w<=0.01f)return false;
 float nx=(m.m[0]*point.x+m.m[4]*point.y+m.m[8]*point.z+m.m[12])/w;
 float ny=(m.m[1]*point.x+m.m[5]*point.y+m.m[9]*point.z+m.m[13])/w;
 x=(nx*0.5f+0.5f)*width_;y=(1-(ny*0.5f+0.5f))*height_;
 return nx>=-1&&nx<=1&&ny>=-1&&ny<=1;
}
void Renderer::drawGameUI(const GameUI& ui,bool runtime){
 if(!uiShader_||width_<=0||height_<=0||ui.elements().empty())return;
 glx.BindFramebuffer(GL_FRAMEBUFFER,runtime?0:fbo_);glViewport(0,0,width_,height_);
 glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 std::vector<float> verts;verts.reserve(8192);
 const auto addVertex=[&](float x,float y,float r,float g,float b,float a){
  verts.insert(verts.end(),{x*2.f/float(width_)-1.f,1.f-y*2.f/float(height_),r,g,b,a});
 };
 const auto quad=[&](float x,float y,float w,float h,float r,float g,float b,float a){
  addVertex(x,y,r,g,b,a);addVertex(x+w,y,r,g,b,a);addVertex(x+w,y+h,r,g,b,a);
  addVertex(x,y,r,g,b,a);addVertex(x+w,y+h,r,g,b,a);addVertex(x,y+h,r,g,b,a);
 };
 for(const auto& item:ui.elements()){
  const float x=item.x*width_,y=item.y*height_,w=item.w*width_,h=item.h*height_;
  if(item.type!=UIElement::Type::Text)quad(x,y,w,h,item.r,item.g,item.b,item.a);
  if(item.type==UIElement::Type::Text||item.type==UIElement::Type::Button){
   const std::string label=item.text.substr(0,140);
   // stb_easy_font is an embedded ASCII raster-style font; no texture file required.
   char buffer[30000]{};
   const int count=stb_easy_font_print(0,0,const_cast<char*>(label.c_str()),nullptr,buffer,sizeof(buffer));
   const float x0=x+(item.type==UIElement::Type::Button?10.f:0.f);
   const float y0=y+(item.type==UIElement::Type::Button?std::max(2.f,(h-12.f)*.5f):0.f);
   for(int j=0;j<count;++j){
    struct FontVertex {float x,y,z;unsigned char c[4];};
    const auto* q=reinterpret_cast<const FontVertex*>(buffer+std::size_t(j)*4*sizeof(FontVertex));
    auto tri=[&](int a,int b,int c){for(int k:{a,b,c}){
     const float r=item.type==UIElement::Type::Button?1.f:item.r;
     const float g=item.type==UIElement::Type::Button?1.f:item.g;
     const float bl=item.type==UIElement::Type::Button?1.f:item.b;
     addVertex(x0+q[k].x,y0+q[k].y,r,g,bl,item.a);
    }};
    tri(0,1,2);tri(0,2,3);
   }
  }
 }
 glx.UseProgram(uiShader_);glx.BindVertexArray(uiVao_);glx.BindBuffer(GL_ARRAY_BUFFER,uiVbo_);
 glx.BufferData(GL_ARRAY_BUFFER,GL::Size(verts.size()*sizeof(float)),verts.data(),GL_DYNAMIC_DRAW);
 glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(verts.size()/6));
 glx.BindVertexArray(0);glx.UseProgram(0);glDisable(GL_BLEND);
 glx.BindFramebuffer(GL_FRAMEBUFFER,0);
}
void Renderer::shutdown(){
 for(auto& [key,mesh]:obj_)deleteMesh(mesh);obj_.clear();
 deleteMesh(cube_);deleteMesh(plane_);deleteMesh(sphere_);deleteMesh(grid_);
 if(uiVbo_)glx.DeleteBuffers(1,&uiVbo_);uiVbo_=0;
 if(uiVao_)glx.DeleteVertexArrays(1,&uiVao_);uiVao_=0;
 if(uiShader_)glx.DeleteProgram(uiShader_);uiShader_=0;
 if(shader_)glx.DeleteProgram(shader_);shader_=0;
 if(skyShader_)glx.DeleteProgram(skyShader_);skyShader_=0;
 if(skyVao_)glx.DeleteVertexArrays(1,&skyVao_);skyVao_=0;
 if(skyTexture_)glDeleteTextures(1,&skyTexture_);skyTexture_=0;
 skyTextureSignature_.clear();skyLoaded_=false;
 if(fbo_)glx.DeleteFramebuffers(1,&fbo_);fbo_=0;
 if(depth_)glx.DeleteRenderbuffers(1,&depth_);depth_=0;
 if(color_)glDeleteTextures(1,&color_);color_=0;width_=height_=0;
}
}
