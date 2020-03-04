#pragma once
#include "cmdl_utils.h"

struct diff_cmdl {
	const static cmdl_option show_help;
	const static cmdl_option new_files;
	const static cmdl_option old_files;
	const static cmdl_option filter;
	const static cmdl_option recursive;
	const static cmdl_option wcs_folder;
	const static cmdl_option out_file;
	const static cmdl_option * const options[7];
	const static cmdl_option * const default_option;
};

struct diff_params {
	bool show_help;
	std::wstring error;
	std::wstring new_files_pattern;
	std::wstring old_files_pattern;
	std::wstring path_filter;
	std::wstring output_file_name;
	FILE* out;
	bool is_wcs;
	bool is_rec;
	std::map<std::wstring, std::map<std::wstring, std::wstring>> new_file_groups;
	std::map<std::wstring, std::map<std::wstring, std::wstring>> old_file_groups;
};

diff_params init_diff_params(int argc, wchar_t* argv[], const std::wstring_view default_file_pattern = L"*");
diff_params init_diff_params(const options_data_t& options_data, const std::wstring_view default_file_pattern = L"*");
