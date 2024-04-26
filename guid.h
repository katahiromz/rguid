// guid.hpp - The GUID analyzer library
// License: MIT

#pragma once

#if defined(_WIN32) && !defined(_WON32)
#include <windows.h>
#endif
#include <string>
#include <vector>
#include <cstdio> // for FILE
#include <cstdint>

//////////////////////////////////////////////////////////////////////////////////////////////////

#define GUID_VERSION "rguid version 1.9.4 by katahiromz"

//////////////////////////////////////////////////////////////////////////////////////////////////
// GUID / REFGUID / DEFINE_GUID

#if !defined(_WIN32) || defined(_WON32)
typedef struct _GUID
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} GUID;
typedef const GUID& REFGUID;
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#endif  // ndef _WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////
// HRESULT

#if !defined(_WIN32) || defined(_WON32)
typedef uint32_t HRESULT;
#define S_OK 0
#define E_INVALIDARG 0x80070057
#define CO_E_CLASSSTRING 0x800401F3
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
// GUID_ENTRY / GUID_DATA / GUID_FOUND

struct GUID_ENTRY
{
    std::wstring name;
    GUID guid;
};
typedef std::vector<GUID_ENTRY> GUID_DATA, GUID_FOUND;

GUID_DATA* guid_load_data_a(const  char   *data_file);
#ifdef _WIN32
GUID_DATA* guid_load_data_w(const wchar_t *data_file);
#endif

#ifdef UNICODE
    #define guid_load_data guid_load_data_w
#else
    #define guid_load_data guid_load_data_a
#endif

GUID_DATA* guid_read_from_file(FILE *fp);
void guid_close_data(GUID_DATA *data);

//////////////////////////////////////////////////////////////////////////////////////////////////

void guid_random_generate(GUID& guid);

bool guid_equal(const GUID& guid1, const GUID& guid2);

#ifdef _WIN32
std::string guid_ansi_from_wide (const wchar_t *text, unsigned int cp = 0);
#endif
std::wstring guid_wide_from_ansi(const  char   *text, unsigned int cp = 0);

bool guid_from_definition(GUID& guid, const wchar_t *text);
bool guid_from_definition(GUID& guid, const wchar_t *text, std::wstring *p_name);
bool guid_from_guid_text(GUID& guid, const wchar_t *text);
bool guid_from_struct_text(GUID& guid, const wchar_t *text);
bool guid_from_hex_text(GUID& guid, const wchar_t *text);

std::wstring guid_to_definition(const GUID& guid, const wchar_t *name = NULL);
std::wstring guid_to_guid_text(const GUID& guid);
std::wstring guid_to_struct_text(const GUID& guid);
std::wstring guid_to_hex_text(const GUID& guid);

bool guid_is_definition(const wchar_t *text);
bool guid_is_guid_text(const wchar_t *text);
bool guid_is_struct_text(const wchar_t *text);
bool guid_is_hex_text(const wchar_t *text);

bool guid_parse(GUID& guid, const wchar_t *text);
std::wstring guid_dump(const GUID& guid, const wchar_t *name);

bool guid_search_by_guid(GUID_FOUND& found, const GUID_DATA *data, const GUID& guid);
bool guid_search_by_name(GUID_FOUND& found, const GUID_DATA *data, const wchar_t *name);
bool guid_search_by_text(GUID_FOUND& found, const GUID_DATA *data, const wchar_t *text);

//////////////////////////////////////////////////////////////////////////////////////////////////

class GuidDataBase
{
    GUID_DATA *m_data;

public:
    GuidDataBase() : m_data(NULL)
    {
    }
    GuidDataBase(const char *filename) : m_data(guid_load_data_a(filename))
    {
    }
    GuidDataBase(const wchar_t *filename) : m_data(guid_load_data_w(filename))
    {
    }
    ~GuidDataBase()
    {
        close();
    }

    bool load(const char *filename)
    {
        if (m_data) close();
        m_data = guid_load_data_a(filename);
        return is_loaded();
    }
    bool load(const wchar_t *filename)
    {
        if (m_data) close();
        m_data = guid_load_data_w(filename);
        return is_loaded();
    }
    bool is_loaded() const { return !!m_data; }

    void close()
    {
        if (m_data)
        {
            guid_close_data(m_data);
            m_data = NULL;
        }
    }

    bool search_by_guid(GUID_FOUND& found, const GUID& guid)
    {
        return guid_search_by_guid(found, m_data, guid);
    }
    bool search_by_name(GUID_FOUND& found, const wchar_t *name)
    {
        return guid_search_by_name(found, m_data, name);
    }
    bool search_by_text(GUID_FOUND& found, const wchar_t *text)
    {
        return guid_search_by_text(found, m_data, text);
    }

          GUID_DATA& data()       { return *m_data; };
    const GUID_DATA& data() const { return *m_data; };
};
