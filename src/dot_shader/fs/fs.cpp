#include "fs.h"
#include <fstream>

namespace DotShader::FS {

std::optional<std::string> FS::read_whole_file(const std::string& path) {
    std::ifstream file{ path };

    if (!file.good())
        return std::nullopt;

    file.seekg(0, file.end);
    int fileLen = file.tellg();
    file.seekg(0, file.beg);

    if (!file.good() || !fileLen)
        return std::nullopt;

    std::string str(fileLen, '\0');
    file.read(str.data(), fileLen);

    if (!file.good())
        return std::nullopt;

    return std::move(str);
}

}