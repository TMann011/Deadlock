#pragma once

#include "source2/UserCmd.hpp"

namespace Inkwell::Hooks
{
    class CreateMoveHook
    {
    public:
        static auto Initialize() -> bool;
        static auto Shutdown() -> void;

        static auto __fastcall Hooked_CreateMove( Source2::CCitadelInput* input , uint32_t split_screen_index , char a3 ) -> void;

    private:
        using CreateMoveFn = void ( __fastcall* )( Source2::CCitadelInput* , uint32_t , char );
        static inline CreateMoveFn s_original_create_move = nullptr;
    };
}
