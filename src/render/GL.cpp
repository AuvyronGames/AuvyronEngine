/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/render/GL.cpp
 * @license MIT
 */

#include "render/GL.hpp"
namespace auv {
GL glx;
bool GL::load(){
 bool ok=true;
 #define LOAD(name) name=reinterpret_cast<decltype(name)>(glfwGetProcAddress("gl" #name));ok=ok&&name!=nullptr
 LOAD(CreateShader);LOAD(ShaderSource);LOAD(CompileShader);LOAD(GetShaderiv);LOAD(GetShaderInfoLog);
 LOAD(CreateProgram);LOAD(AttachShader);LOAD(LinkProgram);LOAD(GetProgramiv);LOAD(GetProgramInfoLog);
 LOAD(DeleteShader);LOAD(DeleteProgram);LOAD(UseProgram);LOAD(GetUniformLocation);
 LOAD(UniformMatrix4fv);LOAD(Uniform3f);LOAD(Uniform1i);LOAD(Uniform1f);LOAD(ActiveTexture);
 LOAD(GenVertexArrays);LOAD(BindVertexArray);LOAD(DeleteVertexArrays);LOAD(GenBuffers);LOAD(BindBuffer);
 LOAD(BufferData);LOAD(DeleteBuffers);LOAD(EnableVertexAttribArray);LOAD(VertexAttribPointer);
 LOAD(GenFramebuffers);LOAD(BindFramebuffer);LOAD(FramebufferTexture2D);
 LOAD(CheckFramebufferStatus);LOAD(DeleteFramebuffers);
 LOAD(GenRenderbuffers);LOAD(BindRenderbuffer);LOAD(RenderbufferStorage);
 LOAD(FramebufferRenderbuffer);LOAD(DeleteRenderbuffers);
 #undef LOAD
 return ok;
}
}
