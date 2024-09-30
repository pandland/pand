#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC
#endif

DLL_PUBLIC int say_hello(const char *name) {
    printf("Hello %s, from dynamic library function\n", name);
    return 0;
}
