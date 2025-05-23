// guid.cpp - The GUID analyzer library
// License: MIT

#include "guid.h"
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cwchar>
#include "WonCLSIDFromString.h"
#include "WonStringFromGUID2.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T_STR>
static inline bool
mstr_replace_all(T_STR& str, const T_STR& from, const T_STR& to)
{
    bool ret = false;
    size_t i = 0;
    for (;;) {
        i = str.find(from, i);
        if (i == T_STR::npos)
            break;
        ret = true;
        str.replace(i, from.size(), to);
        i += to.size();
    }
    return ret;
}

template <typename T_STR>
static inline bool
mstr_replace_all(T_STR& str,
                 const typename T_STR::value_type *from,
                 const typename T_STR::value_type *to)
{
    return mstr_replace_all(str, T_STR(from), T_STR(to));
}

template <typename T_STR_CONTAINER>
static inline void
mstr_split(T_STR_CONTAINER& container,
    const typename T_STR_CONTAINER::value_type& str,
    const typename T_STR_CONTAINER::value_type& chars)
{
    container.clear();
    size_t i = 0, k = str.find_first_of(chars);
    while (k != T_STR_CONTAINER::value_type::npos)
    {
        container.push_back(str.substr(i, k - i));
        i = k + 1;
        k = str.find_first_of(chars, i);
    }
    container.push_back(str.substr(i));
}

