#pragma once
#include <string>

namespace DotShader::Helpers {

std::wstring ascii_to_w(const std::string& ascii) {
    return std::wstring(ascii.begin(), ascii.end());
}

}