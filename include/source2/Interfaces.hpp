#pragma once

#include <windows.h>
#include <string_view>

using CreateInterfaceFn = void* ( * )( const char* pName , int* pReturnCode );

namespace Inkwell::Source2
{
    class ISchemaSystem;
    class IVEngineClient2;
    class IGameEntitySystem;
    class IInputSystem;

    class Interfaces
    {
    public:
        static auto Initialize() -> bool;

        static auto SchemaSystem() -> ISchemaSystem*;
        static auto EngineClient() -> IVEngineClient2*;
        static auto GameEntitySystem() -> IGameEntitySystem*;
        static auto InputSystem() -> IInputSystem*;

    private:
        static auto Capture( std::string_view module_name , std::string_view interface_name ) -> void*;

        static inline ISchemaSystem* s_schema_system = nullptr;
        static inline IVEngineClient2* s_engine_client = nullptr;
        static inline IGameEntitySystem* s_game_entity_system = nullptr;
        static inline IGameEntitySystem** s_game_entity_system_ptr = nullptr;
        static inline IInputSystem* s_input_system = nullptr;
    };
}
