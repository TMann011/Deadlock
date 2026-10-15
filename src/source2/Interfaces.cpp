#include "source2/Interfaces.hpp"
#include "core/MemoryEngine.hpp"
#include "core/Logger.hpp"

namespace Inkwell::Source2
{
    auto Interfaces::Capture( std::string_view module_name , std::string_view interface_name ) -> void*
    {
        const auto module_handle = GetModuleHandleA( module_name.data() );
        if ( !module_handle )
        {
            Logger::Error( "Module handle missing for " + std::string( module_name ) );
            return nullptr;
        }

        const auto create_interface = reinterpret_cast<CreateInterfaceFn>(
            GetProcAddress( module_handle , "CreateInterface" ) );

        if ( !create_interface )
        {
            Logger::Error( "CreateInterface export missing in " + std::string( module_name ) );
            return nullptr;
        }

        auto* iface = create_interface( interface_name.data() , nullptr );
        if ( !iface )
        {
            Logger::Error( "Failed to capture " + std::string( interface_name ) );
        }
        return iface;
    }

    auto Interfaces::Initialize() -> bool
    {
        Logger::Log( "INIT" , "Capturing SchemaSystem_001..." );
        s_schema_system = reinterpret_cast<ISchemaSystem*>(
            Capture( "schemasystem.dll" , "SchemaSystem_001" ) );

        Logger::Log( "INIT" , "Capturing Source2EngineToClient001..." );
        s_engine_client = reinterpret_cast<IVEngineClient2*>(
            Capture( "engine2.dll" , "Source2EngineToClient001" ) );

        Logger::Log( "INIT" , "Capturing InputSystemVersion001..." );
        s_input_system = reinterpret_cast<IInputSystem*>(
            Capture( "inputsystem.dll" , "InputSystemVersion001" ) );

        Logger::Log( "INIT" , "Scanning GameEntitySystem in client.dll..." );
        
        // Method 1: Deadlock verified dwEntityList signature (RVA 0x16E68AA -> 0x34C7430)
        auto entity_list_sig = reinterpret_cast<uintptr_t>(
            Core::MemoryEngine::FindPattern( "client.dll" , "48 8B 0D ? ? ? ? 48 89 7C 24 ? 8B FA C1 EB" ) );
        if ( entity_list_sig )
        {
            s_game_entity_system_ptr = Core::MemoryEngine::ResolveRelativeAddress<IGameEntitySystem**>( entity_list_sig , 3 , 7 );
            if ( s_game_entity_system_ptr )
            {
                Logger::Success( "GameEntitySystem resolved via dwEntityList pattern." );
            }
        }

        // Method 2: GetBaseEntity caller in Deadlock client.dll (RVA 0x4273D2 -> 0x3CB40C0)
        if ( !s_game_entity_system_ptr )
        {
            auto entity_sys_ptr = reinterpret_cast<uintptr_t>(
                Core::MemoryEngine::FindPattern( "client.dll" , "48 8B 0D ? ? ? ? F2 0F 11 44 24" ) );
            if ( entity_sys_ptr )
            {
                s_game_entity_system_ptr = Core::MemoryEngine::ResolveRelativeAddress<IGameEntitySystem**>( entity_sys_ptr , 3 , 7 );
                if ( s_game_entity_system_ptr )
                {
                    Logger::Success( "GameEntitySystem resolved via primary caller pattern." );
                }
            }
        }

        // Method 3: Direct verified RVAs fallback
        if ( !s_game_entity_system_ptr )
        {
            auto client_base = reinterpret_cast<uintptr_t>( GetModuleHandleA( "client.dll" ) );
            if ( client_base )
            {
                s_game_entity_system_ptr = reinterpret_cast<IGameEntitySystem**>( client_base + 0x34C7430 );
                Logger::Success( "GameEntitySystem bound via static verified RVA 0x34C7430." );
            }
        }

        if ( s_game_entity_system_ptr )
        {
            __try
            {
                if ( *s_game_entity_system_ptr )
                    s_game_entity_system = *s_game_entity_system_ptr;
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
            }
        }

        if ( s_schema_system && s_engine_client )
        {
            Logger::Success( "Source 2 interface factories bound." );
            return true;
        }

        Logger::Error( "Source 2 core interface binding failure." );
        return false;
    }

    auto Interfaces::SchemaSystem() -> ISchemaSystem* { return s_schema_system; }
    auto Interfaces::GameEntitySystem() -> IGameEntitySystem*
    {
        if ( s_game_entity_system_ptr )
        {
            uintptr_t ptr_val = reinterpret_cast<uintptr_t>( s_game_entity_system_ptr );
            if ( ptr_val >= 0x10000 && ptr_val < 0x7FFFFFFFFFFF )
            {
                __try
                {
                    if ( *s_game_entity_system_ptr )
                        return *s_game_entity_system_ptr;
                }
                __except ( EXCEPTION_EXECUTE_HANDLER )
                {
                }
            }
        }
        return s_game_entity_system;
    }

    auto Interfaces::EngineClient() -> IVEngineClient2* { return s_engine_client; }
    auto Interfaces::InputSystem() -> IInputSystem* { return s_input_system; }
}
