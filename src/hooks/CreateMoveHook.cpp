#include "hooks/CreateMoveHook.hpp"
#include "core/MemoryEngine.hpp"
#include "core/HookEngine.hpp"
#include "core/Logger.hpp"
#include "features/Aimbot.hpp"
#include "features/FogOfWar.hpp"
#include "source2/Interfaces.hpp"

namespace Inkwell::Hooks
{
    auto CreateMoveHook::Initialize() -> bool
    {
        // Pattern: 85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18 in client.dll
        auto* create_move_addr = Core::MemoryEngine::FindPattern(
            "client.dll" ,
            "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18" );

        if ( !create_move_addr )
        {
            Logger::Error( "Failed to find CCitadelInput::CreateMove signature." );
            return false;
        }

        auto& hook_mgr = Core::HookEngine::Get();
        if ( !hook_mgr.InstallDetour( create_move_addr , reinterpret_cast<void*>( &Hooked_CreateMove ) , reinterpret_cast<void**>( &s_original_create_move ) ) )
            return false;

        Logger::Success( "CCitadelInput::CreateMove hooked." );
        return true;
    }

    auto CreateMoveHook::Shutdown() -> void
    {
    }

    using GetUserCmdFn = Source2::CUserCmd* ( __fastcall* )( int );
    static GetUserCmdFn s_pfn_get_user_cmd = nullptr;
    static bool s_get_user_cmd_searched = false;

    auto __fastcall CreateMoveHook::Hooked_CreateMove( Source2::CCitadelInput* input , uint32_t split_screen_index , char a3 ) -> void
    {
        if ( s_original_create_move )
        {
            s_original_create_move( input , split_screen_index , a3 );
        }

        if ( !s_get_user_cmd_searched )
        {
            auto* fn = Core::MemoryEngine::FindPattern(
                "client.dll" ,
                "33 C0 83 F9 FF 0F 44 C8 48 63 C1 48 8D 0D" );
            if ( fn )
            {
                s_pfn_get_user_cmd = reinterpret_cast<GetUserCmdFn>( fn );
            }
            else
            {
                auto client_base = reinterpret_cast<uintptr_t>( GetModuleHandleA( "client.dll" ) );
                if ( client_base )
                {
                    s_pfn_get_user_cmd = reinterpret_cast<GetUserCmdFn>( client_base + 0x1680FE0 );
                }
            }
            s_get_user_cmd_searched = true;
        }

        Source2::CUserCmd* user_cmd = nullptr;
        if ( s_pfn_get_user_cmd )
        {
            __try
            {
                user_cmd = s_pfn_get_user_cmd( static_cast<int>( split_screen_index ) );
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
                user_cmd = nullptr;
            }
        }

        if ( user_cmd )
        {
            Features::Aimbot::OnCreateMove( input , user_cmd );
        }

        // Process minimap FOW disclosure
        Features::FogOfWarManager::RevealMinimap( Source2::Interfaces::GameEntitySystem() );
    }
}
