#include "api.h"
#include "glExtra.h"
#include "circle.h"
#include "shader.h"
#include "shader.h"
#include <stdio.h>
#include "config.h"

#define MAX_STACK_STEAL 0x80
#ifdef _WIN32
#include <Windows.h>
#include <Wingdi.h>
#include <malloc.h>
#define alloca _alloca
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#endif
#include <GL/gl.h>
#ifdef __linux__
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <SDL/SDL_syswm.h>
#endif

static float SPI = M_PI;

GLuint CurrentShader = 0;
GLuint DefaultShader = 0;

unsigned int Scale = 1;
unsigned int Width = 320;
unsigned int Height= 240;

int ClipRectX, ClipRectY, ClipRectW, ClipRectH;

GLuint EmptyTexture = 0;

GLuint TexCoordBuffer = 0;
GLuint ZeroTexCoordBuffer = 0;
GLuint FullColorBuffer = 0;
GLuint SeriousCoordBuffer = 0;

GLuint VertexAttrib = 0;
GLuint ColorAttrib = 0;
GLuint TexCoordAttrib = 0;

#ifdef __linux__
Display *display;
Window window;
GLXContext glcontext;
#elif defined(_WIN32)
HDC dc;
HGLRC glcontext;
#endif


EXPORT(unsigned int GetVertexAttrib(void)){
    return VertexAttrib;
}
EXPORT(unsigned int GetColorAttrib(void)){
    return ColorAttrib;
}
EXPORT(unsigned int GetTexCoordAttrib(void)){
    return TexCoordAttrib;
}

inline bool NotVisible(int x, int y, int w, int h){

    if(x+w<ClipRectX)
        return true;
    if(y+h<ClipRectY)
        return true;
    if(x>ClipRectX+ClipRectW)
        return true;
    if(y>ClipRectY+ClipRectH)
        return true;

    return false;

}

uint32_t *ScreenCopy = NULL;

BlendMode currentBlendMode = bmBlend;

inline void SetBlendMode(int blendmode){
    if(blendmode==bmBlend)
        return;

    currentBlendMode = blendmode;

    switch(blendmode){
    case bmReplace:
        glDisable(GL_BLEND);
        return;
    case bmSubtract:
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    case bmAdd:
        glBlendFunc(GL_ONE, GL_ONE);
        return;
    case bmMultiply:
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        return;
    case bmRGB:
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        return;
    case bmAlpha:
        glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return;
    case bmInvert:
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
    }
}

