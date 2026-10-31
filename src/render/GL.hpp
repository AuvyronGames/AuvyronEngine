/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/render/GL.hpp
 * @license MIT
 */

#pragma once
#include <GLFW/glfw3.h>
#include <cstddef>
namespace auv {
struct GL {
 using Size=std::ptrdiff_t;
 GLuint (APIENTRY *CreateShader)(GLenum){};
 void (APIENTRY *ShaderSource)(GLuint,GLsizei,const char* const*,const GLint*){};
 void (APIENTRY *CompileShader)(GLuint){};
 void (APIENTRY *GetShaderiv)(GLuint,GLenum,GLint*){};
 void (APIENTRY *GetShaderInfoLog)(GLuint,GLsizei,GLsizei*,char*){};
 GLuint (APIENTRY *CreateProgram)(){};
 void (APIENTRY *AttachShader)(GLuint,GLuint){};
 void (APIENTRY *LinkProgram)(GLuint){};
 void (APIENTRY *GetProgramiv)(GLuint,GLenum,GLint*){};
 void (APIENTRY *GetProgramInfoLog)(GLuint,GLsizei,GLsizei*,char*){};
 void (APIENTRY *DeleteShader)(GLuint){};
 void (APIENTRY *DeleteProgram)(GLuint){};
 void (APIENTRY *UseProgram)(GLuint){};
 GLint (APIENTRY *GetUniformLocation)(GLuint,const char*){};
 void (APIENTRY *UniformMatrix4fv)(GLint,GLsizei,GLboolean,const GLfloat*){};
 void (APIENTRY *Uniform3f)(GLint,GLfloat,GLfloat,GLfloat){};
 void (APIENTRY *Uniform1i)(GLint,GLint){};
 void (APIENTRY *Uniform1f)(GLint,GLfloat){};
 void (APIENTRY *ActiveTexture)(GLenum){};
 void (APIENTRY *GenVertexArrays)(GLsizei,GLuint*){};
 void (APIENTRY *BindVertexArray)(GLuint){};
 void (APIENTRY *DeleteVertexArrays)(GLsizei,const GLuint*){};
 void (APIENTRY *GenBuffers)(GLsizei,GLuint*){};
 void (APIENTRY *BindBuffer)(GLenum,GLuint){};
 void (APIENTRY *BufferData)(GLenum,Size,const void*,GLenum){};
 void (APIENTRY *DeleteBuffers)(GLsizei,const GLuint*){};
 void (APIENTRY *EnableVertexAttribArray)(GLuint){};
 void (APIENTRY *VertexAttribPointer)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*){};
 void (APIENTRY *GenFramebuffers)(GLsizei,GLuint*){};
 void (APIENTRY *BindFramebuffer)(GLenum,GLuint){};
 void (APIENTRY *FramebufferTexture2D)(GLenum,GLenum,GLenum,GLuint,GLint){};
 GLenum (APIENTRY *CheckFramebufferStatus)(GLenum){};
 void (APIENTRY *DeleteFramebuffers)(GLsizei,const GLuint*){};
 void (APIENTRY *GenRenderbuffers)(GLsizei,GLuint*){};
 void (APIENTRY *BindRenderbuffer)(GLenum,GLuint){};
 void (APIENTRY *RenderbufferStorage)(GLenum,GLenum,GLsizei,GLsizei){};
 void (APIENTRY *FramebufferRenderbuffer)(GLenum,GLenum,GLenum,GLuint){};
 void (APIENTRY *DeleteRenderbuffers)(GLsizei,const GLuint*){};
 bool load();
};
extern GL glx;
}
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA8 0x8058
#endif

#ifndef GL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP 0x8513
#endif
#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#endif
#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R 0x8072
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_RGB8
#define GL_RGB8 0x8051
#endif

#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif
