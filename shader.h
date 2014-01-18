#ifndef FJGL_SHADER_HEAD
#define FJGL_SHADER_HEAD
#include <GL/gl.h>
#include "api.hpp"
#ifdef __cplusplus
extern "C"{
#endif
EXPORT(GLuint LoadEmbeddedShader(void));
extern GLuint CurrentShader;
extern GLuint DefaultShader;
#ifdef __cplusplus
}
#endif
EXPORT(GLuint TS_LoadSystemShader(const char *file));
EXPORT(GLuint TS_LoadShader(const char *file));
EXPORT(GLuint TS_CreateProgram(GLuint frag, GLuint vert));
EXPORT(GLuint TS_CreateShader(const char *text, GLenum type));

#endif
