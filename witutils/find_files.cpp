#include "stdafx.h"
#include "find_files.h"

using namespace std;

map<wstring, wstring> find_files_impl(const wchar_t * pattern, bool directories)
{
	map<wstring, wstring> ret;
	wchar_t path[MAX_PATH] = {};
	wcscpy_s(path, pattern);
	WIN32_FIND_DATA fd;
	HANDLE find = ::FindFirstFile(pattern, &fd);
	if (find != INVALID_HANDLE_VALUE) {
		do {
			if ((wcscmp(fd.cFileName, L".") == 0) || (wcscmp(fd.cFileName, L"..") == 0))
				continue;

			if (directories == ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)) {
				PathRemoveFileSpec(path);
				PathCombine(path, path, fd.cFileName);
				wstring name = fd.cFileName;
				for (auto& c : name) c = towlower(c);
				ret[name] = path;
			}
		} while (::FindNextFile(find, &fd));
		::FindClose(find);
	}
	return ret;
}

map<wstring, wstring> find_files(const wchar_t * pattern)
{
	return find_files_impl(pattern, false);
}

namespace {
	wstring path_combine(const wstring& dir, const wstring& file) {
		wchar_t path[MAX_PATH] = {};
		wcscpy_s(path, dir.c_str());
		PathCombine(path, path, file.c_str());
		return path;
	}
}

std::map<std::wstring, std::map<std::wstring, std::wstring>> find_files_ex(const wstring& pattern, bool recursive, const std::wstring& default_file_pattern)
{
	std::map<std::wstring, std::map<std::wstring, std::wstring>> ret;
	
	if (pattern.empty())
		return ret;
	
	wstring dir;
	wstring file_pat = pattern;
	if (PathIsDirectory(pattern.c_str()) != FALSE) {
		dir = pattern;
		file_pat = default_file_pattern;
	} else if (PathIsFileSpec(pattern.c_str()) == FALSE) {
		const auto file_spec = PathFindFileName(pattern.c_str());
		if (file_spec != pattern.c_str()) {
			dir = wstring(pattern.c_str(), file_spec - pattern.c_str());
			file_pat = file_spec;
		}
	}

	auto files = find_files_impl(path_combine(dir, file_pat).c_str(), false);
	if (!files.empty()) ret[wstring()] = move(files);

	if (recursive) {
		static const wstring asterisk = L"*";
		auto directories = find_files_impl(path_combine(dir, asterisk).c_str(), true);
		for (auto& dir_pair : directories) {
			auto files = find_files_ex(dir_pair.second, true, file_pat);
			for (auto pair : files) {
				wchar_t key[MAX_PATH] = {};
				wcscpy_s(key, dir_pair.first.c_str());
				PathCombine(key, key, pair.first.c_str());
				ret[key] = move(pair.second);
			}
		}
	}

	return ret;
}