inline void EndBlendMode(){

    if(currentBlendMode==bmBlend)
        return;

    switch(currentBlendMode){
    case bmReplace:
        glEnable(GL_BLEND);
        goto end;
    case bmSubtract:
        glBlendEquation(GL_FUNC_ADD);
    case bmRGB:
    case bmAlpha:
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    case bmInvert:
    case bmMultiply:
    case bmAdd:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

end:

    currentBlendMode = bmBlend;

}

void CreateAveragedData(int x, int y, int w, int h, char *indata, char *outdata){
    char *pixels = malloc((w*h)<<2);
	int i;
	int e;

    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    for(i = 0; i<w; ++i){
        for(e = 0; e<h; ++e){
            size_t offset = i+(e*w);
            offset<<=2;
            outdata[offset] = ((pixels[offset])>>1) + ((indata[offset])>>1);
            offset++;
            outdata[offset] = ((pixels[offset])>>1) + ((indata[offset])>>1);
            offset++;
            outdata[offset] = ((pixels[offset])>>1) + ((indata[offset])>>1);
            offset++;
            outdata[offset] = ((pixels[offset])>>1) + ((indata[offset])>>1);

        }
    }

    free(pixels);

}

void ResetOrtho(void){

    float scaleSize = (float)Scale;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    scaleSize = configl.scale;

    if(scaleSize==0){
        //ideally just disable video altogether.
        scaleSize = 1;
    }
    glViewport(0, 0, Width*(int)scaleSize, Height*(int)scaleSize);
    glScissor(0, 0, Width*(int)scaleSize, Height*(int)scaleSize);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    glOrtho(0, Width, Height, 0, 1, -1);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

EXPORT(void STDCALL GetDriverInfo(DriverInfo_t* driverinfo)){
    driverinfo->name = "FJ-GL";
    driverinfo->author = "Martin McDonough";
    driverinfo->date = __DATE__;
    driverinfo->version = "0.08";
    driverinfo->description = "A more modern OpenGL graphics backend for Sphere 1.5 and 1.6";

}

#ifdef _WIN32

EXPORT(void STDCALL ConfigureDriver(HWND parent)){
	return;
}
#endif

#ifdef __linux__
bool InternalInitVideo(int w, int h){
#elif defined (_WIN32)
EXPORT(bool STDCALL InitVideoDriver(HWND window, int w, int h)){

#endif
    GLint ScreenWidth;
    GLint ScreenHeight;
#ifdef _WIN32


	PIXELFORMATDESCRIPTOR fd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pf;

#endif
    uint32_t white = 0xFFFFFFFF;

    bool fullscreen = configl.fullscreen;

    if(!configl.niceCircles){
        printf("[FJ-GL] Using faster linear approximations.\n");
        SPI = M_PI/2.0f;
    }

    GLfloat texcoords[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

	GLfloat fullcolor[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    GLfloat stexcoords[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    GLfloat ztexcoords[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    Width = w;
    Height = h;

    Scale = configl.scale;

    ClipRectX = 0;
    ClipRectY = 0;
    ClipRectW = w*Scale;
    ClipRectH = h*Scale;

    SetClippingRectangle(0, 0, w*Scale, h*Scale);

    ScreenCopy = realloc(ScreenCopy, 4*w*h);
#ifdef __linux__
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
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,configl.vsync);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_ShowCursor(SDL_DISABLE);

    void * s = SDL_SetVideoMode(Width*Scale, Height*Scale, 32, SDL_OPENGL|((fullscreen)?SDL_FULLSCREEN:0));

    LoadGLFunctions();

    SDL_WM_SetCaption("Sphere RPG Engine", "Sphere");

#elif defined (_WIN32)

    SetWindowPos(window, HWND_TOPMOST, 0, 0, Width*Scale, Height*Scale, SWP_SHOWWINDOW);

	dc = GetDC(window);

	pf = ChoosePixelFormat(dc, &fd);

	SetPixelFormat(dc, pf, &fd);

	glcontext = wglCreateContext(dc);

	wglMakeCurrent(dc, glcontext);

#endif

    CurrentShader = LoadEmbeddedShader();
    DefaultShader = CurrentShader;
    glUseProgram(CurrentShader);

    ScreenWidth = glGetUniformLocation(CurrentShader, "ScreenWidth");
    ScreenHeight = glGetUniformLocation(CurrentShader, "ScreenHeight");
    if(ScreenWidth>=0){
        float ScreenWidthVal = (float)Width*(float)Scale;
        glProgramUniform1f(CurrentShader, ScreenWidth, ScreenWidthVal);
    }
    else{
        fprintf(stderr, "[FJ-GL] Error: Could not bind ScreenWidth to Shader.\n");
    }
    if(ScreenHeight>=0){
        float ScreenHeightVal = (float)Height*(float)Scale;
        glProgramUniform1f(CurrentShader, ScreenHeight, ScreenHeightVal);
    }
    else{
        fprintf(stderr, "[FJ-GL] Error: Could not bind ScreenHeight to Shader.\n");
    }

#ifdef __linux__

    if(s==NULL){
        fprintf(stderr, "[FJ-GL] Error: Could not open window.\n\tError: %s\n", SDL_GetError());
    }

#endif

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    ResetOrtho();
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef __linux__
	SDL_ShowCursor(0);
#endif

    if(configl.hasVertexArrays){
        VertexAttrib    = glGetAttribLocation(CurrentShader, "Vertex");
        ColorAttrib     = glGetAttribLocation(CurrentShader, "Color");
        TexCoordAttrib  = glGetAttribLocation(CurrentShader, "TexCoord");
        printf("[FJ-GL] Using Vertex Arrays.\n");
    }

    if(configl.niceImages){
        printf("[FJ-GL] Using nicest video settings.\n");
        glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST); //Unused in FJ-GL, but included for completeness because it does no harm.
    }
    else {
        printf("[FJ-GL] Using fastest video settings.\n");
        glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_FASTEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST); //Unused in FJ-GL, but included for completeness because it does no harm.
    }

    glDepthMask(GL_FALSE);
    glDisable(GL_DITHER);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glGenTextures(1, &EmptyTexture);
    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glGenerateMipmap(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);


    glGenBuffers(1, &TexCoordBuffer);
    glAlphaFunc(GL_NOTEQUAL, 0.0f);
    glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*8, texcoords, GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //GLuint fullcolor[] = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu};




    glGenBuffers(1, &FullColorBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, FullColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)<<4, fullcolor, GL_STATIC_DRAW);

    glGenBuffers(1, &SeriousCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, SeriousCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint)<<4, stexcoords, GL_STATIC_DRAW);

    glGenBuffers(1, &ZeroTexCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ZeroTexCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint)<<3, ztexcoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);



    return true;

}

EXPORT(void STDCALL CloseVideoDriver(void)){

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteBuffers(1, &TexCoordBuffer);
    glDeleteBuffers(1, &FullColorBuffer);
    glDeleteTextures(1, &EmptyTexture);

    //SDL_GL_DeleteContext(glcontext);
    //SDL_DestroyWindow(screen);

//	SDL_VideoQuit();
#ifdef __linux__
    SDL_Quit();
#endif
    free(ScreenCopy);

}
EXPORT(bool STDCALL ToggleFullScreen(void)){
    return true;
}

EXPORT(void STDCALL FlipScreen(void)){
   // glFlush();
#ifdef __linux__
    SDL_GL_SwapBuffers();
#elif defined (_WIN32)
	SwapBuffers(dc);
#endif
    glClear(GL_COLOR_BUFFER_BIT);
}
EXPORT(void STDCALL SetClippingRectangle(int x, int y, int w, int h)){

    if((unsigned)w>Width)
        w=(int)Width;
    if((unsigned)h>Height)
        w=(int)Width;
    if((unsigned)x<0)
        x=0;
    if((unsigned)y<0)
        y=0;

    if((x==ClipRectX)&&(y==ClipRectY)&&(w==ClipRectW)&&(h==ClipRectH))
        return;

    ClipRectX = x;
    ClipRectY = y;
    ClipRectW = w;
    ClipRectH = h;

    glScissor(x, Height-h-y, w, h);

}
EXPORT(void STDCALL GetClippingRectangle(int* x, int* y, int* w, int* h)){

    *x = ClipRectX;
    *y = ClipRectY;
    *w = ClipRectW;
    *h = ClipRectH;

}


void CreateAveragedData(int x, int y, int w, int h, char *indata, char *outdata);


void BlitImageMaskAverage(IMAGE *image, int x, int y, RGBA mask, BlendMode mask_blendmode){
    RGBA *pixels = malloc((image->w*image->h)<<2);
    IMAGE *newimage = CreateImage(image->w, image->h, pixels);

    CreateAveragedData(x, y, image->w, image->h, (char *)LockImage(image), (char *)pixels);

    BlitImageMask(newimage, x, y, bmReplace, mask, mask_blendmode);
    DestroyImage(newimage);
}

void BlitImageAverage(IMAGE *image, int x, int y){
    BlitImageMaskAverage(image, x, y, 0xFFFFFFFFu, bmBlend);
}

EXPORT(void STDCALL BlitImage(IMAGE * image, int x, int y, BlendMode blendmode)){

    if(NotVisible(x, y, image->w, image->h))
        return;

    if(blendmode==bmAverage){
        BlitImageAverage(image, x, y);
        return;
    }

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);

    glBindBuffer(GL_ARRAY_BUFFER, FullColorBuffer);
    glColorPointer(4, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, image->TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		GLint vertex[] = {x, y, x+image->w, y, x+image->w, y+image->h, x, y+image->h};

		glVertexPointer(2, GL_INT, 0, vertex);
	}


    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}


