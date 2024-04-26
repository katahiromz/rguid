#pragma once

#if !defined(_WIN32) || defined(_WON32)

#ifdef __cplusplus
extern "C"
#endif
uint32_t CLSIDFromString(const wchar_t *idstr, GUID *id);

#endif
