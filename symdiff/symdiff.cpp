#include "pch.h"

using namespace std;

struct sym_data {
	bool loaded;
	set<wstring> compilands;
	set<wstring> functions;
	set<wstring> global_data;
	set<wstring> public_symbols;
	map<wstring, set<wstring>> udts;
	map<wstring, set<wstring>> enums;
};

_com_ptr_t<_com_IIID<IDiaDataSource, &__uuidof(IDiaDataSource)>> create_data_source();
sym_data load_sym_data(const wstring& file, const wstring& sym_path);

namespace cmdl {
	const cmdl_option new_sym_path{ L"ns",		L"new_sym_path",			L"<path>",	L"specify new symbol path" };
	const cmdl_option old_sym_path{ L"os",		L"old_sym_path",			L"<path>",	L"specify old symbol path" };
}

int wmain(int argc, wchar_t* argv[])
{
	wprintf_s(L"\n SymDiff v0.2 https://github.com/WalkingCat/SymDiff\n\n");

	vector<const cmdl_option*> options(begin(diff_cmdl::options), end(diff_cmdl::options));
	options.push_back(&cmdl::new_sym_path);
	options.push_back(&cmdl::old_sym_path);

	auto options_data = parse_cmdl(argc, argv, options.data(), options.size());
	const auto& params = init_diff_params(options_data);

	const auto new_sym_path = options_data[&cmdl::new_sym_path];
	const auto old_sym_path = options_data[&cmdl::old_sym_path];

	if (params.show_help || (!params.error.empty()) || (params.new_files_pattern.empty() && params.old_files_pattern.empty())) {
		if (!params.error.empty()) {
			printf_s("\t%ls\n\n", params.error.c_str());
		}
		if (params.show_help) print_cmdl_usage(L"symdiff", options.data(), options.size(), diff_cmdl::default_option);
		return 0;
	}

	auto out = params.output_file;

	if (!create_data_source()) {
		fwprintf_s(out, L"\n error: can't initialize msdia library\n\n");
		return 0;
	}

	fwprintf_s(out, L"\n diff legends: +: added, -: removed, *: changed, $: changed (original)\n");

	const map<wstring, wstring> empty_files;
	diff_maps(params.new_file_groups, params.old_file_groups,
		[&](const wstring& group_name, const map<wstring, wstring>* new_files, const map<wstring, wstring>* old_files) {
			bool printed_group_name = false;
			wchar_t printed_group_prefix = L' ';
			auto print_group_name = [&](const wchar_t prefix) {
				if (!printed_group_name) {
					fwprintf_s(out, L"\n %lc %ls (\n", prefix, group_name.c_str());
					printed_group_name = true;
					printed_group_prefix = prefix;
				}
			};

			bool printed_previous_file_name = false;
			diff_maps(new_files ? *new_files : empty_files, old_files ? *old_files : empty_files,
				[&](const wstring& file_name, const wstring * new_file, const wstring * old_file) {
					bool printed_file_name = false;
					auto print_file_name = [&](const wchar_t prefix) {
						if (!printed_file_name) {
							print_group_name(new_files ? old_files ? L'*' : L'+' : L'-');
							if (printed_previous_file_name) {
								fwprintf_s(out, L"\n");
							}
							fwprintf_s(out, L"   %lc %ls\n", prefix, file_name.c_str());
							printed_previous_file_name = printed_file_name = true;
						}
					};

					if (new_file == nullptr) {
						print_file_name('-');
						return;
					}

					if (old_file == nullptr) {
						print_file_name('+');
						return;
					}
					sym_data new_data = load_sym_data(*new_file, new_sym_path);
					sym_data old_data = load_sym_data(*old_file, old_sym_path);
					if ((!new_data.loaded) || (!old_data.loaded)) {
						return;
					}

					auto diff = [&](const wchar_t * section, const set<wstring>& new_strings, const set<wstring>& old_strings) {
						bool printed_section = false;
						diff_sets(new_strings, old_strings,
							[&](const wstring * const new_string, const wstring * const old_string) {
								print_file_name('*');
								if (!printed_section) {
									fprintf_s(out, "     %ls\n", section);
									printed_section = true;
								}
								if (old_string == nullptr) {
									fprintf_s(out, "     + %ls\n", new_string->c_str());
								} else if (new_string == nullptr) {
									fprintf_s(out, "     - %ls\n", old_string->c_str());
								}
							}
						);
					};

					diff(L"[Compilands]", new_data.compilands, old_data.compilands);
					diff(L"[Functions]", new_data.functions, old_data.functions);
					diff(L"[GlobalData]", new_data.global_data, old_data.global_data);
					diff(L"[PublicSymbols]", new_data.public_symbols, old_data.public_symbols);

					auto diff_types = [&](const wchar_t * section, const map<wstring, set<wstring>>& new_types, const map<wstring, set<wstring>>& old_types) {
						bool printed_section = false;
						diff_maps(new_types, old_types,
							[&](const wstring& type_name, const set<wstring> * new_type_members, const set<wstring> * old_type_members) {
								bool printed_type_name = false;
								auto print_type_name = [&](const wchar_t prefix) {
									if (!printed_type_name) {
										if (!printed_section) {
											print_file_name(L'*');
											fwprintf_s(out, L"     %ls\n", section);
											printed_section = true;
										}
										fwprintf_s(out, L"     %lc %ls\n", prefix, type_name.c_str());
										printed_type_name = true;
									}
								};
								if (new_type_members == nullptr) {
									print_type_name(L'-');
									return;
								} else if (old_type_members == nullptr) {
									print_type_name(L'+');
								}
								diff_sets(*new_type_members, old_type_members ? *old_type_members : set<wstring>(),
									[&](const wstring* new_member, const wstring* old_member) {
										print_type_name(L'*');
										if (old_member == nullptr) {
											fprintf_s(out, "       + %ls\n", new_member->c_str());
										} else if (new_member == nullptr) {
											fprintf_s(out, "       - %ls\n", old_member->c_str());
										}
									}
								);
							}
						);
					};

					diff_types(L"[UDTs]", new_data.udts, old_data.udts);
					diff_types(L"[Enums]", new_data.enums, old_data.enums);
				}
			);

			if (printed_group_name)
				fwprintf_s(out, L" %lc )\n", printed_group_prefix);
		}
	);

	fwprintf_s(out, L"\n");

	return 0;
}