EXPORT(void STDCALL BlitImageMask(IMAGE * image, int x, int y, BlendMode blendmode, RGBA mask, BlendMode mask_blendmode)){

    if(NotVisible(x, y, image->w, image->h))
        return;

    if(blendmode==bmBlend){ //Not compliant, but fixes MESA.
        BlitImage(image, x, y, bmBlend);
        return;
    }

    if(blendmode==bmAverage){
        BlitImageMaskAverage(image, x, y, mask, mask_blendmode);
        return;
    }

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);
	{
		GLuint color[] = {mask, mask, mask, mask};

		glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
	}
    glBindBuffer(GL_ARRAY_BUFFER, image->TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		GLint vertex[] = {x, y, x+image->w, y, x+image->w, y+image->h, x, y+image->h};

		glVertexPointer(2, GL_INT, 0, vertex);
	}
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}

EXPORT(void STDCALL TransformBlitImage(IMAGE * image, int x[4], int y[4], BlendMode blendmode)){

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);

    glBindBuffer(GL_ARRAY_BUFFER, FullColorBuffer);
    glColorPointer(4, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, image->TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		GLint vertex[] = {x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3]};

		glVertexPointer(2, GL_INT, 0, vertex);
	}


    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}
EXPORT(void STDCALL TransformBlitImageMask(IMAGE * image, int x[4], int y[4], BlendMode blendmode, RGBA mask, BlendMode mask_blendmode)){

    SetBlendMode(blendmode);

    glBindTexture(GL_TEXTURE_2D, image->texture);
	{
		GLuint color[] = {mask, mask, mask, mask};

		glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
	}
	glBindBuffer(GL_ARRAY_BUFFER, image->TexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		GLint vertex[] = {x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3]};

		glVertexPointer(2, GL_INT, 0, vertex);
	}


    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    EndBlendMode();

}
EXPORT(int STDCALL GetImageWidth(IMAGE * image)){
    return image->w;
}
EXPORT(int STDCALL GetImageHeight(IMAGE * image)){
    return image->h;
}
EXPORT(RGBA* STDCALL LockImage(IMAGE * image)){
    if(image->pixels)
        goto fin;

    image->pixels = malloc((image->w*image->h)<<2);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

fin:
    return image->pixels;

}

