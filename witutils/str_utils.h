#pragma once
#include <string>

inline std::wstring tolower(const std::wstring_view str) {
	std::wstring ret(str);
	for (auto& chr : ret) {
		chr = std::tolower(chr);
	}
	return std::move(ret);
}
