
#ifndef _WIN32
#include "api.hpp"
#include <cstdint>
#include <t5.h>
#include <cstring>
#include "shader.h"

extern "C" bool InitVideo(int, int, std::string);

const char *rootDir = "";
const char *shaderDir = "";
const char *systemShader = "";

//extern "C" bool InitVideo (int, int, std::string) __attribute__ ((weak, alias ("_Z9InitVIdeoiiSs")));

bool InitVideo(int w, int h, std::string u){

    T5_init(1, "./");

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