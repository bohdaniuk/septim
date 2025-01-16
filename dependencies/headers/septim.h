#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <cstddef> // Pull in std::byte first
#include <windows.h> // Pull in Windows headers second
#include "sqliteFunc.h"
#include "reportFunc.h"
#include "util.h"
#include "httpFunc.h"
#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <ctime>
#include <Windows.h>
#include <curl/curl.h>
#include <json.hpp>
#include <iomanip>