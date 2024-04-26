#include "guid.h"

#if !defined(_WIN32) || defined(_WON32)

extern "C"
int WonStringFromGUID2(REFGUID id, wchar_t *str, int cmax)
{
#define CHARS_IN_GUID 39
    static const wchar_t formatW[] = L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";
    if (&id == NULL || cmax < CHARS_IN_GUID)
        return 0;
    swprintf(str, formatW, id.Data1, id.Data2, id.Data3,
             id.Data4[0], id.Data4[1], id.Data4[2], id.Data4[3],
             id.Data4[4], id.Data4[5], id.Data4[6], id.Data4[7]);
    return CHARS_IN_GUID;
#undef CHARS_IN_GUID
}

#endif
