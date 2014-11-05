#pragma once

#include <GL/gl.h>

void TS_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
	    GLint srcX, GLint srcY, GLint srcZ,
	    GLuint dstName, GLenum dstTarget, GLint dstLevel,
	    GLint dstX, GLint dstY, GLint dstZ,
	    GLsizei width, GLsizei height, GLsizei depth);

#ifdef SHIM_GL_COPYIMAGESUBDATA

#define glCopyImageSubData TS_CopyImageSubData

#endif
