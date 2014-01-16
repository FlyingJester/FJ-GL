#include "api.h"
#include "glExtra.h"
#include "circle.h"
#include "shader.h"
#include "shader.h"
#include <stdio.h>

#define MAX_STACK_STEAL 0x80

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <SDL/SDL_syswm.h>

unsigned int Scale = 1;
unsigned int Width = 320;
unsigned int Height= 240;

int ClipRectX, ClipRectY, ClipRectW, ClipRectH;

GLuint EmptyTexture = 0;

GLuint TexCoordBuffer = 0;
GLuint FullColorBuffer = 0;
GLuint SeriousCoordBuffer = 0;
Display *display;
Window window;
GLXContext glcontext;

void *ScreenCopy = NULL;

BlendMode currentBlendMode = bmBlend;

void SetBlendMode(int blendmode){
    if(blendmode==bmBlend)
        return;

    currentBlendMode = blendmode;

    switch(blendmode){
    case bmReplace:
        glDisable(GL_BLEND);
        break;
    case bmSubtract:
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    case bmAdd:
        glBlendFunc(GL_ONE, GL_ONE);
        break;
    case bmMultiply:
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
    }
}

void EndBlendMode(){

    if(currentBlendMode==bmBlend)
        return;



    switch(currentBlendMode){
    case bmReplace:
        glEnable(GL_BLEND);
        goto end;
    case bmSubtract:
        glBlendEquation(GL_FUNC_ADD);
    case bmMultiply:
    case bmAdd:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        goto end;
    }

end:

    currentBlendMode = bmBlend;

}

void ResetOrtho(void){

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    float scaleSize = Scale;

    if(scaleSize==0){
        //ideally just disable video altogether.
        scaleSize = 1;
    }
    glViewport(0, 0, Width*scaleSize, Height*scaleSize);
    glScissor(0, 0, Width*scaleSize, Height*scaleSize);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    glOrtho(0, Width, Height, 0, 1, -1);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

EXPORT(void GetDriverInfo(DriverInfo_t* driverinfo)){
    driverinfo->name = "FJ-GL";
    driverinfo->author = "Martin McDonough";
    driverinfo->date = __DATE__;
    driverinfo->version = "0.01";
    driverinfo->description = "A more modern OpenGL graphics backend for Sphere 1.5 and 1.6";

}

bool InternalInitVideo(int w, int h){

    Width = w;
    Height = h;

    Scale = 1;

    ClipRectX = 0;
    ClipRectY = 0;
    ClipRectW = w;
    ClipRectH = h;

    ScreenCopy = realloc(ScreenCopy, 4*w*h);

    if(SDL_WasInit(SDL_INIT_EVERYTHING)==0){
        fprintf(stderr, "[FJ-GL] Warning: SDL2 was not initialized.\n\tError: %s\n", SDL_GetError());
        SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
    }

    if(SDL_WasInit(SDL_INIT_VIDEO)==0){
        SDL_InitSubSystem(SDL_INIT_VIDEO);
        fprintf(stderr, "[FJ-GL] Warning: SDL2 was not initialized.\n\tError: %s\n", SDL_GetError());
    }

    if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)==0){
        fprintf(stderr, "[FJ-GL] Error: Could not initialize SDL2.\n\tError: %s\n", SDL_GetError());
        return false;
    }

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
    //SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    //SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    //SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    const char *title = strcpy(malloc(100), "Sphere RPG Engine");

    SDL_ShowCursor(SDL_DISABLE);


    void * s = SDL_SetVideoMode(Width*Scale, Height*Scale, 32, SDL_OPENGL);

    LoadGLFunctions();

    CurrentShader = LoadEmbeddedShader();
    DefaultShader = CurrentShader;
    glUseProgram(CurrentShader);

    GLint ScreenWidth = glGetUniformLocation(CurrentShader, "ScreenWidth");
    GLint ScreenHeight = glGetUniformLocation(CurrentShader, "ScreenHeight");
    if(ScreenWidth>=0){
        float ScreenWidthVal = (float)Width;
        glProgramUniform1f(CurrentShader, ScreenWidth, ScreenWidthVal);
    }

    if(ScreenHeight>=0){
        float ScreenHeightVal = (float)Height;
        glProgramUniform1f(CurrentShader, ScreenHeight, ScreenHeightVal);
    }

    if(s==NULL){
        fprintf(stderr, "[FJ-GL] Error: Could not open window.\n\tError: %s\n", SDL_GetError());
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    ResetOrtho();
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SDL_ShowCursor(0);

    uint32_t white = 0xFFFFFFFF;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &EmptyTexture);
    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    GLfloat texcoords[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    glGenBuffers(1, &TexCoordBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*8, texcoords, GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    GLuint fullcolor[] = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu};

    glGenBuffers(1, &FullColorBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, FullColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint)*4, fullcolor, GL_STATIC_DRAW);

    GLfloat stexcoords[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

    glGenBuffers(1, &SeriousCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, SeriousCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint)*16, stexcoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);



    return true;

}

