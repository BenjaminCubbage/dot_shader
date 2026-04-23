#pragma once
#include <string>
#include <string_view>
#include "dot_shader/fs/fs.h"

namespace DotShader::Env {

struct EnvSettings {
    std::string resource_dir;
};

class Env {
  private:
    template<class T> requires
        requires (std::string_view sv) {
            T{}(sv, sv);
        }
    static bool enumerate_settings(std::string& str) {
        bool any{};
        size_t line_stt{};
        size_t line_eqs{};
        size_t line_end{};

        while (line_stt < str.size()) {
            if (str[line_stt] == '#')
                continue;

            for (line_eqs = line_stt;
                 line_eqs < str.size() && str[line_eqs] != '=' && str[line_eqs] != '\n';
                 ++line_eqs);

            for (line_end = line_eqs;
                 line_end < str.size() && str[line_end] != '\n';
                 ++line_end);

            if (line_eqs > line_stt &&
                line_end > line_eqs + 1) {
                any = true;
                T{}(std::string_view(&str[line_stt    ], line_eqs - line_stt),
                    std::string_view(&str[line_eqs + 1], line_end - line_eqs));
            }

            line_stt = line_end + 1;
        }

        return any;
    }

    static inline void set_property(const std::string_view property, const std::string_view value) {
        if (property.compare("RESOURCE_DIR") == 0)
            m_settings.resource_dir = std::string(value);
    }

    static inline bool check_properties() {
        if (m_settings.resource_dir.size() == 0) {
            std::cout 
                << "[Env]: Missing required property: RESOURCE_DIR"
                << std::endl;
            return false;
        }

        return true;
    }

  public:
    static bool populate_settings(const std::string& path);
    static bool populate_settings_from_args(int argc, char** args);

    static inline const EnvSettings& settings() {
        return m_settings;
    }

    static inline EnvSettings m_settings{};
};

}