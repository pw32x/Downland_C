#include "debug_utils.h"

#ifdef _WINDOWS

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

void DebugPrintf(const char *format, ...) 
{
    char buffer[512];  // Buffer for formatted string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    OutputDebugStringA(buffer);
}
#endif