EXPORT(void CloseVideoDriver(void)){

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteBuffers(1, &TexCoordBuffer);
    glDeleteBuffers(1, &FullColorBuffer);
    glDeleteTextures(1, &EmptyTexture);

    //SDL_GL_DeleteContext(glcontext);
    //SDL_DestroyWindow(screen);

//	SDL_VideoQuit();

    SDL_Quit();
    free(ScreenCopy);

}
EXPORT(bool ToggleFullscreen(void)){
    return true;
}

EXPORT(void FlipScreen(void)){
    //glXSwapBuffers(display, window);
    SDL_GL_SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT);
}
EXPORT(void SetClippingRectangle(int x, int y, int w, int h)){

    if(w>Width)
        w=Width;
    if(h>Height)
        w=Width;
    if(x<0)
        x=0;
    if(y<0)
        y=0;

    if((x==ClipRectX)&&(y==ClipRectY)&&(w==ClipRectW)&&(h==ClipRectH))
        return;

    ClipRectX = x;
    ClipRectY = y;
    ClipRectW = w;
    ClipRectH = h;

    glScissor(x, Height-h-y, w, h);

}
EXPORT(void GetClippingRectangle(int* x, int* y, int* w, int* h)){

    *x = ClipRectX;
    *y = ClipRectY;
    *w = ClipRectW;
    *h = ClipRectH;

}

EXPORT(IMAGE * CreateImage(int width, int height, RGBA* pixels)){

    RGBA *newpixels = malloc(width*height*4);


    for(int i = 0; i<height; ++i){
        memcpy(newpixels+(i*width), pixels+(i*width), width*4);
    }
//    memcpy(newpixels, pixels, width*height*4);

    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    IMAGE *im = malloc(sizeof(IMAGE));
    im->pixels = newpixels;
    im->texture = texture;
    im->w = width;
    im->h = height;

    return im;

}

EXPORT(IMAGE * CloneImage(IMAGE * image)){
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glCopyImageSubData(image->texture, GL_TEXTURE_2D, 0, 0, 0, 0, texture, GL_TEXTURE_2D, 0, 0, 0, 0, image->w, image->h, 1);

    IMAGE *im = malloc(sizeof(IMAGE));
    im->pixels = NULL;
    im->texture = texture;
    im->w = image->w;
    im->h = image->h;

    return im;
}

EXPORT(IMAGE * GrabImage(IMAGE * image, int x, int y, int width, int height)){
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ScreenCopy);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ScreenCopy);

    IMAGE *im = malloc(sizeof(IMAGE));
    im->pixels = NULL;
    im->texture = texture;
    im->w = width;
    im->h = height;

    return im;

}

EXPORT(void DestroyImage(IMAGE * image)){
    if(image->pixels)
        free(image->pixels);
    glDeleteTextures(1, &(image->texture));
    free(image);
}

