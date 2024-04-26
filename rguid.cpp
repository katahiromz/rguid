// rguid.cpp --- The GUID analyzer
// License: MIT

#include "guid.h"
#include <cassert>

#if !defined(_WIN32) || defined(_WON32)
DEFINE_GUID(IID_IShellLinkW, 0x000214F9, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif

GuidDataBase g_database;
bool g_bSearch = false;
bool g_bList = false;
int g_nGenerate = 0;

void show_version(void)
{
    std::puts(GUID_VERSION);
}

void usage(void)
{
    std::printf(
        "rguid --- The GUID analyzer."
        "\n"
        "Usage:\n"
        "  rguid --search \"STRING\"\n"
        "  rguid IID_IDeskBand\n"
        "  rguid \"{EB0FE172-1A3A-11D0-89B3-00A0C90A90AC}\"\n"
        "  rguid \"72 E1 0F EB 3A 1A D0 11 89 B3 00 A0 C9 0A 90 AC\"\n"
        "  rguid \"{ 0xEB0FE172, 0x1A3A, 0x11D0, { 0x89, 0xB3, 0x00, 0xA0, 0xC9, 0x0A,\n"
        "            0x90, 0xAC } }\"\n"
        "  rguid \"DEFINE_GUID(IID_IDeskBand, 0xEB0FE172, 0x1A3A, 0x11D0, 0x89, 0xB3,\n"
        "                      0x00, 0xA0, 0xC9, 0x0A, 0x90, 0xAC);\"\n"
        "  rguid --list\n"
        "  rguid --generate NUMBER\n"
        "  rguid --help\n"
        "  rguid --version\n"
        "\n"
        "You can specify multiple GUIDs.\n");
}

typedef enum RET
{
    RET_DONE = 0,
    RET_FAILED = -1,
    RET_SUCCESS = +1,
} RET;

void do_unittest(void)
{
#ifndef NDEBUG
    GUID guid = IID_IShellLinkW;

    assert(g_database.load(L"guid.dat"));

    GUID_FOUND found;
    assert(g_database.search_by_name(found, L"IID_IShellLinkW"));
    assert(found.size() >= 1);
    assert(found[0].name == L"IID_IShellLinkW");
    assert(guid_equal(found[0].guid, IID_IShellLinkW));

    found.clear();
    assert(g_database.search_by_guid(found, guid));
    assert(found.size() >= 1);
    assert(found[0].name == L"IID_IShellLinkW");
    assert(guid_equal(found[0].guid, IID_IShellLinkW));

    auto define_guid = guid_to_definition(guid, NULL);
    assert(define_guid ==
        L"DEFINE_GUID(<Name>, 0x000214F9, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);");
    assert(guid_is_definition(define_guid.c_str()));

    auto guid_text = guid_to_guid_text(guid);
    assert(guid_text == L"{000214F9-0000-0000-C000-000000000046}");
    assert(guid_is_guid_text(guid_text.c_str()));

    auto hex_text = guid_to_hex_text(guid);
    assert(hex_text == L"F9 14 02 00 00 00 00 00 C0 00 00 00 00 00 00 46");
    assert(guid_is_hex_text(hex_text.c_str()));

    auto struct_text = guid_to_struct_text(guid);
    assert(struct_text == L"{ 0x000214F9, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } }");
    assert(guid_is_struct_text(struct_text.c_str()));
#endif
}

RET do_guid(REFGUID guid, std::wstring *pstrName = NULL)
{
    std::printf("\n--------------------\n");

    std::wstring name;
    if (pstrName == NULL)
    {
        for (auto& entry : g_database.data())
        {
            if (guid_equal(guid, entry.guid))
            {
                name = entry.name;
                std::printf("Name: %ls\n", name.c_str());
            }
        }
    }

    auto str = guid_dump(guid, (pstrName ? pstrName->c_str() : name.c_str()));
    std::printf("%ls", str.c_str());
    return RET_SUCCESS;
}

RET do_arg(std::wstring str)
{
    GUID guid;
    if (guid_parse(guid, str.c_str()))
    {
        return do_guid(guid);
    }

    if (!g_bSearch)
    {
        GUID_FOUND found;
        if (g_database.search_by_name(found, str.c_str()))
            return do_guid(found[0].guid, &found[0].name);
    }

    GUID_FOUND found;
    if (g_database.search_by_text(found, str.c_str()))
    {
        if (found.size() == 1)
            return do_guid(found[0].guid, &found[0].name);
    }
    else
    {
        std::printf("Not found\n");
        return RET_FAILED;
    }

    if (found.size() > 1)
    {
        std::printf("Found %d found.\n", (int)found.size());
    }

    for (auto& item : found)
    {
        do_guid(item.guid, &item.name);
    }

    return RET_SUCCESS;
}

RET parse_option(std::wstring str)
{
    if (str == L"--help")
    {
        usage();
        return RET_DONE;
    }

    if (str == L"--version")
    {
        show_version();
        return RET_DONE;
    }

    if (str == L"--list")
    {
        g_bList = true;
        return RET_SUCCESS;
    }

    if (str == L"--search")
    {
        g_bSearch = true;
        return RET_SUCCESS;
    }

    fprintf(stderr, "Invalid option: %ls\n", str.c_str());
    return RET_FAILED;
}

bool is_ident(const wchar_t *param)
{
    if (param[0] == 0)
        return false;

    for (size_t ich = 0; param[ich]; ++ich)
    {
        wchar_t ch = param[ich];
        if (ich == 0 && !iswalpha(ch) && ch != L'_')
            return false;
        if (ich > 0 && !iswalnum(ch) && ch != L'_')
            return false;
    }

    return true;
}

RET parse_cmd_line(std::vector<std::wstring>& args, int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return RET_FAILED;
    }

    std::wstring param;

    for (int iarg = 1; iarg < argc; ++iarg)
    {
        std::wstring str = guid_wide_from_ansi(argv[iarg]);

        if (str[0] == L'-')
        {
            if (str == L"--generate")
            {
                if (argc <= iarg + 1)
                    g_nGenerate = 1;
                else
                    g_nGenerate = atoi(argv[iarg + 1]);

                if (g_nGenerate <= 0)
                {
                    fprintf(stderr, "ERROR: Zero or negative value specified for '--generate'\n");
                    return RET_FAILED;
                }

                param.clear();
                args.clear();
                return RET_SUCCESS;
            }

            switch (parse_option(str))
            {
            case RET_FAILED:
                return RET_FAILED;
            case RET_DONE:
                return RET_DONE;
            case RET_SUCCESS:
                continue;
            }
        }

        if (param.size())
            param += L' ';

        param += str;

        if (guid_is_guid_text(param.c_str()) || guid_is_struct_text(param.c_str()) ||
            guid_is_definition(param.c_str()) || guid_is_hex_text(param.c_str()))
        {
            args.push_back(param);
            param.clear();
            continue;
        }

        if (is_ident(param.c_str()) && param != L"DEFINE_GUID")
        {
            args.push_back(param);
            param.clear();
            continue;
        }
    }

    if (param.size())
        args.push_back(param);

    return RET_SUCCESS;
}

int main(int argc, char **argv)
{
#ifndef NDEBUG
    do_unittest();
#endif

    std::vector<std::wstring> args;
    RET ret = parse_cmd_line(args, argc, argv);
    if (ret == RET_DONE)
        return 0;
    if (ret == RET_FAILED)
        return -1;

    if (!g_database.load(L"guid.dat") && !g_bList && g_nGenerate == 0)
    {
        std::printf("ERROR: File 'guid.dat' is not loaded\n");
        return -2;
    }

    if (g_bList)
    {
        for (auto& entry : g_database.data())
        {
            auto define_guid = guid_to_definition(entry.guid, entry.name.c_str());
            std::printf("%ls\n", define_guid.c_str());
        }
        return 0;
    }

    if (g_nGenerate > 0)
    {
        for (int i = 0; i < g_nGenerate; ++i)
        {
            GUID guid;
            guid_random_generate(guid);
            do_guid(guid);
        }
        return 0;
    }

    for (auto& item : args)
    {
        if (do_arg(item) == RET_FAILED)
            return -4;
    }

    return 0;
}
