
#ifndef _WIN32
#include "api.hpp"
#include <cstdint>
#include <t5.h>
#include <cstring>
#include "shader.h"
#include "config.h"

extern "C" bool InitVideo(int, int, std::string);

const char *rootDir = "";
#ifdef _WIN32
const char *shaderDir = "";
const char *systemShader = "";
#endif


//extern "C" bool InitVideo (int, int, std::string) __attribute__ ((weak, alias ("_Z9InitVIdeoiiSs")));

bool InitVideo(int w, int h, std::string u){


    configl.fullscreen = 0;
    configl.vsync      = 0;
    configl.scale      = 1;
    configl.fullscreen = 0;
    configl.niceCircles= 1;
    configl.niceImages = 0;

    const char *videodir    = strdup((u+std::string("/system/video")).c_str());
    const char *configfile  = strdup((u+std::string("/system/video/fj-gl.b")).c_str());

    if(!T5_IsDir(videodir)){
        fprintf(stderr, "[FJ-GL] No system/video directory found.\n");
        //return InternalInitVideo(w, h);
    }

    if(!T5_IsFile(configfile)){
        fprintf(stderr, "[FJ-GL] No config file found.\n");
        WriteDefaultConfig(configfile);
    }
    ReadConfig(fopen(configfile, "rb"));

    free((void *)videodir);
    free((void *)configfile);

    return InternalInitVideo(w, h);


}

#else

#include <Windows.h>
BOOL WINAPI DllMain(
         HINSTANCE hinstDLL,  // handle to DLL module
         DWORD fdwReason,     // reason for calling function
         LPVOID lpReserved )  // reserved
{
	return true;
}
#endif
