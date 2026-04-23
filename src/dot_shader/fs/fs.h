#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace DotShader::FS {

class FS {
  public:
    static inline const std::string& resources_dir() {
        static std::string dir;
        if (dir.size() == 0)
            dir = std::filesystem::current_path().string() + "/resources";
        return dir;
    }

    static inline std::string resource_full_path(std::string_view resource_name) {
        return resources_dir() + "/" + std::string(resource_name);
    }

    static std::optional<std::string> read_whole_file(const std::string& path);
};

}