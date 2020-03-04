#pragma once
#include <string>
#include <vector>
#include <map>

std::map<std::wstring, std::map<std::wstring, std::wstring>>
find_files_wcs(const std::wstring_view directory, const std::wstring_view path_filter, const std::wstring_view file_pattern = L"*");

std::map<std::wstring, std::map<std::wstring, std::wstring>>
find_files_wcs_ex(const std::wstring_view pattern, const std::wstring_view path_filter, const std::wstring_view default_file_pattern = L"*");