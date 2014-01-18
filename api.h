
#ifdef _WIN32
#include <Windows.h>
#include <Wingdi.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <stdint.h>

#endif

#include "api.hpp"

extern int ClipRectX, ClipRectY, ClipRectW, ClipRectH;

extern GLuint EmptyTexture;

extern GLuint TexCoordBuffer;
extern GLuint FullColorBuffer;
extern GLuint SeriousCoordBuffer;

extern uint32_t *ScreenCopy;
