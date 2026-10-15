#include <windows.h>
#include "core/Logger.hpp"
#include "core/HookEngine.hpp"
#include "source2/Interfaces.hpp"
#include "source2/SchemaEngine.hpp"
#include "hooks/SteamOverlayHook.hpp"
#include "hooks/CreateMoveHook.hpp"
#include "hooks/EntityListHook.hpp"
#include <iomanip>
#include <sstream>

#pragma pack(push, 8)
struct ArgsBase
{
    long long size = 0;
};

struct ManualMapParam_t : public ArgsBase
{
    char m_DllDir[MAX_PATH] = { 0 };
};
#pragma pack(pop)

static HMODULE g_module_handle = nullptr;

static auto InternalInit( LPVOID reserved ) -> void
{
    Inkwell::Logger::Init();

    Inkwell::Logger::Log( "CORE" , "Verifying game module handles in memory..." );

    int wait_iterations = 0;
    while ( !GetModuleHandleA( "client.dll" ) ||
            !GetModuleHandleA( "engine2.dll" ) ||
            !GetModuleHandleA( "schemasystem.dll" ) )
    {
        Sleep( 250 );
        if ( ++wait_iterations > 120 )
        {
            Inkwell::Logger::Error( "Timeout waiting for core game modules (client/engine2/schemasystem)." );
            return;
        }
    }

    // Optional grace period for Steam overlay if available
    for ( int i = 0; i < 8 && !GetModuleHandleA( "gameoverlayrenderer64.dll" ); ++i )
    {
        Sleep( 250 );
    }

    Inkwell::Logger::Success( "Target engine modules confirmed loaded." );
    Sleep( 250 );

    if ( reserved )
    {
        auto* map_params = reinterpret_cast<ManualMapParam_t*>( reserved );
        Inkwell::Logger::Success( "Manual Mapping parameters parsed." );
    }

    // Step 1: Initialize MinHook trampoline engine
    Inkwell::Logger::Log( "INIT" , "Step 1: Initializing MinHook engine..." );
    if ( !Inkwell::Core::HookEngine::Get().Initialize() )
    {
        Inkwell::Logger::Error( "HookEngine initialization failed." );
        return;
    }
    Inkwell::Logger::Success( "MinHook engine ready." );

    // Step 2: Bind Source 2 interfaces
    Inkwell::Logger::Log( "INIT" , "Step 2: Binding Source 2 interfaces..." );
    if ( !Inkwell::Source2::Interfaces::Initialize() )
    {
        Inkwell::Logger::Error( "Source 2 interface initialization aborted." );
        return;
    }

    // Step 3: Initialize dynamic Schema system
    Inkwell::Logger::Log( "INIT" , "Step 3: Initializing SchemaEngine..." );
    if ( !Inkwell::Source2::SchemaEngine::Get().Initialize() )
    {
        Inkwell::Logger::Error( "SchemaEngine initialization failed." );
        return;
    }

    // Step 4: Install Steam Overlay renderer hooks (DX11 Present / ResizeBuffers)
    Inkwell::Logger::Log( "INIT" , "Step 4: Installing Steam Overlay hooks..." );
    if ( !Inkwell::Hooks::SteamOverlayHook::Initialize() )
    {
        Inkwell::Logger::Warn( "Steam Overlay hooks deferred." );
    }

    // Step 5: Install Entity lifecycle hooks
    Inkwell::Logger::Log( "INIT" , "Step 5: Installing EntityList hooks..." );
    if ( !Inkwell::Hooks::EntityListHook::Initialize() )
    {
        Inkwell::Logger::Warn( "EntityList hooks deferred." );
    }

    // Step 6: Install CreateMove hook
    Inkwell::Logger::Log( "INIT" , "Step 6: Installing CreateMove hook..." );
    if ( !Inkwell::Hooks::CreateMoveHook::Initialize() )
    {
        Inkwell::Logger::Warn( "CreateMove hooks deferred." );
    }

    Inkwell::Logger::Success( "Inkwell Core live session operational. All hooks engaged." );

    // Step 7: Export baseline live schema dump
    static auto GetDynamicSchemaDumpPath = []() -> std::string {
        char module_path[MAX_PATH] = { 0 };
        if ( g_module_handle && GetModuleFileNameA( g_module_handle , module_path , MAX_PATH ) )
        {
            std::string path( module_path );
            size_t last_slash = path.find_last_of( "\\/" );
            if ( last_slash != std::string::npos )
            {
                return path.substr( 0 , last_slash + 1 ) + "schema_dump_live.txt";
            }
        }
        char temp_path[MAX_PATH] = { 0 };
        GetTempPathA( MAX_PATH , temp_path );
        return std::string( temp_path ) + "schema_dump_live.txt";
    };

    Inkwell::Source2::SchemaEngine::Get().DumpSchemaToFile( GetDynamicSchemaDumpPath() );
}

