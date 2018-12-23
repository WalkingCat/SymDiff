#pragma once

#include <stdio.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <map>

#include "../witutils/diff_utils.h"
#include "../witutils/diff_commons.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

#include <dia2.h>
#include <diacreate.h>
#pragma comment(lib, "diaguids.lib")

#include <comip.h>
#pragma comment(lib, "comsupp.lib")