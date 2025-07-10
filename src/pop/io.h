#pragma once
#include "pch.h"
#include "overlay_manager.h"

inline std::wstring get_file_path_in_my_documents(const std::wstring &relativePath)
{
    WCHAR path[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
    {
        std::wstring fullPath = std::wstring(path) + L"\\pop\\" + relativePath;
        return fullPath;
    }
    else
    {
        std::wcerr << L"Failed to get the My Documents folder path." << std::endl;
        return L"";
    }
}

inline std::map<std::wstring, std::wstring> load_bmp_files_map()
{
    std::map<std::wstring, std::wstring> filesMap;
    std::wstring directoryPath = get_file_path_in_my_documents(L"");

    if (!directoryPath.empty())
    {
        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
            {
                if (entry.is_regular_file() && entry.path().extension() == L".bmp")
                {
                    std::wstring filename = entry.path().stem().wstring();
                    filesMap[filename] = entry.path().wstring();
                }
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::wcerr << L"Filesystem error: " << e.what() << std::endl;
        }
    }

    return filesMap;
}