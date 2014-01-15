#include "glExtra.h"
#define GL_GLEXT_PROTOTYPES 1
#include <SDL/SDL_opengl.h>
#include <SDL/SDL.h>
#include <stdio.h>

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

#define CHANNEL_MASKS rmask, gmask, bmask, amask

void (APIENTRY * glGenBuffers)(GLsizei, GLuint*) = NULL;
void (APIENTRY * glDeleteBuffers)(GLsizei, GLuint*) = NULL;
void (APIENTRY * glBindBuffer)(GLenum,  GLuint) = NULL;
void (APIENTRY * glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum) = NULL;
void (APIENTRY * glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *) = NULL;
void (APIENTRY * glCopyImageSubData)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = NULL;


void TS_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
	    GLint srcX, GLint srcY, GLint srcZ,
	    GLuint dstName, GLenum dstTarget, GLint dstLevel,
	    GLint dstX, GLint dstY, GLint dstZ,
	    GLsizei width, GLsizei height, GLsizei depth);
#define CHECK_FOR_PROCESS(NAME){\
        if(SDL_GL_GetProcAddress(NAME)==NULL){\
        fprintf(stderr, "[FJ-GL] Init Error: " NAME " is not present in OpenGL library.\n");\
        exit(1);\
    }\
}

#define GET_GL_FUNCTION(NAME, TYPING)\
CHECK_FOR_PROCESS( #NAME );\
NAME = TYPING SDL_GL_GetProcAddress( #NAME )

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
}


void TS_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
	    GLint srcX, GLint srcY, GLint srcZ,
	    GLuint dstName, GLenum dstTarget, GLint dstLevel,
	    GLint dstX, GLint dstY, GLint dstZ,
	    GLsizei width, GLsizei height, GLsizei depth){

    int srcwidth, srcheight, dstwidth, dstheight;
    glBindTexture(GL_TEXTURE_2D, dstName);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &dstwidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &dstheight);
    glBindTexture(GL_TEXTURE_2D, srcName);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &srcwidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &srcheight);

    if(!((srcwidth==width)&&(srcheight==height)&&(srcheight==dstheight)&&(srcwidth==srcheight))){

        //Make the surfaces to hold the source and destinations.
        SDL_Surface *srcsurface = SDL_CreateRGBSurface(0, srcwidth, srcheight, 32, CHANNEL_MASKS);

        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcsurface->pixels);

        //Fill the dst surface with the current pixels of the destination texture.
        glBindTexture(GL_TEXTURE_2D, dstName);

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &dstwidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &dstheight);

        SDL_Surface *dstsurface = SDL_CreateRGBSurface(0, dstwidth, dstheight, 32, CHANNEL_MASKS);

        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, dstsurface->pixels);

        //Copy the pixels needed for the new sub data.

        SDL_Rect srcrect = {srcX, srcY, width, height};
        SDL_Rect dstrect = {dstX, dstY, width, height};

        SDL_BlitSurface(srcsurface, &srcrect, dstsurface, &dstrect);


        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dstwidth, dstheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dstsurface->pixels);

        SDL_FreeSurface(dstsurface);
        SDL_FreeSurface(srcsurface);
    }
    else{
        void * pixels = malloc(width*height);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, dstName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        free(pixels);
    }
}