EXPORT(void STDCALL UnlockImage(IMAGE * image)){
    glBindTexture(GL_TEXTURE_2D, image->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
}

EXPORT(void STDCALL DirectBlit(int x, int y, int w, int h, RGBA* pixels)){

    if(NotVisible(x, y, w, h))
        return;
    {
		IMAGE image;

		glGenTextures(1, &(image.texture));

		glBindTexture(GL_TEXTURE_2D, image.texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        image.TexCoordBuffer = TexCoordBuffer;
		image.pixels = NULL;
		image.w = w;
		image.h = h;

		BlitImage(&image, x, y, 0);

		glDeleteTextures(1, &(image.texture));
    }
    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
}

EXPORT(void STDCALL DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels)){

    IMAGE image;

    glGenTextures(1, &(image.texture));

    glBindTexture(GL_TEXTURE_2D, image.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    image.TexCoordBuffer = TexCoordBuffer;
    image.pixels = NULL;
    image.w = w;
    image.h = h;

    //IMAGE *image = CreateImage(w, h, pixels);
    TransformBlitImage(&image, x, y, 0);
    glDeleteTextures(1, &(image.texture));
    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    //DestroyImage(image);
}

EXPORT(void STDCALL DirectGrab(int x, int y, int w, int h, RGBA* pixels)){
	int i = 0;

    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ScreenCopy);


    for(i = 0; i<h; ++i){
        memcpy(pixels+((i*w)<<2), ScreenCopy+((h-i)*w<<2), w<<2);
    }

}

EXPORT(void STDCALL DrawPoint(int x, int y, RGBA color)){

    //glBindTexture(GL_TEXTURE_2D, EmptyTexture);

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, &color);
    glBindBuffer(GL_ARRAY_BUFFER, ZeroTexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		GLint vertex[] = {x, y};

		glVertexPointer(2, GL_INT, 0, vertex);
	}


    glDrawArrays(GL_POINTS, 0, 1);



}

