#pragma once

#include <windows.h>
#include <cstdint>
#include <string_view>
#include <vector>

namespace Inkwell::Core
{
    class MemoryEngine
    {
    public:
        static auto FindPattern( std::string_view module_name , std::string_view pattern , uint32_t offset = 0 ) -> void*;
        static auto FindPattern( uintptr_t start_addr , uintptr_t end_addr , std::string_view pattern , uint32_t offset = 0 ) -> void*;

        static inline auto ResolveRelativeCall( uintptr_t instruction_addr , uint32_t instruction_size = 5 ) -> void*
        {
            if ( !instruction_addr ) return nullptr;
            const auto displacement = *reinterpret_cast<const int32_t*>( instruction_addr + ( instruction_size - 4 ) );
            return reinterpret_cast<void*>( instruction_addr + instruction_size + displacement );
        }

        template <typename T = void*>
        static inline auto ResolveRelativeAddress( uintptr_t instruction_addr , uint32_t offset_to_displacement = 3 , uint32_t instruction_size = 7 ) -> T
        {
            if ( !instruction_addr ) return nullptr;
            const auto displacement = *reinterpret_cast<const int32_t*>( instruction_addr + offset_to_displacement );
            return reinterpret_cast<T>( instruction_addr + instruction_size + displacement );
        }

        template <typename T = void*>
        static inline auto GetVFunc( void* instance , uint32_t index ) -> T
        {
            if ( !instance ) return nullptr;
            const auto* vtable = *reinterpret_cast<void***>( instance );
            return reinterpret_cast<T>( vtable[index] );
        }
    };
}
