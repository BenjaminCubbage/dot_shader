#include "env.h"

namespace DotShader::Env {

bool Env::populate_settings(const std::string& path) {
    m_settings = {};

    std::optional<std::string> str = FS::FS::read_whole_file(path);
    if (!str) {
        std::cout
            << "[Env]: Error: Failed to open or read from environment "
                "file: "
            << path
            << '\n'
            << "[Env]: Note: the environment file must be LF, not CRLF."
            << std::endl;
        return false;
    }

    enumerate_settings<decltype([](
        const std::string_view property, 
        const std::string_view value) {
        set_property(property, value);
    })>(*str);
    return check_properties();
}

bool Env::populate_settings_from_args(int argc, char** args) {
    if (argc < 2) {
        std::cout
            << "[Env]: Error: No environment file passed as command-line "
                "argument."
            << std::endl;
        return false;
    }

    return populate_settings(args[1]);
}

}