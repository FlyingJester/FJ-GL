#ifndef FJGL_GLEXTRA_HEAD
#define FJGL_GLEXTRA_HEAD
#include <GL/gl.h>

extern void (APIENTRY * glGenBuffers)(GLsizei, GLuint*);
extern void (APIENTRY * glDeleteBuffers)(GLsizei, GLuint*);
extern void (APIENTRY * glBindBuffer)(GLenum,  GLuint);
extern void (APIENTRY * glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
extern void (APIENTRY * glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *);
extern void (APIENTRY * glCopyImageSubData)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
extern GLenum (APIENTRY * glCreateShader)(GLenum);
extern void (APIENTRY * glDeleteShader)(GLenum);
extern void (APIENTRY * glShaderSource)(GLenum, GLint, const GLchar **, const GLint *);
extern void (APIENTRY * glGetShaderiv)(GLuint, GLenum, GLint*);
extern void (APIENTRY * glCompileShader)(GLenum);
extern GLenum (APIENTRY * glCreateProgram)(void);
extern void (APIENTRY * glUseProgram)(GLenum);
extern void (APIENTRY * glAttachShader)(GLenum, GLenum);
extern void (APIENTRY * glLinkProgram)(GLenum);
extern void (APIENTRY * glGetProgramiv)(GLuint, GLenum, GLint*);
extern GLboolean (APIENTRY * glIsShader)(GLuint);
extern void (APIENTRY * glGetShaderInfoLog)(GLuint,  GLsizei,  GLsizei *,  GLchar *);
extern void (APIENTRY * glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
extern void (APIENTRY * glDeleteProgram)(GLuint);

void LoadGLFunctions(void);

#endif