EXPORT(void STDCALL DrawPointSeries(int** points, int length, RGBA color)){

    if(!length)
        return;

    if(length<MAX_STACK_STEAL)
        goto onstack;
	{
		RGBA  *colors = NULL;
		GLint *vertex = NULL;

		colors = malloc(4*length);

		vertex = malloc(8*length);

		goto fill;

	onstack:;


		colors = alloca(4*length);

		vertex = alloca(8*length);

	fill:;
		 {
			GLint *texCoords = calloc(8, length);
			int i = 0;

			for(i = 0; i<length; ++i){
				colors[i] = color;
				vertex[(i*2)  ] = points[i][0];
				vertex[(i*2)+1] = points[i][1];
			}
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
			glTexCoordPointer(2, GL_INT, 0, texCoords);
			glVertexPointer(2, GL_INT, 0, vertex);
			//glBindTexture(GL_TEXTURE_2D, EmptyTexture);

			glDrawArrays(GL_POINTS, 0, length);

			free(texCoords);
		 }
		if(length<MAX_STACK_STEAL)
			return;

		free(colors);
		free(vertex);

	}
}

EXPORT(void STDCALL DrawLine(int x[2], int y[2], RGBA color)){

    GLint colors[] = {color, color};
    DrawGradientLine(x, y, colors);
}

EXPORT(void STDCALL DrawGradientLine(int x[2], int y[2], RGBA colors[2])){

    GLint vertex[] = {x[0], y[0], x[1], y[1]};

    //glBindTexture(GL_TEXTURE_2D, EmptyTexture);

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glBindBuffer(GL_ARRAY_BUFFER, ZeroTexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexPointer(2, GL_INT, 0, vertex);

    glDrawArrays(GL_LINES, 0, 2);
}

EXPORT(void STDCALL DrawLineSeries(int** points, int length, RGBA color, int type)){

    if(!length)
        return;

	{
		GLenum gltype = GL_LINES;

		switch (type){
		case 1:
			gltype = GL_LINE_STRIP;
			break;
		case 2:
			gltype = GL_LINE_LOOP;
		}

		{
			RGBA  *colors = NULL;
			GLint *vertex = NULL;

			if(length<MAX_STACK_STEAL)
				goto onstack;

			colors = malloc(4*length);

			vertex = malloc(8*length);

			goto fill;

		onstack:;


			colors = alloca(4*length);

			vertex = alloca(8*length);

		fill:;
			 {
				GLint *texCoords = calloc(8, length);
				int i = 0;

				for(i; i<length; ++i){
					colors[i] = color;
					vertex[(i*2)  ] = points[i][0];
					vertex[(i*2)+1] = points[i][1];
				}
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
				glTexCoordPointer(2, GL_INT, 0, texCoords);
				glVertexPointer(2, GL_INT, 0, vertex);

				//glBindTexture(GL_TEXTURE_2D, EmptyTexture);

				glDrawArrays(gltype, 0, length);

				free(texCoords);
			}
			if(length<MAX_STACK_STEAL)
				return;

			free(colors);
			free(vertex);
		}
	}
}

EXPORT(void STDCALL DrawTriangle(int x[3], int y[3], RGBA color)){

    GLint colors[] = {color, color, color};

    DrawGradientTriangle(x, y, colors);
}

EXPORT(void STDCALL DrawGradientTriangle(int x[3], int y[3], RGBA colors[3])){
    //if(!(Colored(colors[0])|Colored(colors[1])|Colored(colors[2])))
    //    return;
    GLint vertex[] = {x[0], y[0], x[1], y[1], x[2], y[2]};

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glBindBuffer(GL_ARRAY_BUFFER, ZeroTexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexPointer(2, GL_INT, 0, vertex);

    //glBindTexture(GL_TEXTURE_2D, EmptyTexture);



    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);


}
EXPORT(void STDCALL DrawPolygon(int** points, int length, int invert, RGBA color)){

    if(!length)
        return;

    if(length<MAX_STACK_STEAL)
        goto onstack;
	{
		RGBA  *colors = NULL;
		GLuint *vertex = NULL;

		colors = malloc(4*length);

		vertex = malloc(8*length);

		goto fill;

	onstack:;


		colors = alloca(4*length);

		vertex = alloca(8*length);

	fill:;
		 {
			GLint *texCoords = calloc(8, length);
			int i = 0;

			for(; i<length; ++i){
				colors[i] = color;
				vertex[(i*2)  ] = points[i][0];
				vertex[(i*2)+1] = points[i][1];
			}
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
			glTexCoordPointer(2, GL_INT, 0, texCoords);
			glVertexPointer(2, GL_INT, 0, vertex);

			//glBindTexture(GL_TEXTURE_2D, EmptyTexture);



			glDrawArrays(GL_TRIANGLE_STRIP, 0, length);




			free(texCoords);
		}
		if(length<MAX_STACK_STEAL)
			return;

		free(colors);
		free(vertex);
	}
}

EXPORT(void STDCALL DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4])){

    if(NotVisible(x, y, w, h)){
        return;
    }


    //glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

    glBindBuffer(GL_ARRAY_BUFFER, ZeroTexCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
		GLint vertex[] = {x, y, x+w, y, x+w, y+h, x, y+h};
		glVertexPointer(2, GL_INT, 0, vertex);
	}


    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


}

