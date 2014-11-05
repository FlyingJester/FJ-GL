#ifndef FJGL_DRVCONF_HEAD
#define FJGL_DRVCONF_HEAD
#include <stdbool.h>
#include <stdio.h>

typedef struct {

    int fullscreen;
    int vsync;

    int scale;

    int niceCircles;
    int niceImages;

    int hasVertexArrays;

} drvc;

extern drvc configl;

EXPORT(bool WriteDefaultConfig(const char *path));

EXPORT(bool ReadConfig(FILE *file));

#endif