template <typename T_STR>
static inline void
mstr_trim(T_STR& str, const typename T_STR::value_type* spaces)
{
    size_t i = str.find_first_not_of(spaces), j = str.find_last_not_of(spaces);
    if ((i == T_STR::npos) || (j == T_STR::npos))
        str.clear();
    else
        str = str.substr(i, j - i + 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool guid_equal(const GUID& guid1, const GUID& guid2)
{
#if defined(_WIN32) && !defined(_WON32)
    return ::IsEqualGUID(guid1, guid2);
#else
    return memcmp(&guid1, &guid2, sizeof(guid1)) == 0;
#endif
}

bool guid_is_valid_value(const wchar_t *text)
{
    std::wstring str = text;
    mstr_trim(str, L" \t\r\n");
    wchar_t *endptr;
    wcstoul(str.c_str(), &endptr, 0);
    return (*endptr == 0 || (endptr[0] == 'L' && endptr[1] == 0));
}

void guid_remove_comments(std::wstring& str)
{
    // Remove C comments
    size_t ich0 = 0, ich1;
    for (;;)
    {
        ich0 = str.find(L"/*", ich0);
        if (ich0 == str.npos)
            break;

        ich1 = str.find(L"*/", ich0);
        if (ich1 == str.npos)
            break;

        str.erase(ich0, ich1 + 2 - ich0);
    }

    // Remove C++ comments
    ich0 = 0;
    for (;;)
    {
        ich0 = str.find(L"//", ich0);
        if (ich0 == str.npos)
            break;

        ich1 = str.find(L"\n", ich0);
        if (ich1 == str.npos)
            str.erase(ich0, str.size() - ich0);
        else
            str.erase(ich0, ich1 - ich0);
    }
}

bool guid_from_definition(GUID& guid, const wchar_t *text, std::wstring *p_name)
{
    std::wstring str = text;
    mstr_trim(str, L" \t\r\n");
    guid_remove_comments(str);
    mstr_trim(str, L" \t\r\n");

    bool codecapi = false, midl = false, oleguid = false;
    if (str.find(L"DEFINE_GUID") == 0 || str.find(L"EXTERN_GUID") == 0)
    {
        str.erase(0, 11);
    }
    else if (str.find(L"DEFINE_CODECAPI_GUID") == 0)
    {
        str.erase(0, 20);
        codecapi = true;
    }
    else if (str.find(L"MIDL_DEFINE_GUID") == 0)
    {
        str.erase(0, 16);
        midl = true;
    }
    else if (str.find(L"DEFINE_OLEGUID") == 0)
    {
        str.erase(0, 14);
        oleguid = true;
    }

    else
    {
        return false;
    }

    mstr_trim(str, L" \t\r\n");

    if (str.size() && str[0] == L'(')
        str.erase(0, 1);
    else
        return false;

    mstr_trim(str, L" \t\r\n");

    if (str.size() && str[str.size() - 1] == L';')
        str.erase(str.size() - 1, 1);

    mstr_trim(str, L" \t\r\n");

    if (str.size() && str[str.size() - 1] == L')')
        str.erase(str.size() - 1, 1);
    else
        return false;

    mstr_trim(str, L" \t\r\n");
    mstr_replace_all(str, L" ", L"");
    mstr_replace_all(str, L"\t", L"");

    std::vector<std::wstring> items;
    mstr_split(items, str, L",");

    if (codecapi)
        items.erase(items.begin() + 1);

    if (midl)
        items.erase(items.begin());

    if (oleguid && items.size() == 4)
    {
        std::vector<std::wstring> new_items =
        {
            items[0], items[1], items[2], items[3], L"0xC0",
            L"0", L"0", L"0", L"0", L"0", L"0", L"0x46"
        };
        items = std::move(new_items);
    }

    if (items.size() != 12)
        return false;

    for (auto& item : items)
    {
        mstr_trim(item, L" \t");
    }

    for (int i = 1; i <= 11; ++i)
    {
        if (!guid_is_valid_value(items[i].c_str()))
            return false;
    }

    guid.Data1 = (uint32_t)wcstoul(items[1].c_str(), NULL, 0);
    guid.Data2 = (uint16_t)wcstoul(items[2].c_str(), NULL, 0);
    guid.Data3 = (uint16_t)wcstoul(items[3].c_str(), NULL, 0);
    guid.Data4[0] = (uint8_t)wcstoul(items[4].c_str(), NULL, 0);
    guid.Data4[1] = (uint8_t)wcstoul(items[5].c_str(), NULL, 0);
    guid.Data4[2] = (uint8_t)wcstoul(items[6].c_str(), NULL, 0);
    guid.Data4[3] = (uint8_t)wcstoul(items[7].c_str(), NULL, 0);
    guid.Data4[4] = (uint8_t)wcstoul(items[8].c_str(), NULL, 0);
    guid.Data4[5] = (uint8_t)wcstoul(items[9].c_str(), NULL, 0);
    guid.Data4[6] = (uint8_t)wcstoul(items[10].c_str(), NULL, 0);
    guid.Data4[7] = (uint8_t)wcstoul(items[11].c_str(), NULL, 0);

    if (p_name)
        *p_name = items[0];

    return true;
}

bool guid_from_definition(GUID& guid, const wchar_t *text)
{
    return guid_from_definition(guid, text, NULL);
}

bool guid_from_guid_text(GUID& guid, const wchar_t *text)
{
    return WonCLSIDFromString(text, &guid) == S_OK;
}

bool guid_from_struct_text(GUID& guid, std::wstring str)
{
    mstr_trim(str, L" \t\r\n");
    guid_remove_comments(str);
    mstr_trim(str, L" \t\r\n");

    int level = 0;
    for (size_t ich = 0; ich < str.size(); ++ich)
    {
        if (str[ich] == L'{')
            ++level;
        if (str[ich] == L'}')
            --level;
    }

    if (level != 0)
        return false;

    if (str.size() && str[str.size() - 1] == L';')
        str.erase(str.size() - 1, 1);
    mstr_trim(str, L" \t\r\n");
    if (str.size() && str[0] == L'{')
    {
        str.erase(0, 1);
    }
    else
    {
        return false;
    }
    mstr_trim(str, L" \t\r\n");
    if (str.size() && str[str.size() - 1] == L'}')
    {
        str.erase(str.size() - 1, 1);
    }
    else
    {
        return false;
    }
    mstr_trim(str, L" \t\r\n");

    mstr_replace_all(str, L"{", L"");
    mstr_replace_all(str, L"}", L"");

    mstr_replace_all(str, L" ", L"");
    mstr_replace_all(str, L"\t", L"");

    mstr_trim(str, L" \t\r\n");

    std::vector<std::wstring> items;
    mstr_split(items, str, L",");

    if (items.size() != 11)
    {
        return false;
    }

    for (auto& item : items)
    {
        mstr_trim(item, L" \t");
        if (!guid_is_valid_value(item.c_str()))
        {
            return false;
        }
    }

    guid.Data1 = (uint32_t)wcstoul(items[0].c_str(), NULL, 0);
    guid.Data2 = (uint16_t)wcstoul(items[1].c_str(), NULL, 0);
    guid.Data3 = (uint16_t)wcstoul(items[2].c_str(), NULL, 0);
    guid.Data4[0] = (uint8_t)wcstoul(items[3].c_str(), NULL, 0);
    guid.Data4[1] = (uint8_t)wcstoul(items[4].c_str(), NULL, 0);
    guid.Data4[2] = (uint8_t)wcstoul(items[5].c_str(), NULL, 0);
    guid.Data4[3] = (uint8_t)wcstoul(items[6].c_str(), NULL, 0);
    guid.Data4[4] = (uint8_t)wcstoul(items[7].c_str(), NULL, 0);
    guid.Data4[5] = (uint8_t)wcstoul(items[8].c_str(), NULL, 0);
    guid.Data4[6] = (uint8_t)wcstoul(items[9].c_str(), NULL, 0);
    guid.Data4[7] = (uint8_t)wcstoul(items[10].c_str(), NULL, 0);

    return true;
}

std::wstring guid_to_guid_text(const GUID& guid)
{
    wchar_t text[40];
    WonStringFromGUID2(guid, text, _countof(text));
    return text;
}

std::string guid_ansi_from_wide(const wchar_t *text, unsigned int cp)
{
    char buf[1024];
#if defined(_WIN32) && !defined(_WON32)
    ::WideCharToMultiByte(cp, 0, text, -1, buf, _countof(buf), NULL, NULL);
    buf[_countof(buf) - 1] = 0; // Avoid buffer overrun
#else
    // ASCII only
    size_t ich;
    for (ich = 0; text[ich] && ich < _countof(buf) - 1; ++ich)
    {
        //assert(text[ich] < 0x80);
        buf[ich] = (char)text[ich];
    }
    buf[ich] = 0;
#endif
    return buf;
}

std::wstring guid_wide_from_ansi(const char *text, unsigned int cp)
{
    wchar_t buf[1024];
#if defined(_WIN32) && !defined(_WON32)
    ::MultiByteToWideChar(cp, 0, text, -1, buf, _countof(buf));
    buf[_countof(buf) - 1] = 0; // Avoid buffer overrun
#else
    // ASCII only
    size_t ich;
    for (ich = 0; text[ich] && ich < _countof(buf) - 1; ++ich)
    {
        //assert(text[ich] < 0x80);
        buf[ich] = text[ich];
    }
    buf[ich] = 0;
#endif
    return buf;
}

GUID_DATA* guid_read_from_file(FILE *fp)
{
    GUID_DATA *entries = new GUID_DATA;

    char buf[1024];
    while (fgets(buf, sizeof(buf), fp))
    {
        std::string str = buf;
        mstr_trim(str, " \t\r\n");
#if defined(_WIN32) && !defined(_WON32)
        auto wide = guid_wide_from_ansi(str.c_str(), CP_UTF8);
#else
        std::wstring wide;
        for (size_t i = 0; i < str.size(); ++i)
            wide += str[i];
#endif

        GUID guid;
        std::wstring name;
        if (guid_from_definition(guid, wide.c_str(), &name))
            entries->push_back({ name, guid });
    }

    return entries;
}

GUID_DATA *guid_load_data_a(const char *data_file)
{
    FILE *fp = fopen(data_file, "rb");
    if (!fp)
        return NULL;

    GUID_DATA *ptr = guid_read_from_file(fp);
    fclose(fp);
    return ptr;
}

#ifdef _WIN32
GUID_DATA *guid_load_data_w(const wchar_t *data_file)
{
    FILE *fp = _wfopen(data_file, L"rb");
    if (!fp)
        return NULL;

    GUID_DATA *ptr = guid_read_from_file(fp);
    fclose(fp);
    return ptr;
}
#endif

void guid_close_data(GUID_DATA *data)
{
    delete data;
}

std::wstring guid_to_hex_text(const GUID& guid)
{
    std::wstring ret;
    const uint8_t *pb = (const uint8_t*)&guid;
    for (size_t ib = 0; ib < sizeof(guid); ++ib)
    {
        if (ib)
            ret += L' ';
#define HEX L"0123456789ABCDEF"
        wchar_t sz[3] = { HEX[pb[ib] >> 4], HEX[pb[ib] & 0xF], 0 };
        ret += sz;
    }
    return ret;
}

bool guid_from_struct_text(GUID& guid, const wchar_t *text)
{
    std::wstring str = text;
    return guid_from_struct_text(guid, str);
}

bool guid_from_hex_text(GUID& guid, const wchar_t *text)
{
    std::wstring str = text;
    mstr_replace_all(str, L"0x", L"");

    GUID ret;
    std::vector<uint8_t> bytes;
    wchar_t sz[3];
    size_t ich = 0, ib = 0;
    for (auto ch : str)
    {
        if (!iswxdigit(ch))
            continue;
        if ((ich % 2) == 0)
        {
            sz[0] = ch;
        }
        else
        {
            sz[1] = ch;
            sz[2] = 0;
            uint8_t byte = (uint8_t)wcstoul(sz, NULL, 16);
            bytes.push_back(byte);
            ib++;
        }
        ich++;
    }

    if (ib != sizeof(GUID))
        return false;

    memcpy(&guid, bytes.data(), sizeof(ret));
    return true;
}

std::wstring guid_to_definition(const GUID& guid, const wchar_t *name)
{
    wchar_t sz[256];
    swprintf(sz,
        L"DEFINE_GUID(%ls, 0x%08X, 0x%04X, 0x%04X, 0x%02X, 0x%02X, 0x%02X, "
        L"0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X);", (name && name[0] ? name : L"<Name>"),
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return sz;
}

std::wstring guid_to_struct_text(const GUID& guid, const wchar_t *name)
{
    std::wstring ret;
    if (name)
    {
        ret += L"const GUID ";
        ret += name;
        ret += L" = ";
    }
    wchar_t sz[256];
    swprintf(sz,
        L"{ 0x%08X, 0x%04X, 0x%04X, { 0x%02X, 0x%02X, 0x%02X, "
        L"0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X } }",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    ret += sz;
    if (name)
        ret += L';';
    return ret;
}

bool guid_parse(GUID& guid, const wchar_t *text)
{
    if (guid_from_guid_text(guid, text))
        return true;

    if (guid_from_definition(guid, text))
        return true;

    if (guid_from_struct_text(guid, text))
        return true;

    if (guid_from_hex_text(guid, text))
        return true;

    return false;
}

std::wstring guid_dump(const GUID& guid, const wchar_t *name)
{
    if (name && name[0] == 0)
        name = NULL;

    std::wstring ret;

    auto define_guid = guid_to_definition(guid, name);
    ret += define_guid;
    ret += L"\n\n";

    auto guid_text = guid_to_guid_text(guid);
    ret += L"GUID: ";
    ret += guid_text;
    ret += L"\n\n";

    auto hex_text = guid_to_hex_text(guid);
    ret += L"Hex: ";
    ret += hex_text;
    ret += L"\n\n";

    auto struct_text = guid_to_struct_text(guid, name);
    if (!name)
        ret += L"Struct: ";
    ret += struct_text;
    ret += L"\n";

    return ret;
}

bool guid_is_definition(const wchar_t *text)
{
    GUID guid;
    return guid_from_definition(guid, text, NULL);
}

bool guid_is_guid_text(const wchar_t *text)
{
    GUID guid;
    return guid_from_guid_text(guid, text);
}

bool guid_is_hex_text(const wchar_t *text)
{
    std::wstring str = text;

    mstr_trim(str, L" \t\r\n");
    mstr_replace_all(str, L"0x", L"");
    mstr_replace_all(str, L",", L"");
    mstr_replace_all(str, L" ", L"");
    mstr_replace_all(str, L"\t", L"");

    if (str.size() != 32)
        return false;

    for (auto& ch : str)
    {
        if (!iswxdigit(ch))
            return false;
    }

    return true;
}

bool guid_is_struct_text(const wchar_t *text)
{
    GUID guid;
    return guid_from_struct_text(guid, text);
}

void guid_random_generate(GUID& guid)
{
    CoCreateGuid(&guid);
}

bool guid_search_by_name(GUID_FOUND& found, const GUID_DATA *data, const wchar_t *name)
{
    std::wstring strName = name;
    _wcsupr(&strName[0]);

    for (auto& entry : *data)
    {
        std::wstring entry_name = entry.name;
        _wcsupr(&entry_name[0]);

        if (entry_name == strName)
        {
            found.push_back(entry);
            return true;
        }
    }

    return false;
}

bool guid_search_by_guid(GUID_FOUND& found, const GUID_DATA *data, const GUID& guid)
{
    for (auto& entry : *data)
    {
        if (guid_equal(entry.guid, guid))
            found.push_back(entry);
    }
    return !found.empty();
}

bool guid_search_by_text(GUID_FOUND& found, const GUID_DATA *data, const wchar_t *text)
{
    std::wstring str = text;
    _wcsupr(&str[0]);

    for (auto& entry : *data)
    {
        auto def_text = guid_to_definition(entry.guid, entry.name.c_str());
        _wcsupr(&def_text[0]);
        if (def_text.find(str) != def_text.npos)
        {
            found.push_back(entry);
            continue;
        }

        auto guid_text = guid_to_guid_text(entry.guid);
        _wcsupr(&guid_text[0]);
        if (guid_text.find(str) != guid_text.npos)
        {
            found.push_back(entry);
            continue;
        }

        auto struct_text = guid_to_struct_text(entry.guid);
        _wcsupr(&struct_text[0]);
        if (struct_text.find(str) != struct_text.npos)
        {
            found.push_back(entry);
            continue;
        }

        auto hex_text = guid_to_hex_text(entry.guid);
        _wcsupr(&hex_text[0]);

        if (hex_text.find(str) != hex_text.npos)
        {
            found.push_back(entry);
            continue;
        }
    }

    return !found.empty();
}

#if defined(_WIN32) && !defined(_WON32)
enum ENCODING
{
    ENCODING_UTF8 = 0,
    ENCODING_UTF16LE = 1,
    ENCODING_UTF16BE = 2,
};

static bool guid_scan_text(GUID_FOUND& found, void *ptr, size_t size, ENCODING encoding)
{
    size_t cw = size / sizeof(uint16_t);

    if (encoding == ENCODING_UTF16BE)
    {
        uint16_t *pw = (uint16_t *)ptr;
        while (cw-- > 0)
        {
            auto low = *pw & 0xFF;
            auto high = (*pw >> 8) & 0xFF;
            *pw++ = (low << 8) | high;
        }
        return guid_scan_text(found, ptr, size, ENCODING_UTF16LE);
    }

    if (encoding == ENCODING_UTF8)
    {
        int cchW = MultiByteToWideChar(CP_UTF8, 0, (char *)ptr, (INT)size, NULL, 0);
        wchar_t *pszW = (wchar_t *)malloc((cchW + 1) * sizeof(wchar_t));
        if (!pszW)
            return false;
        MultiByteToWideChar(CP_UTF8, 0, (char *)ptr, (INT)size, pszW, cchW);
        bool ret = guid_scan_text(found, pszW, cchW, ENCODING_UTF16LE);
        free(pszW);
        return ret;
    }

    assert(encoding == ENCODING_UTF16LE);

    const wchar_t *pszW = (const wchar_t *)ptr;

    // Scan DEFINE_GUID(...) and EXTERN_GUID(...)
    bool no_define_guid = false;
    bool no_extern_guid = false;
    bool no_codecapi_guid = false;
    bool no_midl_guid = false;
    bool no_const = false;
    for (auto pch = pszW;; )
    {
        const wchar_t *pch0 = nullptr;
        const wchar_t *pch1 = nullptr;
        const wchar_t *pch5 = nullptr;
        const wchar_t *pch6 = nullptr;
        const wchar_t *pch7 = nullptr;
        if (!no_define_guid)
        {
            pch0 = wcsstr(pch, L"DEFINE_GUID");
            if (!pch0)
                no_define_guid = true;
        }
        if (!no_extern_guid)
        {
            pch1 = wcsstr(pch, L"EXTERN_GUID");
            if (!pch1)
                no_extern_guid = true;
        }
        if (!no_codecapi_guid)
        {
            pch5 = wcsstr(pch, L"DEFINE_CODECAPI_GUID");
            if (!pch5)
                no_codecapi_guid = true;
        }
        if (!no_midl_guid)
        {
            pch6 = wcsstr(pch, L"MIDL_DEFINE_GUID");
            if (!pch6)
                no_midl_guid = true;
        }
        if (!no_const)
        {
            pch7 = wcsstr(pch, L"const");
            if (!pch7)
                no_const = true;
        }
        if (!pch0 && pch1)
            pch0 = pch1;
        if (!pch0 && pch5)
            pch0 = pch5;
        if (!pch0 && pch6)
            pch0 = pch6;
        if (!pch0 && pch7)
            pch0 = pch7;
        if (pch0 > pch1 && pch1)
            pch0 = pch1;
        if (pch0 > pch5 && pch5)
            pch0 = pch5;
        if (pch0 > pch6 && pch6)
            pch0 = pch6;
        if (pch0 > pch7 && pch7)
            pch0 = pch7;
        if (!pch0)
            break;

        if (pch0 == pch7)
        {
            if (!iswspace(pch0[5]))
            {
                pch = pch0 + 5;
                continue;
            }
            pch0 += 5; // "const"

            const wchar_t *pch10 = pch0;
            while (*pch0 && iswspace(*pch0))
                ++pch0;

            // GUID
            if (memcmp(pch0, L"GUID", 4 * sizeof(wchar_t)) != 0 || !iswspace(pch0[4]))
            {
                pch = pch10 + 1;
                continue;
            }
            pch0 += 4;

            // identifier
            std::wstring name;
            for (;;)
            {
                if (*pch0 && iswspace(*pch0))
                    ++pch0;

                if (!iswalpha(*pch0) && *pch0 != L'_')
                {
                    name.clear();
                    break;
                }

                do
                {
                    name += *pch0;
                    ++pch0;
                } while (*pch0 && (iswalnum(*pch0) || *pch0 == L'_'));

                if (name == L"OLEDBDECLSPEC")
                {
                    name.clear();
                    continue;
                }

                break;
            }

            if (name.empty())
            {
                pch = pch10 + 1;
                continue;
            }

            while (*pch0 && iswspace(*pch0))
                ++pch0;

            // "="
            if (*pch0 != L'=')
            {
                pch = pch10 + 1;
                continue;
            }
            ++pch0;

            while (*pch0 && iswspace(*pch0))
                ++pch0;

            if (*pch0 != L'{')
            {
                pch = pch10 + 1;
                continue;
            }

            const wchar_t *pch11 = pch0;
            int level = 0;
            while (*pch0)
            {
                if (*pch0 == ';' || *pch0 == '=')
                {
                    pch11 = pch0;
                    break;
                }
                if (*pch0 == '{')
                    ++level;
                if (*pch0 == '}')
                {
                    --level;
                    if (level == 0)
                        break;
                }
                ++pch0;
            }

            GUID guid;
            std::wstring str(pch11, pch0 - pch11 + 1);
            if (guid_from_struct_text(guid, str))
            {
                GUID_ENTRY entry;
                entry.name = name;
                entry.guid = guid;
                found.push_back(entry);
                pch = pch0 + 1;
                continue;
            }

            pch = pch11;
            continue;
        }

        auto pch2 = wcschr(pch0, L')');
        if (!pch2)
        {
            pch = pch0 + 1;
            continue;
        }

        GUID guid;
        std::wstring str(pch0, pch2 - pch0 + 1), name;
        if (guid_from_definition(guid, str.c_str(), &name))
        {
            GUID_ENTRY entry;
            entry.name = name;
            entry.guid = guid;
            found.push_back(entry);
            pch = pch2 + 1;
            continue;
        }

        pch = pch0 + 1;
    }

    // Scan {GUID}
    for (auto pch = pszW;; )
    {
        auto pch0 = wcsstr(pch, L"{");
        if (!pch0)
            break;

        auto pch1 = wcschr(pch0, L'}');
        if (pch1 == nullptr)
        {
            pch = pch0 + 1;
            continue;
        }

        GUID guid;
        std::wstring str(pch0, pch1 - pch0 + 1);
        if (guid_from_guid_text(guid, str.c_str()))
        {
            GUID_ENTRY entry;
            entry.guid = guid;
            found.push_back(entry);
            pch = pch1 + 1;
            continue;
        }

        pch = pch0 + 1;
    }

    guid_sort_and_unique(found);

    return found.size();
}

void guid_sort_and_unique(GUID_FOUND& found)
{
    std::sort(found.begin(), found.end(), [](const GUID_ENTRY& x, const GUID_ENTRY& y) {
        if (x.name < y.name)
            return true;
        if (x.name > y.name)
            return false;
        int cmp = memcmp(&x.guid, &y.guid, sizeof(GUID));
        return cmp < 0;
    });
    found.erase(std::unique(found.begin(), found.end(), [](const GUID_ENTRY& x, const GUID_ENTRY& y) {
        return memcmp(&x.guid, &y.guid, sizeof(GUID)) == 0 && x.name == y.name;
    }), found.end());
}

bool guid_scan_fp(GUID_FOUND& found, FILE *fp)
{
    char bom[3];
    if (!fread(bom, 3, 1, fp))
        return false;

    ENCODING encoding;
    if (memcmp(bom, "\xEF\xBB\xBF", 3) == 0)
    {
        encoding = ENCODING_UTF8;
    }
    else if (memcmp(bom, "\xFF\xFE", 2) == 0)
    {
        encoding = ENCODING_UTF16LE;
        fseek(fp, -1, SEEK_CUR);
    }
    else if (memcmp(bom, "\xFE\xFF", 2) == 0)
    {
        encoding = ENCODING_UTF16BE;
        fseek(fp, -1, SEEK_CUR);
    }
    else if (bom[0] && !bom[1])
    {
        encoding = ENCODING_UTF16LE;
        fseek(fp, -3, SEEK_CUR);
    }
    else if (!bom[0] && bom[1])
    {
        encoding = ENCODING_UTF16BE;
        fseek(fp, -3, SEEK_CUR);
    }
    else
    {
        encoding = ENCODING_UTF8;
    }

    std::string binary;
    char buf[1024];
    for (;;)
    {
        size_t size = fread(buf, 1, 1024, fp);
        if (!size)
            break;

        binary.append(buf, size);
    }

    binary.append(1, 0);

    return guid_scan_text(found, const_cast<char*>(binary.c_str()), binary.size(), encoding);
}

bool guid_scan_file_a(GUID_FOUND& found, const char *fname)
{
    FILE *fp = fopen(fname, "rb");
    if (!fp)
        return false;

    bool ret = guid_scan_fp(found, fp);
    fclose(fp);
    return ret;
}

bool guid_scan_file_w(GUID_FOUND& found, const wchar_t *fname)
{
    FILE *fp = _wfopen(fname, L"rb");
    if (!fp)
        return false;

    bool ret = guid_scan_fp(found, fp);
    fclose(fp);
    return ret;
}
#endif