EXPORT(void BlitImage(IMAGE * image, int x, int y, BlendMode blendmode)){

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);

    glBindBuffer(GL_ARRAY_BUFFER, FullColorBuffer);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLint vertex[] = {x, y, x+image->w, y, x+image->w, y+image->h, x, y+image->h};

    glVertexPointer(2, GL_INT, 0, vertex);



    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}


EXPORT(void BlitImageMask(IMAGE * image, int x, int y, BlendMode blendmode, RGBA mask, BlendMode mask_blendmode)){

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);

    GLuint color[] = {mask, mask, mask, mask};

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLint vertex[] = {x, y, x+image->w, y, x+image->w, y+image->h, x, y+image->h};

    glVertexPointer(2, GL_INT, 0, vertex);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}

EXPORT(void TransformBlitImage(IMAGE * image, int x[4], int y[4], BlendMode blendmode)){

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);

    glBindBuffer(GL_ARRAY_BUFFER, FullColorBuffer);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLint vertex[] = {x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3]};

    glVertexPointer(2, GL_INT, 0, vertex);



    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}
EXPORT(void TransformBlitImageMask(IMAGE * image, int x[4], int y[4], BlendMode blendmode, RGBA mask, BlendMode mask_blendmode)){

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);

    GLuint color[] = {mask, mask, mask, mask};

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLint vertex[] = {x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3]};

    glVertexPointer(2, GL_INT, 0, vertex);



    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}
EXPORT(int GetImageWidth(IMAGE * image)){
    return image->w;
}
EXPORT(int GetImageHeight(IMAGE * image)){
    return image->h;
}
EXPORT(RGBA* LockImage(IMAGE * image)){
    if(image->pixels)
        goto fin;

    image->pixels = malloc(image->w*image->h*4);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

fin:
    return image->pixels;

}

EXPORT(void UnlockImage(IMAGE * image)){
    glBindTexture(GL_TEXTURE_2D, image->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
}

EXPORT(void DirectBlit(int x, int y, int w, int h, RGBA* pixels)){

    IMAGE image;

    glGenTextures(1, &(image.texture));

    glBindTexture(GL_TEXTURE_2D, image.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    image.pixels = NULL;
    image.w = w;
    image.h = h;

    BlitImage(&image, x, y, 0);

    glDeleteTextures(1, &(image.texture));

}

EXPORT(void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)){

    IMAGE image;

    glGenTextures(1, &(image.texture));

    glBindTexture(GL_TEXTURE_2D, image.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    image.pixels = NULL;
    image.w = w;
    image.h = h;

    //IMAGE *image = CreateImage(w, h, pixels);
    TransformBlitImage(&image, x, y, 0);
    glDeleteTextures(1, &(image.texture));
    //DestroyImage(image);
}

EXPORT(void DirectGrab(int x, int y, int w, int h, RGBA* pixels)){
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ScreenCopy);


    for(int i = 0; i<h; ++i){
        memcpy(pixels+((i*w)<<2), ScreenCopy+((h-i)*w<<2), w<<2);
    }

}

EXPORT(void DrawPoint(int x, int y, RGBA color)){

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, &color);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLint vertex[] = {x, y};

    glVertexPointer(2, GL_INT, 0, vertex);



    glDrawArrays(GL_POINTS, 0, 1);



}

EXPORT(void DrawPointSeries(int** points, int length, RGBA color)){

    if(!length)
        return;

    if(length<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;
    GLint *vertex = NULL;

inheap:;

    colors = malloc(4*length);

    vertex = malloc(8*length);

    goto fill;

onstack:;


    colors = alloca(4*length);

    vertex = alloca(8*length);

fill:;

    GLint *texCoords = calloc(8, length);


    for(int i = 0; i<length; ++i){
        colors[i] = color;
        vertex[(i*2)  ] = points[i][0];
        vertex[(i*2)+1] = points[i][1];
    }
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);
    glBindTexture(GL_TEXTURE_2D, EmptyTexture);




    glDrawArrays(GL_POINTS, 0, length);



    free(texCoords);

    if(length<MAX_STACK_STEAL)
        return;

    free(colors);
    free(vertex);


}

