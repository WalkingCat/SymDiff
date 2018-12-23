#pragma once
#include <string>
#include <vector>
#include <map>

std::map<std::wstring, std::map<std::wstring, std::wstring>> find_files_wcs(const std::wstring& directory, const std::wstring& file_pattern = L"*");
std::map<std::wstring, std::map<std::wstring, std::wstring>> find_files_wcs_ex(const std::wstring& pattern, const std::wstring& default_file_pattern = L"*");