
#include "api.h"
#include <stdbool.h>
#include "config.h"

drvc configl;

/////
// Config File spec:

/////
// 4 byte signature (must be 'fjgl')
// 1 byte XOR check values.
// 1 byte flags
//      bit meaning
//      0   fullscreen
//      1   vsync
//      2   niceCicles
//      3   niceImages
//      4
//      5
//      6
//      7
//
// 1 byte scale (0-255).
//
const char *sigref = "fjgl";

bool WriteDefaultConfig(const char *path){
    printf("[FJ-GL] Writing config file to %s\n", path);
    FILE *file = fopen(path, "wb");
    fwrite(sigref, 1, 4, file);

    unsigned char flags = 0x0u;
    unsigned char scale = 1u;
    unsigned char XORt = 0^'f'^'j'^'g'^'l'^0x0u^1u;

    fwrite(&XORt, 1, 1, file);
    fwrite(&flags, 1, 1, file);
    fwrite(&scale, 1, 1, file);

    fflush(file);
    fclose(file);

    return true;

}

bool ReadConfig(FILE *file){
    rewind(file);
    char signature[5];
    signature[4] = '\0';
    fread(signature, 1, 4, file);

    if(strcmp(signature, sigref)){
        fprintf(stderr, "[FJ-GL] Warning: Signature bad in config file.\n");
        return false;
    }
    unsigned char XORc;

    fread(&XORc, 1, 1, file);

    XORc^='f'^'j'^'g'^'l';

    unsigned char flags = 0;
    fread(&flags, 1, 1, file);

    XORc^=flags;

    if(flags&0x1){
        configl.fullscreen = 1;
        printf("[FJ-GL] Entering Fullscreen.\n");
    }
    if(flags&0x2){
        configl.vsync = 1;
        printf("[FJ-GL] Using VSync.\n");
    }
    if(flags&0x4){
        configl.niceCircles = 1;
        printf("[FJ-GL] Using accurate linear approximations.\n");
    }
    if(flags&0x8){
        configl.niceImages = 1;
        printf("[FJ-GL] Using linear interpolation of textures.\n");
    }

    fread(&flags, 1, 1, file);
    XORc^=flags;

    configl.scale = flags;

    rewind(file);

    if(XORc!=0x0u){
        fprintf(stderr, "[FJ-GL] Warning: Checksum bad on config file.\n");
        return false;
    }
    return true;
}
