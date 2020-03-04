#pragma once
#include <string>
#include <map>

std::map<std::wstring, std::wstring> find_files(const std::wstring_view pattern);
std::map<std::wstring, std::map<std::wstring, std::wstring>> find_files_ex(const std::wstring_view pattern, bool recursive, const std::wstring_view path_filter, const std::wstring_view default_file_pattern = L"*");
