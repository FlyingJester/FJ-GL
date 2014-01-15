#include "api.hpp"

extern "C" bool InitVideo(int, int, std::string);

//extern "C" bool InitVideo (int, int, std::string) __attribute__ ((weak, alias ("_Z9InitVIdeoiiSs")));

bool InitVideo(int w, int h, std::string _unused){
    return InternalInitVideo(w, h);
}
