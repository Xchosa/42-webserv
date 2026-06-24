#pragma once

#include <map>
#include <string>
#include <iostream>

const std::string& getMimeType(std::string& suffix);
const std::string& getMimeTypeFromFile(std::string& path);
