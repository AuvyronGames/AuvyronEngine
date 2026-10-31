/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/Math.hpp
 * @license MIT
 */

#pragma once
#include <cmath>
#include <algorithm>

namespace auv {
struct Vec3 {
 float x=0,y=0,z=0;
 Vec3()=default;
 Vec3(float X,float Y,float Z):x(X),y(Y),z(Z){}
 Vec3 operator+(Vec3 b)const{return {x+b.x,y+b.y,z+b.z};}
 Vec3 operator-(Vec3 b)const{return {x-b.x,y-b.y,z-b.z};}
 Vec3 operator*(float k)const{return {x*k,y*k,z*k};}
 Vec3& operator+=(Vec3 b){x+=b.x;y+=b.y;z+=b.z;return *this;}
};
inline float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline Vec3 norm(Vec3 a){auto k=std::sqrt(dot(a,a));return k>0.000001f?a*(1.f/k):Vec3{};}
inline float rad(float d){return d*0.01745329251994329577f;}
struct Mat4 {
 float m[16]{};
 static Mat4 identity(){Mat4 r;for(int i=0;i<4;++i)r.m[i*5]=1;return r;}
 static Mat4 translation(Vec3 p){auto r=identity();r.m[12]=p.x;r.m[13]=p.y;r.m[14]=p.z;return r;}
 static Mat4 scale(Vec3 p){auto r=identity();r.m[0]=p.x;r.m[5]=p.y;r.m[10]=p.z;return r;}
 static Mat4 rotation(Vec3 d){
  float x=rad(d.x),y=rad(d.y),z=rad(d.z);
  Mat4 rx=identity(),ry=identity(),rz=identity();
  rx.m[5]=std::cos(x);rx.m[6]=std::sin(x);rx.m[9]=-std::sin(x);rx.m[10]=std::cos(x);
  ry.m[0]=std::cos(y);ry.m[2]=-std::sin(y);ry.m[8]=std::sin(y);ry.m[10]=std::cos(y);
  rz.m[0]=std::cos(z);rz.m[1]=std::sin(z);rz.m[4]=-std::sin(z);rz.m[5]=std::cos(z);
  return rz*ry*rx;
 }
 static Mat4 perspective(float fov,float aspect,float nearZ,float farZ){
  Mat4 r;float s=1/std::tan(rad(fov)*0.5f);
  r.m[0]=s/aspect;r.m[5]=s;r.m[10]=(farZ+nearZ)/(nearZ-farZ);
  r.m[11]=-1;r.m[14]=(2*farZ*nearZ)/(nearZ-farZ);return r;
 }
 static Mat4 lookAt(Vec3 eye,Vec3 target,Vec3 up){
  Vec3 f=norm(target-eye),s=norm(cross(f,up)),u=cross(s,f);
  Mat4 r=identity();r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;
  r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;
  r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;
  r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;
 }
 Mat4 operator*(const Mat4& b)const{
  Mat4 r;
  for(int c=0;c<4;++c)for(int row=0;row<4;++row){
   for(int k=0;k<4;++k)r.m[c*4+row]+=m[k*4+row]*b.m[c*4+k];
  }
  return r;
 }
};
inline Mat4 modelMatrix(Vec3 p,Vec3 rot,Vec3 scl){return Mat4::translation(p)*Mat4::rotation(rot)*Mat4::scale(scl);}
}