static auto WINAPI KeybindListenerThread( LPVOID ) -> DWORD
{
    while ( true )
    {
        Sleep( 50 );

        // VK_INSERT is handled exclusively in Hooked_WndProc to eliminate race-condition double toggles

        // VK_F9: On-demand live schema dump refresh
        if ( GetAsyncKeyState( VK_F9 ) & 1 )
        {
            char module_path[MAX_PATH] = { 0 };
            std::string dump_path = "schema_dump_live.txt";
            if ( g_module_handle && GetModuleFileNameA( g_module_handle , module_path , MAX_PATH ) )
            {
                std::string path( module_path );
                size_t last_slash = path.find_last_of( "\\/" );
                if ( last_slash != std::string::npos )
                    dump_path = path.substr( 0 , last_slash + 1 ) + "schema_dump_live.txt";
            }
            Inkwell::Source2::SchemaEngine::Get().DumpSchemaToFile( dump_path );
        }

        // VK_END: Panic De-Instrumentation & Clean Library Exit
        if ( GetAsyncKeyState( VK_END ) & 1 )
        {
            Inkwell::Logger::Warn( "Panic key triggered! Restoring original instructions..." );
            Inkwell::Hooks::SteamOverlayHook::Shutdown();
            Inkwell::Hooks::CreateMoveHook::Shutdown();
            Inkwell::Hooks::EntityListHook::Shutdown();
            Inkwell::Core::HookEngine::Get().Shutdown();
            Inkwell::Logger::Success( "All hooks disabled cleanly. Releasing module..." );
            Inkwell::Logger::Shutdown();
            FreeLibraryAndExitThread( g_module_handle , 0 );
            return 0;
        }
    }
    return 0;
}

static auto FilterException( unsigned int code , struct _EXCEPTION_POINTERS* ep ) -> int
{
    std::stringstream ss;
    ss << "Exception intercepted: 0x" << std::hex << code;
    if ( ep && ep->ExceptionRecord )
    {
        ss << " at Address: 0x" << std::hex << reinterpret_cast<uintptr_t>( ep->ExceptionRecord->ExceptionAddress );
    }
    Inkwell::Logger::Error( ss.str() );
    return EXCEPTION_EXECUTE_HANDLER;
}

static auto WINAPI InkwellCoreThread( LPVOID reserved ) -> DWORD
{
    __try
    {
        InternalInit( reserved );
        CreateThread( nullptr , 0 , KeybindListenerThread , nullptr , 0 , nullptr );
    }
    __except ( FilterException( GetExceptionCode() , GetExceptionInformation() ) )
    {
    }

    return 0;
}

BOOL WINAPI DllMain( HINSTANCE instance , DWORD reason , LPVOID reserved )
{
    switch ( reason )
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( instance );
            g_module_handle = instance;
            CreateThread( nullptr , 0 , InkwellCoreThread , reserved , 0 , nullptr );
            break;

        case DLL_PROCESS_DETACH:
            Inkwell::Hooks::SteamOverlayHook::Shutdown();
            Inkwell::Hooks::CreateMoveHook::Shutdown();
            Inkwell::Hooks::EntityListHook::Shutdown();
            Inkwell::Core::HookEngine::Get().Shutdown();
            Inkwell::Logger::Shutdown();
            break;
    }

    return TRUE;
}
