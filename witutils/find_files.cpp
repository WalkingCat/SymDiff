#include "stdafx.h"
#include "find_files.h"

using namespace std;

map<wstring, wstring> find_files_impl(const wstring_view pattern, bool directories)
{
	map<wstring, wstring> ret;
	wchar_t path[MAX_PATH] = {};
	wcscpy_s(path, pattern.data());
	WIN32_FIND_DATA fd;
	HANDLE find = ::FindFirstFile(pattern.data(), &fd);
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

map<wstring, wstring> find_files(const wstring_view pattern)
{
	return find_files_impl(pattern, false);
}

namespace {
	wstring path_combine(const wstring_view dir, const wstring_view file) {
		wchar_t path[MAX_PATH] = {};
		wcscpy_s(path, dir.data());
		PathCombine(path, path, file.data());
		return path;
	}
}

std::map<std::wstring, std::map<std::wstring, std::wstring>>
find_files_ex(const wstring_view pattern, bool recursive, const wstring_view path_filter, const std::wstring_view default_file_pattern)
{
	std::map<std::wstring, std::map<std::wstring, std::wstring>> ret;

	if (pattern.empty())
		return ret;

	wstring dir, file_pat{ pattern };
	if (PathIsDirectory(pattern.data()) != FALSE) {
		dir = pattern;
		file_pat = default_file_pattern;
	} else if (PathIsFileSpec(pattern.data()) == FALSE) {
		const auto file_spec = PathFindFileName(pattern.data());
		if (file_spec != pattern.data()) {
			dir = wstring(pattern.data(), file_spec - pattern.data());
			file_pat = file_spec;
		}
	}

	if (path_filter.empty()) {
		auto files = find_files_impl(path_combine(dir, file_pat), false);
		if (!files.empty()) ret[wstring()] = move(files);
	}

	if (recursive) {
		static constexpr wstring_view asterisk = L"*";
		const auto dirs = find_files_impl(path_combine(dir, asterisk), true);
		for (auto& dir_pair : dirs) {
			const bool include = (dir_pair.first.find(path_filter) != wstring::npos);
			auto files = find_files_ex(dir_pair.second, true, include ? wstring_view() : path_filter, file_pat);
			for (auto pair : files) {
				ret[path_combine(dir_pair.first, pair.first)] = move(pair.second);
			}
		}
	}

	return ret;
}
