#pragma once

#include <string>
#include <vector>

namespace io {

void die(const char *s);

std::vector<std::string> readFileToVector(const std::string &filePath);

} // namespace io
