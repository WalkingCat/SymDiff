#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

inline std::vector<std::wstring> read_text_file(const std::wstring_view path) {
	std::vector<std::wstring> ret;
	FILE* f = nullptr;
	_wfopen_s(&f, path.data(), L"r, ccs=UTF-8");
	if (f != nullptr) {
		std::wifstream fs(f);
		while (fs) {
			std::wstring line;
			std::getline(fs, line);
			ret.emplace_back(move(line));
		}
		fclose(f);
	}
	return ret;
}