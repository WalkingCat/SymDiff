#include "stdafx.h"
#include "cmdl_utils.h"

using namespace std;

options_data_t parse_cmdl(int argc, wchar_t * argv[], const cmdl_option * const options[], size_t count, const cmdl_option* const default_option) {
	unordered_map<const cmdl_option*, wstring> ret;

	for (int i = 1; i < argc; ++i) {
		const wchar_t* arg = argv[i];
		const cmdl_option* option = nullptr;
		if ((arg[0] == '-') || ((arg[0] == '/'))) {
			const bool is_alt = ((arg[0] == '-') && (arg[1] == '-'));

			for (size_t i = 0; i < count; ++i) {
				auto o = options[i];
				const bool match = is_alt ?
					((o->arg_alt != nullptr) && (wcscmp(arg + 2, o->arg_alt) == 0)) :
					((o->arg != nullptr) && (wcscmp(arg + 1, o->arg) == 0));
				if (match) {
					option = o;
				}
			}

			bool err = false;
			if (option != nullptr) {
				if (option->data_desc != nullptr) {
					if ((i + 1) < argc) {
						ret[option] = argv[++i];
					} else err = true;
				} else {
					ret[option] = wstring();
				}
			} else err = true;

			if (err) {
				ret[nullptr] = arg;
				break;
			}

		} else if ((default_option != nullptr) && (default_option->data_desc != nullptr) && (ret.find(default_option) == ret.end())) {
			ret[default_option] = arg;
		}

	}

	return ret;
}

void print_cmdl_usage(const wchar_t * app, const cmdl_option* const options[], size_t count, const cmdl_option* const default_option) {
	wprintf_s(L" Usage: %ls [options]\n\n", app);
	for (size_t i = 0; i < count; ++i) {
		auto o = options[i];
		if (o->arg != nullptr) {
			wprintf_s((o == default_option) ? L" [-%ls]" : L" -%ls", o->arg);
		} else {
			wprintf_s(L" ");
		}

		size_t len = 0;
		if (o->arg_alt != nullptr) {
			len = wcslen(o->arg_alt);
			wprintf_s(L"\t--%ls", o->arg_alt);
		} else wprintf_s(L"\t");

		if (len < 6) wprintf_s(L"\t");

		if (o->data_desc != nullptr) len += wprintf_s(L" %ls", o->data_desc);

		if (len < 14) wprintf_s(L"\t");

		wprintf_s(L"\t: %ls\n", o->desc);
	}
	wprintf_s(L"\n");
}