_com_ptr_t<_com_IIID<IDiaDataSource, &__uuidof(IDiaDataSource)>> create_data_source() {
	_com_ptr_t<_com_IIID<IDiaDataSource, &__uuidof(IDiaDataSource)>> data_source;
	HRESULT hr = NoOleCoCreate(CLSID_DiaSource, IID_IDiaDataSource, (void**)&data_source);
	if (FAILED(hr)) hr = NoRegCoCreate(L"msdia140.dll", CLSID_DiaSource, IID_IDiaDataSource, (void**)&data_source);
	return data_source;
}

__int64 SizeOfFile(const wchar_t* name);

sym_data load_sym_data(const wstring& file, const wstring& sym_path) {
	sym_data ret;
	ret.loaded = false;

	wstring sym_file;
	if (_wcsicmp(PathFindExtensionW(file.c_str()), L".pdb") == 0) {
		sym_file = file;
	} else {
		struct init_t {
			init_t() { SymSetOptions(SymGetOptions() | SYMOPT_EXACT_SYMBOLS | SYMOPT_UNDNAME); }
		} static init;

		wchar_t buffer[MAX_PATH] = {};
		if (SymGetSymbolFileW(NULL, sym_path.c_str(), file.c_str(), sfPdb, buffer, _countof(buffer), nullptr, 0) != FALSE) {
			sym_file = buffer;
		}
	}

	if (!sym_file.empty()) {
		if (SizeOfFile(sym_file.c_str()) > 100 * 1024 * 1024) {
			return ret;	// skip Windows.UI.Xaml.pdb for now
		}

		auto data_source = create_data_source();
		if (data_source && SUCCEEDED(data_source->loadDataFromPdb(sym_file.c_str()))) {
			ret.loaded = true;

			_com_ptr_t<_com_IIID<IDiaSession, &__uuidof(IDiaSession)>> session;
			data_source->openSession(&session);

			_com_ptr_t<_com_IIID<IDiaSymbol, &__uuidof(IDiaSymbol)>> global;
			session->get_globalScope(&global);

			const enum SymTagEnum tags[] = { SymTagCompiland, SymTagFunction, SymTagData,
				SymTagPublicSymbol, SymTagUDT, SymTagEnum };

			for (auto tag : tags) {
				_com_ptr_t<_com_IIID<IDiaEnumSymbols, &__uuidof(IDiaEnumSymbols)>> symbols;
				global->findChildren(tag, NULL, nsNone, &symbols);

				if (!symbols) continue;

				_com_ptr_t<_com_IIID<IDiaSymbol, &__uuidof(IDiaSymbol)>> symbol;
				ULONG celt = 0;
				while (SUCCEEDED(symbols->Next(1, &symbol, &celt)) && (celt == 1)) {
					_bstr_t name;
					symbol->get_undecoratedName(name.GetAddress());
					if (!name) symbol->get_name(name.GetAddress());
					if (!!name) {
						switch (tag) {
						case SymTagCompiland:
							ret.compilands.insert((const wchar_t*)name); break;
						case SymTagFunction:
							if (wcsstr((const wchar_t*)name, L"<lambda_") == nullptr) { // remove C++ lambdas
								ret.functions.insert((const wchar_t*)name);
							}
							break;
						case SymTagData:
							ret.global_data.insert((const wchar_t*)name); break;
						case SymTagPublicSymbol:
							ret.public_symbols.insert((const wchar_t*)name); break;
						case SymTagUDT:
						case SymTagEnum:
							auto& children = (tag == SymTagUDT) ?
								ret.udts[(const wchar_t*)name] :
								ret.enums[(const wchar_t*)name];

							_com_ptr_t<_com_IIID<IDiaEnumSymbols, &__uuidof(IDiaEnumSymbols)>> child_symbols;
							symbol->findChildren(SymTagNull, NULL, nsNone, &child_symbols);

							_com_ptr_t<_com_IIID<IDiaSymbol, &__uuidof(IDiaSymbol)>> child_sym;
							ULONG child_celt = 0;
							while (SUCCEEDED(child_symbols->Next(1, &child_sym, &child_celt)) && (child_celt == 1)) {
								_bstr_t child_name;
								child_sym->get_undecoratedName(child_name.GetAddress());
								if (!child_name) child_sym->get_name(child_name.GetAddress());
								if (!!child_name) {
									children.insert((const wchar_t*)child_name);
								}
							}
						}
					}
				}
			}
		}
	}

	return ret;
}

__int64 SizeOfFile(const wchar_t* file)
{
	__int64 ret = -1;

	HANDLE h = CreateFile(file, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER size;
		if (GetFileSizeEx(h, &size) != FALSE) {
			ret = size.QuadPart;
		}
		CloseHandle(h);
	}

	return ret;
}