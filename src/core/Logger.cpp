#include "core/Logger.hpp"
#include <sstream>

namespace Inkwell
{
    static Settings g_settings;

    auto GetConfig() -> Settings&
    {
        return g_settings;
    }

    auto Logger::Init() -> void
    {
        std::scoped_lock lock( s_log_mutex );
        char temp_path[MAX_PATH];
        GetTempPathA( MAX_PATH , temp_path );
        std::string log_filepath = std::string( temp_path ) + "inkwell_deadlock.log";

        s_log_file.open( log_filepath , std::ios::out | std::ios::trunc );
        if ( s_log_file.is_open() )
        {
            s_log_file << "=========================================================================\n";
            s_log_file << "  ___ _   _ _  ____          _______ _     _     \n";
            s_log_file << " |_ _| \\ | | |/ /\\ \\        / / ____| |   | |    \n";
            s_log_file << "  | ||  \\| | ' /  \\ \\  /\\  / /|  _| | |   | |    \n";
            s_log_file << "  | || |\\  | . \\   \\ \\/  \\/ / | |___| |___| |___ \n";
            s_log_file << " |___|_| \\_|_|\\_\\   \\_/\\_/  |_________|_________|\n";
            s_log_file << "\n";
            s_log_file << "  INKWELL CORE :: Citadel Instrumentation Engine (Source 2)\n";
            s_log_file << "  Theme: Purple & Black Neon Glass (#9447F2)\n";
            s_log_file << "  License: " << LICENSE_TAG << " | Build: " << PROJECT_VERSION << " (" << BUILD_TAG << ")\n";
            s_log_file << "  Mode: SECURE / LIVE MATCHMAKING STREAM-SAFE\n";
            s_log_file << "=========================================================================\n\n";
            s_log_file.flush();
        }

        OutputDebugStringA( "[Inkwell] Citadel Core v3.2 Initialized [Purple & Black Suite]\n" );
    }

    auto Logger::Shutdown() -> void
    {
        std::scoped_lock lock( s_log_mutex );
        if ( s_log_file.is_open() )
        {
            s_log_file << "[Inkwell] Session Terminated Cleanly.\n";
            s_log_file.close();
        }
    }

    auto Logger::Log( std::string_view tag , std::string_view message ) -> void
    {
        std::scoped_lock lock( s_log_mutex );
        std::string formatted = "[" + std::string( tag ) + "] " + std::string( message ) + "\n";
        if ( s_log_file.is_open() )
        {
            s_log_file << formatted;
            s_log_file.flush();
        }
        OutputDebugStringA( formatted.c_str() );
    }

    auto Logger::Success( std::string_view message ) -> void
    {
        Log( "+" , message );
    }

    auto Logger::Warn( std::string_view message ) -> void
    {
        Log( "!" , message );
    }

    auto Logger::Error( std::string_view message ) -> void
    {
        Log( "-" , message );
    }
}
