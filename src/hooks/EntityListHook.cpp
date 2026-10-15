#include "hooks/EntityListHook.hpp"
#include "core/MemoryEngine.hpp"
#include "core/HookEngine.hpp"
#include "core/Logger.hpp"
#include "features/EntityCache.hpp"

namespace Inkwell::Hooks
{
    auto EntityListHook::Initialize() -> bool
    {
        auto* add_addr = Core::MemoryEngine::FindPattern(
            "client.dll" ,
            "48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81" );

        auto* remove_addr = Core::MemoryEngine::FindPattern(
            "client.dll" ,
            "48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89" );

        if ( !add_addr || !remove_addr )
        {
            Logger::Error( "Failed to locate OnAddEntity / OnRemoveEntity signatures." );
            return false;
        }

        auto& hook_mgr = Core::HookEngine::Get();

        if ( !hook_mgr.InstallDetour( add_addr , reinterpret_cast<void*>( &Hooked_OnAddEntity ) , reinterpret_cast<void**>( &s_original_on_add_entity ) ) )
            return false;

        if ( !hook_mgr.InstallDetour( remove_addr , reinterpret_cast<void*>( &Hooked_OnRemoveEntity ) , reinterpret_cast<void**>( &s_original_on_remove_entity ) ) )
            return false;

        Logger::Success( "OnAddEntity and OnRemoveEntity hooks installed." );
        return true;
    }

    auto EntityListHook::Shutdown() -> void
    {
    }

    auto __fastcall EntityListHook::Hooked_OnAddEntity( void* entity_system , Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void
    {
        Features::EntityCache::Get().OnAddEntity( inst , handle );

        if ( s_original_on_add_entity )
        {
            s_original_on_add_entity( entity_system , inst , handle );
        }
    }

    auto __fastcall EntityListHook::Hooked_OnRemoveEntity( void* entity_system , Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void
    {
        Features::EntityCache::Get().OnRemoveEntity( inst , handle );

        if ( s_original_on_remove_entity )
        {
            s_original_on_remove_entity( entity_system , inst , handle );
        }
    }
}
