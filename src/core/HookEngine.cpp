#include "core/HookEngine.hpp"
#include "../minhook/MinHook.h"
#include "core/Logger.hpp"
#include <string>

namespace Inkwell::Core
{
    auto HookEngine::Get() -> HookEngine&
    {
        static HookEngine instance;
        return instance;
    }

    auto HookEngine::Initialize() -> bool
    {
        if ( m_initialized )
            return true;

        if ( MH_Initialize() != MH_OK )
        {
            Logger::Error( "MinHook initialization failed." );
            return false;
        }

        m_initialized = true;
        return true;
    }

    auto HookEngine::Shutdown() -> void
    {
        if ( m_initialized )
        {
            MH_DisableHook( MH_ALL_HOOKS );
            MH_Uninitialize();
            m_initialized = false;
        }
    }

    auto HookEngine::InstallDetour( void* target_fn , void* detour_fn , void** out_original ) -> bool
    {
        if ( !target_fn || !detour_fn || !out_original )
            return false;

        if ( !m_initialized && !Initialize() )
            return false;

        auto status = MH_CreateHook( target_fn , detour_fn , out_original );
        if ( status != MH_OK && status != MH_ERROR_ALREADY_CREATED )
        {
            Logger::Error( "MH_CreateHook failed with status code " + std::to_string( status ) );
            return false;
        }

        status = MH_EnableHook( target_fn );
        if ( status != MH_OK )
        {
            Logger::Error( "MH_EnableHook failed with status code " + std::to_string( status ) );
            return false;
        }

        return true;
    }

    auto HookEngine::RemoveDetour( void* target_fn ) -> bool
    {
        if ( !m_initialized || !target_fn )
            return false;

        return ( MH_DisableHook( target_fn ) == MH_OK );
    }

    auto HookEngine::EnableAll() -> bool
    {
        if ( !m_initialized )
            return false;

        return ( MH_EnableHook( MH_ALL_HOOKS ) == MH_OK );
    }

    auto HookEngine::RemoveAll() -> void
    {
        if ( m_initialized )
        {
            MH_DisableHook( MH_ALL_HOOKS );
        }
    }
}