EXPORT(void STDCALL DrawRectangle(int x, int y, int w, int h, RGBA color)){

    GLuint colors[] = {color, color, color, color};

    DrawGradientRectangle(x, y, w, h, colors);
}



EXPORT(void STDCALL DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color)){

    if(NotVisible(x, y, w, h))
        return;

    if(!size)
        return;

	{
		GLuint colors[] = {color, color, color, color, color, color, color, color};
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	}

    glBindTexture(GL_TEXTURE_2D, EmptyTexture);
    glBindBuffer(GL_ARRAY_BUFFER, SeriousCoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	{
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
		glVertexPointer(2, GL_INT, 0, vertex);
	}


    glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);


}

EXPORT(void STDCALL DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color)){

    const unsigned int step = (unsigned)(SPI*(float)((rx>ry)?rx:ry));
	GLint * vertex = NULL;
    if(step<5){
        DrawPoint(x, y, color);
        return;
    }

    vertex = ApproximateEllipseGL(x, y, rx, ry, step);
	{
		RGBA  *colors = NULL;

		if(step<MAX_STACK_STEAL)
			goto onstack;

		colors = malloc(step<<2);

		goto fill;

	onstack:;

		colors = alloca(step<<2);

	fill:;
		 {
			GLint *texCoords = calloc(8, step);
			unsigned int i = 0;

			for(; i<step; i++){
				colors[i] = color;
			}

			glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
			glTexCoordPointer(2, GL_INT, 0, texCoords);
			glVertexPointer(2, GL_INT, 0, vertex);

			//glBindTexture(GL_TEXTURE_2D, EmptyTexture);



			glDrawArrays(GL_LINE_LOOP, 0, step);



			FreeEllipsePointsGL(vertex);

			free(texCoords);
		}
		if(step<MAX_STACK_STEAL)
			return;

		free(colors);
	}
}
EXPORT(void STDCALL DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color)){

    const unsigned int step = (unsigned)(SPI*(float)((rx>ry)?rx:ry));

	GLint *vertex = NULL;

    if(step<5){
        DrawPoint(x, y, color);
        return;
    }

    vertex = ApproximateEllipseGL(x, y, rx, ry, step);

	{
		RGBA  *colors = NULL;

		if(step<MAX_STACK_STEAL)
			goto onstack;

		colors = malloc(step<<2);

		goto fill;

	onstack:;

		colors = alloca(step<<2);

	fill:;
		 {
			GLint *texCoords = calloc(8, step);
			unsigned int i = 0;

			for(; i<step; i++){
				colors[i] = color;
			}

			glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
			glTexCoordPointer(2, GL_INT, 0, texCoords);
			glVertexPointer(2, GL_INT, 0, vertex);

			//glBindTexture(GL_TEXTURE_2D, EmptyTexture);

			glDrawArrays(GL_TRIANGLE_FAN, 0, step);

			FreeEllipsePointsGL(vertex);

			free(texCoords);
		 }
		if(step<MAX_STACK_STEAL)
			return;

		free(colors);
	}
}

