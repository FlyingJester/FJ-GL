#ifndef FJGL_CIRCLE_HEAD
#define FJGL_CIRCLE_HEAD

#include <math.h>
#include <stdlib.h>


/////
// Some math.h headers have this, some don't.

#ifdef M_PI
static float PI2 = M_PI*2;
#else
# define M_PI		3.14159265358979323846f	/* pi */
static float PI2 = 6.28318530717958647692f;
#endif
static float PIB2 = M_PI/2.0f;


/////
// Array of {x, y} coordinates

int **ApproximateCircle(const int x, const int y, const unsigned int r, const int step) ;
int *ApproximateCircleGL(const int x, const int y, const unsigned int r, const int step);
int **ApproximateEllipse(const int x, const int y, const unsigned int rx,const unsigned int ry, const int step) ;
int *ApproximateEllipseGL(const int x, const int y,  const unsigned int rx,const unsigned int ry, const int step);

void FreeCirclePoints(int  **p, int step);


#define FreeCirclePointsGL(p) free(p)

#define FreeEllipsePoints FreeCirclePoints
#define FreeEllipsePointsGL FreeCirclePointsGL

#endif
