#pragma once

#include <windows.h>
#include <cstdint>

namespace Inkwell::Core
{
    class HookEngine
    {
    public:
        static auto Get() -> HookEngine&;

        auto Initialize() -> bool;
        auto Shutdown() -> void;

        auto InstallDetour( void* target_fn , void* detour_fn , void** out_original ) -> bool;
        auto RemoveDetour( void* target_fn ) -> bool;
        auto EnableAll() -> bool;
        auto RemoveAll() -> void;

    private:
        HookEngine() = default;
        ~HookEngine() { Shutdown(); }

        bool m_initialized = false;
    };
}
