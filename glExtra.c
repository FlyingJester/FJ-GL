#include "api.h"
#include "glExtra.h"
#include "config.h"
#define GL_GLEXT_PROTOTYPES 1

#ifdef __linux__
#include <SDL/SDL_opengl.h>
#include <SDL/SDL.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define rmask 0xff000000
    #define gmask 0x00ff0000
    #define bmask 0x0000ff00
    #define amask 0x000000ff
#else
    #define rmask 0x000000ff
    #define gmask 0x0000ff00
    #define bmask 0x00ff0000
    #define amask 0xff000000
#endif
#else

//Very, very little-endian.

    #define rmask 0x000000ff
    #define gmask 0x0000ff00
    #define bmask 0x00ff0000
    #define amask 0xff000000

#endif

#include <stdio.h>

#define CHANNEL_MASKS rmask, gmask, bmask, amask

void (APIENTRY * glGenBuffers)(GLsizei, GLuint*) = NULL;
void (APIENTRY * glDeleteBuffers)(GLsizei, GLuint*) = NULL;
void (APIENTRY * glBindBuffer)(GLenum,  GLuint) = NULL;
void (APIENTRY * glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum) = NULL;
void (APIENTRY * glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *) = NULL;
void (APIENTRY * glCopyImageSubData)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = NULL;
GLenum (APIENTRY * glCreateShader)(GLenum) = NULL;
void (APIENTRY * glDeleteShader)(GLenum) = NULL;
void (APIENTRY * glShaderSource)(GLenum, GLint, const GLchar **, const GLint *) = NULL;
void (APIENTRY * glGetShaderiv)(GLuint, GLenum, GLint*) = NULL;
void (APIENTRY * glCompileShader)(GLenum) = NULL;
GLenum (APIENTRY * glCreateProgram)(void) = NULL;
void (APIENTRY * glUseProgram)(GLenum) = NULL;
void (APIENTRY * glAttachShader)(GLenum, GLenum) = NULL;
void (APIENTRY * glLinkProgram)(GLenum) = NULL;
void (APIENTRY * glGetProgramiv)(GLuint, GLenum, GLint*) = NULL;
GLboolean (APIENTRY * glIsShader)(GLuint) = NULL;
void (APIENTRY * glGetShaderInfoLog)(GLuint,  GLsizei,  GLsizei *,  GLchar *) = NULL;
void (APIENTRY * glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = NULL;
void (APIENTRY * glDeleteProgram)(GLuint) = NULL;
GLint(APIENTRY * glGetUniformLocation)(GLuint program, const GLchar *name) = NULL;
void (APIENTRY * glProgramUniform1f)(GLuint program, GLint location, GLfloat v0) = NULL;
void (APIENTRY * glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (APIENTRY * glGenerateMipmap)(GLenum) = NULL;

void (APIENTRY * glGenVertexArrays)(GLsizei, GLuint*) = NULL;
void (APIENTRY * glDeleteVertexArrays)(GLsizei, GLuint*) = NULL;
void (APIENTRY * glBindVertexArray)(GLuint) = NULL;
GLint (APIENTRY * glGetAttribLocation)(GLuint, const GLchar *) = NULL;
GLint (APIENTRY * glVertexAttribPointer)(GLuint name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *data) = NULL;


#ifdef _WIN32
void (APIENTRY * glBlendEquation)(GLenum) = NULL;
#endif
void APIENTRY TS_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
	    GLint srcX, GLint srcY, GLint srcZ,
	    GLuint dstName, GLenum dstTarget, GLint dstLevel,
	    GLint dstX, GLint dstY, GLint dstZ,
	    GLsizei width, GLsizei height, GLsizei depth);
#ifdef __linux__
#define CHECK_FOR_PROCESS(NAME){\
        if(SDL_GL_GetProcAddress(NAME)==NULL){\
        fprintf(stderr, "[FJ-GL] Init Error: " NAME " is not present in OpenGL library.\n");\
        exit(1);\
    }\
}
#define CHECK_FOR_PROCESS_NON_FATAL(NAME){\
        if(SDL_GL_GetProcAddress(NAME)==NULL){\
        fprintf(stderr, "[FJ-GL] Init Error: " NAME " is not present in OpenGL library.\n");\
    }\
}
#else
#define CHECK_FOR_PROCESS(NAME){\
        if(wglGetProcAddress(NAME)==NULL){\
        fprintf(stderr, "[FJ-GL] Init Error: " NAME " is not present in OpenGL library.\n");\
        exit(1);\
    }\
}

#define CHECK_FOR_PROCESS_NON_FATAL(NAME){\
        if(wglGetProcAddress(NAME)==NULL){\
        fprintf(stderr, "[FJ-GL] Init Error: " NAME " is not present in OpenGL library.\n");\
    }\
}

#endif


