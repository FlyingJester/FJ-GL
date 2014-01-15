#ifndef FJGL_API_HEAD
#define FJGL_API_HEAD

#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#ifdef __cplusplus
    #include <string>
    #define TYPE(x, y) x y
    #ifdef __linux__
        #define EXPORT(x) extern "C" x
    #endif


#else
#include <stdbool.h>

    #define TYPE(x, y) typdef y x
    #define EXPORT(x)  x

#endif

#ifdef __GNUC__
    #define ALIGN __attribute__((aligned(16)))
#endif



typedef unsigned int RGBA;

typedef struct {
    const char* name;
    const char* author;
    const char* date;
    const char* version;
    const char* description;
} DriverInfo_t;


typedef enum {
    bmBlend = 0,
    bmReplace,
    bmRGB,
    bmAlpha,
    bmAdd,
    bmSubtract,
    bmMultiply,
    bmAverage,
    bmInvert,
    bmNone
} BlendMode;

typedef struct {
    RGBA *pixels;
    GLuint texture;
    unsigned int w;
    unsigned int h;

} ALIGN IMAGE;

#ifdef __linux__
EXPORT(bool InternalInitVideo(int w, int h));

#else

#endif

EXPORT(void GetDriverInfo(DriverInfo_t* driverinfo));

EXPORT(void CloseVideoDriver(void));
EXPORT(bool ToggleFullscreen(void));

EXPORT(void FlipScreen(void));
EXPORT(void SetClippingRectangle(int x, int y, int w, int h));
EXPORT(void GetClippingRectangle(int* x, int* y, int* w, int* h));

EXPORT(IMAGE * CreateImage(int width, int height, RGBA* pixels));
EXPORT(IMAGE * CloneImage(IMAGE * image));
EXPORT(IMAGE * GrabImage(IMAGE * image, int x, int y, int width, int height));
EXPORT(void DestroyImage(IMAGE * image));
EXPORT(void BlitImage(IMAGE * image, int x, int y, BlendMode blendmode));
EXPORT(void BlitImageMask(IMAGE * image, int x, int y, BlendMode blendmode, RGBA mask, BlendMode mask_blendmode));
EXPORT(void TransformBlitImage(IMAGE * image, int *x, int *y, BlendMode blendmode));
EXPORT(void TransformBlitImageMask(IMAGE * image, int x[4], int y[4], BlendMode blendmode, RGBA mask, BlendMode mask_blendmode));
EXPORT(int GetImageWidth(IMAGE * image));
EXPORT(int GetImageHeight(IMAGE * image));
EXPORT(RGBA* LockImage(IMAGE * image));
EXPORT(void UnlockImage(IMAGE * image));
EXPORT(void DirectBlit(int x, int y, int w, int h, RGBA* pixels));
EXPORT(void DirectTransformBlit(int x[4], int y[4], int w, int h, RGBA* pixels));
EXPORT(void DirectGrab(int x, int y, int w, int h, RGBA* pixels));

EXPORT(void DrawPoint(int x, int y, RGBA color));
EXPORT(void DrawPointSeries(int** points, int length, RGBA color));
EXPORT(void DrawLine(int x[2], int y[2], RGBA color));
EXPORT(void DrawGradientLine(int x[2], int y[2], RGBA colors[2]));
EXPORT(void DrawLineSeries(int** points, int length, RGBA color, int type));
EXPORT(void DrawBezierCurve(int x[4], int y[4], double step, RGBA color, int cubic));
EXPORT(void DrawTriangle(int x[3], int y[3], RGBA color));
EXPORT(void DrawGradientTriangle(int x[3], int y[3], RGBA colors[3]));
EXPORT(void DrawPolygon(int** points, int length, int invert, RGBA color));
EXPORT(void DrawOutlinedRectangle(int x, int y, int w, int h, int size, RGBA color));
EXPORT(void DrawRectangle(int x, int y, int w, int h, RGBA color));
EXPORT(void DrawGradientRectangle(int x, int y, int w, int h, RGBA colors[4]));
EXPORT(void DrawOutlinedComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, RGBA color, int antialias));
EXPORT(void DrawFilledComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[2]));
EXPORT(void DrawGradientComplex(int r_x, int r_y, int r_w, int r_h, int circ_x, int circ_y, int circ_r, float angle, float frac_size, int fill_empty, RGBA colors[3]));
EXPORT(void DrawOutlinedEllipse(int x, int y, int rx, int ry, RGBA color));
EXPORT(void DrawFilledEllipse(int x, int y, int rx, int ry, RGBA color));
EXPORT(void DrawOutlinedCircle(int x, int y, int r, RGBA color, int antialias));
EXPORT(void DrawFilledCircle(int x, int y, int r, RGBA color, int antialias));
EXPORT(void DrawGradientCircle(int x, int y, int r, RGBA colors[2], int antialias));


#endif
