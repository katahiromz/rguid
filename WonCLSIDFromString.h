#pragma once

#ifndef WonCLSIDFromString
    #if defined(_WIN32) && !defined(_WON32)
        #include <objbase.h>
        #define WonCLSIDFromString CLSIDFromString
    #else
        #include "guid.h"
        #ifdef __cplusplus
            extern "C"
        #endif
        uint32_t WonCLSIDFromString(const wchar_t *idstr, GUID *id);
        #define WonCLSIDFromString WonCLSIDFromString
    #endif
#endif
