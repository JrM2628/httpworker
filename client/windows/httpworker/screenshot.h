#pragma once

#include <wtypes.h>
#include "Gdiplus.h"
#include <vector>
#include <iostream>
#pragma comment(lib,"gdiplus.lib")

std::vector<BYTE> takeScreenshotAndSaveToMemory();