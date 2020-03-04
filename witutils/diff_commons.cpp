#include "stdafx.h"
#include "diff_commons.h"
#include "find_files.h"
#include "find_files_wcs.h"
#include "str_utils.h"

using namespace std;

constexpr cmdl_option diff_cmdl::show_help{ L"?",		L"help",		nullptr,		L"show this help" };
constexpr cmdl_option diff_cmdl::new_files{ L"n",		L"new",			L"<filename>",	L"specify new file(s)" };
constexpr cmdl_option diff_cmdl::old_files{ L"o",		L"old",			L"<filename>",	L"specify old file(s)" };
constexpr cmdl_option diff_cmdl::filter{	L"f",		L"filter",		L"<text>",		L"path filter" };
constexpr cmdl_option diff_cmdl::recursive{ L"r",		L"recursive",	nullptr,		L"search folder recursively" };
constexpr cmdl_option diff_cmdl::wcs_folder{ nullptr,	L"wcs",			nullptr,		L"folder is Windows Component Store" };
constexpr cmdl_option diff_cmdl::out_file{	L"O",		L"out",			L"<filename>",	L"output to file" };
constexpr const cmdl_option* diff_cmdl::options[7] = { &show_help, &new_files, &old_files, &filter, &recursive, &wcs_folder, &out_file };
constexpr const cmdl_option* diff_cmdl::default_option = &diff_cmdl::new_files;

diff_params init_diff_params(int argc, wchar_t* argv[], const std::wstring_view default_file_pattern)
{
	return init_diff_params(parse_cmdl(argc, argv, diff_cmdl::options, diff_cmdl::default_option), default_file_pattern);
}

diff_params init_diff_params(const options_data_t& options_data, const std::wstring_view default_file_pattern)
{
	diff_params ret = {};

	ret.show_help = options_data.find(&diff_cmdl::show_help) != options_data.end();
	auto opt_it = options_data.find(nullptr);
	if (opt_it != options_data.end()) {
		ret.error = L"error in option: " + opt_it->second;
		ret.show_help = true;
		return ret;
	}
	opt_it = options_data.find(&diff_cmdl::new_files);
	if (opt_it != options_data.end()) {
		ret.new_files_pattern = opt_it->second;
	}
	opt_it = options_data.find(&diff_cmdl::old_files);
	if (opt_it != options_data.end()) {
		ret.old_files_pattern = opt_it->second;
	}
	opt_it = options_data.find(&diff_cmdl::filter);
	if (opt_it != options_data.end()) {
		ret.path_filter = tolower(opt_it->second);
	}
	opt_it = options_data.find(&diff_cmdl::out_file);
	if (opt_it != options_data.end()) {
		ret.output_file_name = opt_it->second;
	}

	ret.is_wcs = options_data.find(&diff_cmdl::wcs_folder) != options_data.end();
	ret.is_rec = options_data.find(&diff_cmdl::recursive) != options_data.end();

	if (!ret.output_file_name.empty()) {
		ret.out = nullptr;
		_wfopen_s(&ret.out, ret.output_file_name.c_str(), L"w, ccs=UTF-8");

		if (ret.out == nullptr) {
			ret.error = L"can't open " + ret.output_file_name + L" for output";
		}
	} else {
		ret.out = stdout;
	}

	auto search_files = [&](auto is_new) -> map<wstring, map<wstring, wstring>> {
		map<wstring, map<wstring, wstring>> file_groups;
		const auto& files_pattern = decltype(is_new)::value ? ret.new_files_pattern : ret.old_files_pattern;
		fwprintf_s(ret.out, L" %ls files: %ls", decltype(is_new)::value ? L"new" : L"old", files_pattern.c_str());
		if (ret.is_wcs) {
			file_groups = find_files_wcs_ex(files_pattern, ret.path_filter, default_file_pattern);
		} else {
			file_groups = find_files_ex(files_pattern, ret.is_rec, ret.path_filter, default_file_pattern);
		}
		fwprintf_s(ret.out, L"%ls\n", !file_groups.empty() ? L"" : L" (EMPTY!)");
		return file_groups;
	};

	ret.new_file_groups = search_files(true_type());
	ret.old_file_groups = search_files(false_type());
	if (ret.new_file_groups.empty() && ret.old_file_groups.empty()) {
		ret.error = L"nothing to do";
		ret.show_help = true;
	} else {
		if (!(ret.is_wcs || ret.is_rec)) {
			auto& new_files = ret.new_file_groups[wstring()], & old_files = ret.old_file_groups[wstring()];
			if ((new_files.size() == 1) && (old_files.size() == 1)) {
				// allows diff single files with different names
				auto& new_file_name = new_files.begin()->first;
				auto& old_file_name = old_files.begin()->first;
				if (new_file_name != old_file_name) {
					auto diff_file_names = new_file_name + L" <=> " + old_file_name;
					auto new_file = new_files.begin()->second;
					new_files.clear();
					new_files[diff_file_names] = new_file;
					auto old_file = old_files.begin()->second;
					old_files.clear();
					old_files[diff_file_names] = old_file;
				}
			}
		}
	}

	return ret;
}
