#pragma once

#include <string>
#include <vector>

namespace IO {

void die(const char *s);

std::vector<std::string> readFileToVector(const std::string &filePath);

} // namespace IO
