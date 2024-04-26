#pragma once

#if !defined(_WIN32) || defined(_WON32)

#ifdef __cplusplus
extern "C"
int StringFromGUID2(REFGUID id, wchar_t *str, int cmax);
#else
int StringFromGUID2(const GUID *id, wchar_t *str, int cmax);
#endif

#endif
