#include "guid.h"
#include <cstring>

#if !defined(_WIN32) || defined(_WON32)

static inline bool is_valid_hex(wchar_t c)
{
    if (!(((c >= '0') && (c <= '9'))  ||
          ((c >= 'a') && (c <= 'f'))  ||
          ((c >= 'A') && (c <= 'F'))))
        return false;
    return true;
}

static const uint8_t guid_conv_table[256] =
{
  0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 */
  0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 */
  0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x20 */
  0,   1,   2,   3,   4,   5,   6, 7, 8, 9, 0, 0, 0, 0, 0, 0, /* 0x30 */
  0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 */
  0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x50 */
  0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf                             /* 0x60 */
};

/* conversion helper for CLSIDFromString/IIDFromString */
static bool guid_from_string(const wchar_t *s, GUID *id)
{
  int	i;

  if (!s || s[0]!='{') {
    memset( id, 0, sizeof (GUID) );
    if(!s) return true;
    return false;
  }

  /* in form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} */

  id->Data1 = 0;
  for (i = 1; i < 9; i++) {
    if (!is_valid_hex(s[i])) return false;
    id->Data1 = (id->Data1 << 4) | guid_conv_table[s[i]];
  }
  if (s[9]!='-') return false;

  id->Data2 = 0;
  for (i = 10; i < 14; i++) {
    if (!is_valid_hex(s[i])) return false;
    id->Data2 = (id->Data2 << 4) | guid_conv_table[s[i]];
  }
  if (s[14]!='-') return false;

  id->Data3 = 0;
  for (i = 15; i < 19; i++) {
    if (!is_valid_hex(s[i])) return false;
    id->Data3 = (id->Data3 << 4) | guid_conv_table[s[i]];
  }
  if (s[19]!='-') return false;

  for (i = 20; i < 37; i+=2) {
    if (i == 24) {
      if (s[i]!='-') return false;
      i++;
    }
    if (!is_valid_hex(s[i]) || !is_valid_hex(s[i+1])) return false;
    id->Data4[(i-20)/2] = guid_conv_table[s[i]] << 4 | guid_conv_table[s[i+1]];
  }

  if (s[37] == '}' && s[38] == '\0')
    return true;

  return false;
}

#ifdef __cplusplus
extern "C"
#endif
uint32_t WonCLSIDFromString(const wchar_t *idstr, GUID *id)
{
    HRESULT ret = CO_E_CLASSSTRING;

    if (!id)
        return E_INVALIDARG;

    if (guid_from_string(idstr, id))
        return S_OK;

#if 0
    /* It appears a ProgID is also valid */
    ret = clsid_from_string_reg(idstr, &tmp_id);
    if(SUCCEEDED(ret))
        *id = tmp_id;
#endif

    return ret;
}

#endif