EXPORT(void STDCALL DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias)){

    const unsigned int step = (unsigned)(SPI*((float)r));

	GLint * vertex = NULL;

    if(r<3){
        DrawPoint(x, y, color);
        return;
    }

    vertex = ApproximateCircleGL(x, y, r, step);
	{
		RGBA  *colors = NULL;

		if(step<MAX_STACK_STEAL)
			goto onstack;

		colors = malloc(step<<2);

		goto fill;

	onstack:;

		colors = alloca(step<<2);

	fill:;
		 {
		GLint *texCoords = calloc(8, step);
		unsigned int i = 0;

		for(; i<step; i++){
			colors[i] = color;
		}

		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
		glTexCoordPointer(2, GL_INT, 0, texCoords);
		glVertexPointer(2, GL_INT, 0, vertex);

		//glBindTexture(GL_TEXTURE_2D, EmptyTexture);



		glDrawArrays(GL_LINE_LOOP, 0, step);



		FreeEllipsePointsGL(vertex);

		free(texCoords);
		}
		if(step<MAX_STACK_STEAL)
			return;

		free(colors);
	}
}
EXPORT(void STDCALL DrawFilledCircle(int x, int y, int r, RGBA color, int antialias)){

    const unsigned int step = (unsigned)(SPI*((float)r));
	GLint * vertex = NULL;

    if(r<3){
        DrawPoint(x, y, color);
        return;
    }

    vertex = ApproximateCircleGL(x, y, r, step);
	{
		RGBA  *colors = NULL;

		if(step<MAX_STACK_STEAL)
			goto onstack;

		colors = malloc(step<<2);
		goto fill;

	onstack:;

		colors = alloca(step<<2);

	fill:;
		 {
			GLint *texCoords = calloc(8, step);
			unsigned int i = 0;

			for(; i<step; i++){
				colors[i] = color;
			}

			glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
			glTexCoordPointer(2, GL_INT, 0, texCoords);
			glVertexPointer(2, GL_INT, 0, vertex);

			//glBindTexture(GL_TEXTURE_2D, EmptyTexture);



			glDrawArrays(GL_TRIANGLE_FAN, 0, step);



			FreeEllipsePointsGL(vertex);

			free(texCoords);
		 }
		if(step<MAX_STACK_STEAL)
			return;

		free(colors);
	}
}
EXPORT(void STDCALL DrawGradientCircle(int x, int y, int r, RGBA color[2], int antialias)){

	GLint * vertex = NULL;
    unsigned int step = (unsigned)(SPI*((float)r));

    if(r<3){
        DrawPoint(x, y, *color);
        return;
    }


    vertex = ApproximateCircleGL(x, y, r, step);

    step++;

    vertex[0] = x;
    vertex[1] = y;
	{
		RGBA  *colors = NULL;

		if(step<MAX_STACK_STEAL)
			goto onstack;

		colors = malloc(step<<2);

		goto fill;

	onstack:;

		colors = alloca(step<<2);

	fill:;
		{
			GLint *texCoords = calloc(8, step);
			unsigned int i = 1;
			*colors = color[1];

			for(; i<step; i++){
				colors[i] = *color;
			}

			glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
			glTexCoordPointer(2, GL_INT, 0, texCoords);
			glVertexPointer(2, GL_INT, 0, vertex);

			//glBindTexture(GL_TEXTURE_2D, EmptyTexture);



			glDrawArrays(GL_TRIANGLE_FAN, 0, step);



			FreeEllipsePointsGL(vertex);

			free(texCoords);
		}
		if(step<MAX_STACK_STEAL)
			return;

		free(colors);
	}
}

/////
// Unimplemented:

EXPORT(void STDCALL DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic)){
    return;
}

EXPORT(void STDCALL DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias)){
    return;
}
EXPORT(void STDCALL DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2])){
    return;
}
EXPORT(void STDCALL DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3])){
    return;
}
