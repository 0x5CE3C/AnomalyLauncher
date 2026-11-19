#pragma once

#define WIN32_LEAN_AND_MEAN   
#define _CRT_SECURE_NO_WARNINGS 1

#include "targetver.h"

// Библиотеки C++
#include <Lmcons.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <thread>
#include <future>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <mutex>
#include <chrono>
#include <atomic>
#include <tuple>
#include <filesystem>
#include <iostream>
#include <locale>
#include <conio.h>

// Заголовки Windows API
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <process.h>
#include <shellapi.h>
//#include <Gdiplus.h>
#include <mmsystem.h>
#include <Wbemidl.h> 
#include <comdef.h>
#include <Wininet.h>

// Дополнительные заголовки WINAPI
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <codecvt>
#include "psapi.h"
#include <time.h>
#include <io.h>
#include <xthreads.h>
#include <immintrin.h>
#define _WIN32_DCOM 

using namespace std::chrono_literals;
using namespace std;