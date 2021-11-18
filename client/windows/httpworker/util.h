#pragma once
#include <string>
#include <wtypes.h>

std::string encode(std::string key, std::string clear);
std::string decode(std::string key, std::string enc);