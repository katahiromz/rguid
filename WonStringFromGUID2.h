#pragma once

#ifndef WonStringFromGUID2
    #if defined(_WIN32) && !defined(_WON32)
        #include <objbase.h>
        #define WonStringFromGUID2 StringFromGUID2
    #else
        #ifdef __cplusplus
            extern "C"
            int WonStringFromGUID2(REFGUID id, wchar_t *str, int cmax);
        #else
            int WonStringFromGUID2(const GUID *id, wchar_t *str, int cmax);
        #endif
    #endif
#endif
