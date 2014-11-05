
#include <GL/gl.h>
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

#define CHANNEL_MASKS rmask, gmask, bmask, amask

#ifndef _WIN32
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
#else

void APIENTRY TS_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
	    GLint srcX, GLint srcY, GLint srcZ,
	    GLuint dstName, GLenum dstTarget, GLint dstLevel,
	    GLint dstX, GLint dstY, GLint dstZ,
	    GLsizei width, GLsizei height, GLsizei depth){

	void * pixels = malloc(width*height);

    int srcwidth, srcheight, dstwidth, dstheight;
    glBindTexture(GL_TEXTURE_2D, dstName);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &dstwidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &dstheight);
    glBindTexture(GL_TEXTURE_2D, srcName);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &srcwidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &srcheight);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, dstName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    free(pixels);

}
#endif