EXPORT(void DrawLine(int x[2], int y[2], RGBA color)){

    GLint colors[] = {color, color};
    DrawGradientLine(x, y, colors);
}

EXPORT(void DrawGradientLine(int x[2], int y[2], RGBA colors[2])){

    GLint vertex[] = {x[0], y[0], x[1], y[1]};

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexPointer(2, GL_INT, 0, vertex);

    glDrawArrays(GL_LINES, 0, 2);
}

EXPORT(void DrawLineSeries(int** points, int length, RGBA color, int type)){

    if(!length)
        return;

    if(length<MAX_STACK_STEAL)
        goto onstack;

    GLenum gltype = GL_LINES;

    switch (type){
    case 1:
        gltype = GL_LINE_STRIP;
        break;
    case 2:
        gltype = GL_LINE_LOOP;
    }

    RGBA  *colors = NULL;
    GLint *vertex = NULL;

inheap:;

    colors = malloc(4*length);

    vertex = malloc(8*length);

    goto fill;

onstack:;


    colors = alloca(4*length);

    vertex = alloca(8*length);

fill:;

    GLint *texCoords = calloc(8, length);


    for(int i = 0; i<length; ++i){
        colors[i] = color;
        vertex[(i*2)  ] = points[i][0];
        vertex[(i*2)+1] = points[i][1];
    }
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(gltype, 0, length);




    free(texCoords);

    if(length<MAX_STACK_STEAL)
        return;

    free(colors);
    free(vertex);
}

EXPORT(void DrawTriangle(int x[3], int y[3], RGBA color)){

    GLint colors[] = {color, color, color};

    DrawGradientTriangle(x, y, colors);
}

EXPORT(void DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])){
    //if(!(Colored(colors[0])|Colored(colors[1])|Colored(colors[2])))
    //    return;
    GLint vertex[] = {x[0], y[0], x[1], y[1], x[2], y[2]};

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);


}
EXPORT(void DrawPolygon(int** points, int length, int invert, RGBA color)){

    if(!length)
        return;

    if(length<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;
    GLuint *vertex = NULL;

inheap:;

    colors = malloc(4*length);

    vertex = malloc(8*length);

    goto fill;

onstack:;


    colors = alloca(4*length);

    vertex = alloca(8*length);

fill:;

    GLint *texCoords = calloc(8, length);


    for(int i = 0; i<length; ++i){
        colors[i] = color;
        vertex[(i*2)  ] = points[i][0];
        vertex[(i*2)+1] = points[i][1];
    }
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_TRIANGLE_STRIP, 0, length);




    free(texCoords);

    if(length<MAX_STACK_STEAL)
        return;

    free(colors);
    free(vertex);

}

EXPORT(void DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])){

    GLint vertex[] = {x, y, x+w, y, x+w, y+h, x, y+h};

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexPointer(2, GL_INT, 0, vertex);



    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


}

EXPORT(void DrawRectangle(int x, int y, int w, int h, RGBA color)){

    GLuint colors[] = {color, color, color, color};

    DrawGradientRectangle(x, y, w, h, colors);
}



EXPORT(void DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color)){

    GLint vertex[] = {
        x, y,
        x+size, y+size,
        x, y+h,
        x+size, y+h-size,
        x+w, y+h,
        x+w-size, y+h-size,
        x+w, y,
        x+w-size, y+size
    };

    GLuint colors[] = {color, color, color, color, color, color, color, color};

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glBindBuffer(GL_ARRAY_BUFFER, SeriousCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexPointer(2, GL_INT, 0, vertex);



    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);


}

