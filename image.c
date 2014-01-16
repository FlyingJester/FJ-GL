#include "api.h"
#include <GL/gl.h>
#include "glExtra.h"


EXPORT(IMAGE * CreateImage(int width, int height, RGBA* pixels)){

//    RGBA *newpixels = malloc(width*height*4);

/*
    for(int i = 0; i<height; ++i){
        memcpy(newpixels+(i*width), pixels+(i*width), width*4);
    }
*/
    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    IMAGE *im = malloc(sizeof(IMAGE));
    im->pixels = NULL;//newpixels;
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
