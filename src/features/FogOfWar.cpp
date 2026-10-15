#include "features/FogOfWar.hpp"
#include "core/Config.hpp"
#include <cstring>
#include <chrono>

namespace Inkwell::Features
{
    template <typename T>
    struct CUtlVector
    {
        int32_t size;
        T* elements;

        inline auto Count() const -> int32_t { return size; }
        inline auto operator[]( int32_t i ) -> T& { return elements[i]; }
    };

    struct CFOWEntity
    {
        uint8_t pad_00[0x30];
        bool is_visible_on_map;
    };

    class C_CitadelTeam : public Source2::C_BaseEntity
    {
    public:
        INK_SCHEMA( C_CitadelTeam , m_vecFOWEntities , CUtlVector<CFOWEntity> );
    };

    auto FogOfWarManager::RevealMinimap( Source2::IGameEntitySystem* entity_system ) -> void
    {
        if ( !entity_system || !GetConfig().unlock_minimap_fow )
            return;

        static auto s_last_fow = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if ( std::chrono::duration_cast<std::chrono::milliseconds>( now - s_last_fow ).count() < 100 )
            return;
        s_last_fow = now;

        constexpr int MAX_ENTITIES = 2048;
        for ( int i = 1; i < MAX_ENTITIES; ++i )
        {
            auto* entity = entity_system->GetEntityByIndex( i );
            if ( !entity || !entity->GetIdentity() )
                continue;

            const char* name = entity->GetIdentity()->GetName();
            const char* dname = entity->GetIdentity()->GetDesignerName();

            if ( ( name && std::strstr( name , "team" ) != nullptr ) || 
                 ( dname && std::strstr( dname , "team" ) != nullptr ) )
            {
                auto* team = reinterpret_cast<C_CitadelTeam*>( entity );
                auto& fow_vec = team->m_vecFOWEntities();

                for ( int f = 0; f < fow_vec.Count(); ++f )
                {
                    fow_vec[f].is_visible_on_map = true;
                }
            }
        }
    }
}
