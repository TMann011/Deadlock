#pragma once

#include <windows.h>
#include <string_view>
#include <fstream>
#include <mutex>
#include "core/Config.hpp"

namespace Inkwell
{
    class Logger
    {
    public:
        static auto Init() -> void;
        static auto Shutdown() -> void;
        static auto Log( std::string_view tag , std::string_view message ) -> void;
        static auto Success( std::string_view message ) -> void;
        static auto Warn( std::string_view message ) -> void;
        static auto Error( std::string_view message ) -> void;

    private:
        static inline std::ofstream s_log_file;
        static inline std::mutex s_log_mutex;
    };
}
