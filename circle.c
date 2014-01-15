#include "circle.h"

int **ApproximateCircle(const int x, const int y, const unsigned int r, const int step){
    const float fr = r;
    int ** const points = malloc(sizeof(void *)*step);
    float angle = 0;


    float inc = (PI2)/((float)(step));

    int i = 0;

    while(i<step){
        points[i] = malloc(8);
        points[i][0] = (((float)(cos(angle)))*(fr))+x;
        points[i][1] = (((float)(sin(angle)))*(fr))+y;
        angle+=inc;
        i++;
    }
    return points;
}

int *ApproximateCircleGL(const int x, const int y, const unsigned int r, const int step){
    const float fr = r;
    int *const points = malloc(step<<3);
    float angle = 0;

    float inc = (PI2)/((float)(step));

    int i = 0;

    while(i<step){
        points[(i*2)  ] = (((float)(cos(angle)))*(fr))+x;
        points[(i*2)+1] = (((float)(sin(angle)))*(fr))+y;
        angle+=inc;
        i++;
    }
    return points;
}

int **ApproximateEllipse(const int x, const int y, const unsigned int rx,const unsigned int ry, const int step){
    const float frx = rx;
    const float fry = ry;

    int ** const points = malloc(sizeof(void *)*step);
    float angle = 0;


    float inc = (PI2)/((float)(step));

    int i = 0;

    while(i<step){
        points[i] = malloc(8);
        points[i][0] = (((float)(cos(angle)))*(frx))+x;
        points[i][1] = (((float)(sin(angle)))*(fry))+y;
        angle+=inc;
        i++;
    }
    return points;
}
int *ApproximateEllipseGL(const int x, const int y,  const unsigned int rx,const unsigned int ry, const int step){
    const float frx = rx;
    const float fry = ry;
    int *const points = malloc(step<<3);
    float angle = 0;

    float inc = (PI2)/((float)(step));

    int i = 0;

    while(i<step){
        points[(i*2)  ] = (((float)(cos(angle)))*(frx))+x;
        points[(i*2)+1] = (((float)(sin(angle)))*(fry))+y;
        angle+=inc;
        i++;
    }
    return points;
}

void FreeCirclePoints(int  **p, int step){
    while(step){
        free(p[--step]);
    }
    free(p);
}