EXPORT(void DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color)){

    const unsigned int step = M_PI*(float)((rx>ry)?rx:ry);

    if(step<5){
        DrawPoint(x, y, color);
        return;
    }

    GLint * vertex = ApproximateEllipseGL(x, y, rx, ry, step);

    if(step<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;

inheap:;

    colors = malloc(step<<2);

    goto fill;

onstack:;

    colors = alloca(step<<2);

fill:;

    GLint *texCoords = calloc(8, step);


    for(unsigned int i = 0; i<step; i++){
        colors[i] = color;
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_LINE_LOOP, 0, step);



    FreeEllipsePointsGL(vertex);

    free(texCoords);

    if(step<MAX_STACK_STEAL)
        return;

    free(colors);

}
EXPORT(void DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color)){

    const unsigned int step = M_PI*(float)((rx>ry)?rx:ry);

    if(step<5){
        DrawPoint(x, y, color);
        return;
    }

    GLint * vertex = ApproximateEllipseGL(x, y, rx, ry, step);

    if(step<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;

inheap:;

    colors = malloc(step<<2);

    goto fill;

onstack:;

    colors = alloca(step<<2);

fill:;

    GLint *texCoords = calloc(8, step);

    for(unsigned int i = 0; i<step; i++){
        colors[i] = color;
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_TRIANGLE_FAN, 0, step);



    FreeEllipsePointsGL(vertex);

    free(texCoords);

    if(step<MAX_STACK_STEAL)
        return;

    free(colors);
}

EXPORT(void DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias)){

    if(r<3){
        DrawPoint(x, y, color);
        return;
    }

    const unsigned int step = M_PI*((float)r);

    GLint * vertex = ApproximateCircleGL(x, y, r, step);

    if(step<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;

inheap:;

    colors = malloc(step<<2);

    goto fill;

onstack:;

    colors = alloca(step<<2);

fill:;

    GLint *texCoords = calloc(8, step);


    for(unsigned int i = 0; i<step; i++){
        colors[i] = color;
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_LINE_LOOP, 0, step);



    FreeEllipsePointsGL(vertex);

    free(texCoords);

    if(step<MAX_STACK_STEAL)
        return;

    free(colors);
}
EXPORT(void DrawFilledCircle(int x, int y, int r, RGBA color, int antialias)){

    if(r<3){
        DrawPoint(x, y, color);
        return;
    }

    const unsigned int step = M_PI*((float)r);

    GLint * vertex = ApproximateCircleGL(x, y, r, step);

    if(step<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;

inheap:;

    colors = malloc(step<<2);
    goto fill;

onstack:;

    colors = alloca(step<<2);

fill:;

    GLint *texCoords = calloc(8, step);

    for(unsigned int i = 0; i<step; i++){
        colors[i] = color;
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_TRIANGLE_FAN, 0, step);



    FreeEllipsePointsGL(vertex);

    free(texCoords);

    if(step<MAX_STACK_STEAL)
        return;

    free(colors);
}
EXPORT(void DrawGradientCircle(int x, int y, int r, RGBA color[2], int antialias)){

    if(r<3){
        DrawPoint(x, y, *color);
        return;
    }

    unsigned int step = M_PI*((float)r);

    GLint * vertex = ApproximateCircleGL(x, y, r, step);

    step++;

    vertex[0] = x;
    vertex[1] = y;

    if(step<MAX_STACK_STEAL)
        goto onstack;

    RGBA  *colors = NULL;

inheap:;

    colors = malloc(step<<2);

    goto fill;

onstack:;

    colors = alloca(step<<2);

fill:;

    GLint *texCoords = calloc(8, step);

    *colors = color[1];

    for(unsigned int i = 1; i<step; i++){
        colors[i] = *color;
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glTexCoordPointer(2, GL_INT, 0, texCoords);
    glVertexPointer(2, GL_INT, 0, vertex);

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_TRIANGLE_FAN, 0, step);



    FreeEllipsePointsGL(vertex);

    free(texCoords);

    if(step<MAX_STACK_STEAL)
        return;

    free(colors);
}

/////
// Unimplemented:

EXPORT(void DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)){
    return;
}

EXPORT(void DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias)){
    return;
}
EXPORT(void DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2])){
    return;
}
EXPORT(void DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3])){
    return;
}
