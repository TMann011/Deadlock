#include "core/MemoryEngine.hpp"
#include <sstream>

namespace Inkwell::Core
{
    static auto PatternToBytes( std::string_view pattern , std::vector<int>& bytes ) -> void
    {
        bytes.clear();
        std::istringstream stream{ std::string( pattern ) };
        std::string token;

        while ( stream >> token )
        {
            if ( token == "?" || token == "??" )
            {
                bytes.push_back( -1 );
            }
            else
            {
                bytes.push_back( static_cast<int>( std::strtoul( token.c_str() , nullptr , 16 ) ) );
            }
        }
    }

    auto MemoryEngine::FindPattern( uintptr_t start_addr , uintptr_t end_addr , std::string_view pattern , uint32_t offset ) -> void*
    {
        std::vector<int> pattern_bytes;
        PatternToBytes( pattern , pattern_bytes );

        if ( pattern_bytes.empty() || start_addr >= end_addr )
            return nullptr;

        const auto* scan_start = reinterpret_cast<const uint8_t*>( start_addr );
        const auto* scan_end = reinterpret_cast<const uint8_t*>( end_addr );
        const auto pattern_size = pattern_bytes.size();

        for ( const auto* current = scan_start; current <= ( scan_end - pattern_size ); ++current )
        {
            bool found = true;
            for ( size_t i = 0; i < pattern_size; ++i )
            {
                if ( pattern_bytes[i] != -1 && current[i] != static_cast<uint8_t>( pattern_bytes[i] ) )
                {
                    found = false;
                    break;
                }
            }

            if ( found )
                return const_cast<void*>( reinterpret_cast<const void*>( current + offset ) );
        }

        return nullptr;
    }

    auto MemoryEngine::FindPattern( std::string_view module_name , std::string_view pattern , uint32_t offset ) -> void*
    {
        const auto module_handle = GetModuleHandleA( module_name.data() );
        if ( !module_handle )
            return nullptr;

        const auto* dos_header = reinterpret_cast<const IMAGE_DOS_HEADER*>( module_handle );
        if ( dos_header->e_magic != IMAGE_DOS_SIGNATURE )
            return nullptr;

        const auto* nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS64*>(
            reinterpret_cast<const uint8_t*>( dos_header ) + dos_header->e_lfanew );
        if ( nt_headers->Signature != IMAGE_NT_SIGNATURE )
            return nullptr;

        const auto start_addr = reinterpret_cast<uintptr_t>( module_handle ) + nt_headers->OptionalHeader.BaseOfCode;
        const auto end_addr = start_addr + nt_headers->OptionalHeader.SizeOfCode;

        return FindPattern( start_addr , end_addr , pattern , offset );
    }
}
