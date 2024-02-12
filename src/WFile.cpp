//
// Created by Aditya Jain on 12/2/24.
//

#include <Windows.h>
#include "WFile.h"

using namespace std;


string replaceFileExtension(const string& path) {
    const size_t dot = path.find_last_of('.');
    return path.substr(0,dot)+".exe";
}

filesystem::path exeDirectory()
{
#if defined(_MSC_VER)
    wchar_t path[FILENAME_MAX] = { 0 };
    GetModuleFileNameW(nullptr, path, FILENAME_MAX);
    return std::filesystem::path(path).parent_path().string();
#else
    char path[FILENAME_MAX];
    ssize_t count = readlink("/proc/self/exe", path, FILENAME_MAX);
    return std::filesystem::path(std::string(path, (count > 0) ? count: 0)).parent_path().string();
#endif
}
