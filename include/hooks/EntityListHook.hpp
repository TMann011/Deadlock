#pragma once

#include "source2/EntitySystem.hpp"

namespace Inkwell::Hooks
{
    class EntityListHook
    {
    public:
        static auto Initialize() -> bool;
        static auto Shutdown() -> void;

        static auto __fastcall Hooked_OnAddEntity( void* entity_system , Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void;
        static auto __fastcall Hooked_OnRemoveEntity( void* entity_system , Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void;

    private:
        using OnAddEntityFn = void ( __fastcall* )( void* , Source2::CEntityInstance* , Source2::CEntityHandle );
        using OnRemoveEntityFn = void ( __fastcall* )( void* , Source2::CEntityInstance* , Source2::CEntityHandle );

        static inline OnAddEntityFn s_original_on_add_entity = nullptr;
        static inline OnRemoveEntityFn s_original_on_remove_entity = nullptr;
    };
}