#ifdef __linux__
#define GET_GL_FUNCTION(NAME, TYPING)\
CHECK_FOR_PROCESS( #NAME );\
NAME = TYPING SDL_GL_GetProcAddress( #NAME )
#define GET_GL_FUNCTION_NON_FATAL(NAME, TYPING)\
CHECK_FOR_PROCESS_NON_FATAL( #NAME );\
NAME = TYPING SDL_GL_GetProcAddress( #NAME )
#else
#define GET_GL_FUNCTION(NAME, TYPING)\
CHECK_FOR_PROCESS( #NAME );\
NAME = TYPING wglGetProcAddress( #NAME )
#define GET_GL_FUNCTION_NON_FATAL(NAME, TYPING)\
CHECK_FOR_PROCESS_NON_FATAL( #NAME );\
NAME = TYPING wglGetProcAddress( #NAME )

#endif


void LoadGLFunctions(void){
    static int first = 0;

    if(first)
        return;
    first = 1;

    GET_GL_FUNCTION(glGenBuffers,               (void (APIENTRY *)(GLsizei, GLuint*)));
    GET_GL_FUNCTION(glDeleteBuffers,            (void (APIENTRY *)(GLsizei, GLuint*)));
    GET_GL_FUNCTION(glBindBuffer,               (void (APIENTRY *)(GLenum, GLuint)));
    GET_GL_FUNCTION(glBufferData,               (void (APIENTRY *)(GLenum, GLsizeiptr, const GLvoid *, GLenum)));
    GET_GL_FUNCTION(glBufferSubData,            (void (APIENTRY *)(GLenum, GLintptr, GLsizeiptr, const GLvoid *)));
    GET_GL_FUNCTION(glCreateShader,           (GLenum (APIENTRY *)(GLenum)));
    GET_GL_FUNCTION(glDeleteShader,             (void (APIENTRY *)(GLenum)));
    GET_GL_FUNCTION(glShaderSource,             (void (APIENTRY *)(GLuint, GLsizei, const GLchar **, const GLint *)));
    GET_GL_FUNCTION(glGetShaderiv,              (void (APIENTRY *)(GLuint, GLenum, GLint *)));
    GET_GL_FUNCTION(glCompileShader,            (void (APIENTRY *)(GLenum)));
    GET_GL_FUNCTION(glCreateProgram,          (GLenum (APIENTRY *)(void)));
    GET_GL_FUNCTION(glUseProgram,               (void (APIENTRY *)(GLenum)));
    GET_GL_FUNCTION(glAttachShader,             (void (APIENTRY *)(GLenum,  GLenum)));
    GET_GL_FUNCTION(glLinkProgram,              (void (APIENTRY *)(GLenum)));
    GET_GL_FUNCTION(glGetProgramiv,             (void (APIENTRY *)(GLuint, GLenum, GLint*)));
    GET_GL_FUNCTION(glIsShader,            (GLboolean (APIENTRY *)(GLuint)));
    GET_GL_FUNCTION(glGetShaderInfoLog,         (void (APIENTRY *)(GLuint,  GLsizei,  GLsizei *,  GLchar *)));
    GET_GL_FUNCTION(glGetProgramInfoLog,        (void (APIENTRY *)(GLuint, GLsizei, GLsizei*, GLchar*)));
    GET_GL_FUNCTION(glDeleteProgram,            (void (APIENTRY *)(GLuint)));
    GET_GL_FUNCTION(glGetUniformLocation,      (GLint (APIENTRY *)(GLuint, const GLchar *)));
    GET_GL_FUNCTION(glProgramUniform1f,         (void (APIENTRY *)(GLuint program, GLint location, GLfloat v0)));
    GET_GL_FUNCTION(glBlendFuncSeparate,        (void (APIENTRY *)(GLenum, GLenum, GLenum, GLenum)));
    GET_GL_FUNCTION(glGenerateMipmap,           (void (APIENTRY *)(GLenum)));

    GET_GL_FUNCTION_NON_FATAL(glGenVertexArrays,          (void (APIENTRY *)(GLsizei, GLuint*)));
    GET_GL_FUNCTION_NON_FATAL(glDeleteVertexArrays,       (void (APIENTRY *)(GLsizei, GLuint*)));
    GET_GL_FUNCTION_NON_FATAL(glBindVertexArray,          (void (APIENTRY *)(GLuint)));
    GET_GL_FUNCTION_NON_FATAL(glGetAttribLocation,       (GLint (APIENTRY *)(GLuint, GLchar*)));
    GET_GL_FUNCTION_NON_FATAL(glVertexAttribPointer,     (GLint (APIENTRY *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)));


    if(!(glGenVertexArrays&&glDeleteVertexArrays&&glBindVertexArray&&glGetAttribLocation))
        configl.hasVertexArrays = 0;
    else
        configl.hasVertexArrays = 1;

#ifdef __linux__
    if(SDL_GL_GetProcAddress("glCopyImageSubData")!=NULL){
        glCopyImageSubData = (void(APIENTRY *)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei)) SDL_GL_GetProcAddress("glCopyImageSubData");
    }
    else
    if(SDL_GL_GetProcAddress("glCopyImageSubDataNV")!=NULL){
        glCopyImageSubData = (void(APIENTRY *)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei)) SDL_GL_GetProcAddress("glCopyImageSubDataNV");
        fprintf(stderr, "[FJ-GL] Init Warning: glCopyImageSubData is not present in OpenGL library. Using deprecated hardware fallback.\n");
    }
    else{
        glCopyImageSubData = TS_CopyImageSubData;
        fprintf(stderr, "[FJ-GL] Init Warning: glCopyImageSubData is not available, and hardware fallbacks are not available. Emulating in software.\n");
    }
#else
    GET_GL_FUNCTION(glBlendEquation,        (void (APIENTRY *)(GLenum)));
#endif

}
