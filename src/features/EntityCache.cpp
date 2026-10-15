#include "features/EntityCache.hpp"
#include <algorithm>
#include <cstring>

namespace Inkwell::Features
{
    auto EntityCache::Get() -> EntityCache&
    {
        static EntityCache instance;
        return instance;
    }

    auto EntityCache::ClassifyEntity( Source2::C_BaseEntity* ent ) -> EntityClassType
    {
        if ( !ent || !ent->GetIdentity() )
            return EntityClassType::UNKNOWN;

        const char* d_name = ent->GetIdentity()->GetDesignerName();
        const char* e_name = ent->GetIdentity()->GetName();

        if ( !d_name || !*d_name )
            d_name = e_name;

        if ( !d_name || !*d_name )
            return EntityClassType::UNKNOWN;

        if ( std::strcmp( d_name , "player" ) == 0 || std::strstr( d_name , "player" ) != nullptr || std::strstr( d_name , "pawn" ) != nullptr || std::strstr( d_name , "hero" ) != nullptr )
        {
            if ( std::strstr( d_name , "controller" ) != nullptr )
                return EntityClassType::PLAYER_CONTROLLER;
            return EntityClassType::PLAYER_PAWN;
        }
        if ( std::strcmp( d_name , "citadel_player_controller" ) == 0 || std::strstr( d_name , "controller" ) != nullptr )
            return EntityClassType::PLAYER_CONTROLLER;
        if ( std::strcmp( d_name , "npc_trooper_neutral" ) == 0 || std::strstr( d_name , "trooper_neutral" ) != nullptr || std::strstr( d_name , "neutral" ) != nullptr )
            return EntityClassType::NPC_TROOPER_NEUTRAL;
        if ( std::strcmp( d_name , "npc_trooper" ) == 0 || std::strstr( d_name , "trooper" ) != nullptr )
            return EntityClassType::NPC_TROOPER;
        if ( std::strstr( d_name , "item_xp" ) != nullptr || std::strstr( d_name , "item" ) != nullptr || std::strstr( d_name , "orb" ) != nullptr )
            return EntityClassType::ITEM_ORB;
        if ( std::strstr( d_name , "hook" ) != nullptr || std::strstr( d_name , "projectile" ) != nullptr )
            return EntityClassType::PROJECTILE_HOOK;

        return EntityClassType::UNKNOWN;
    }

    auto EntityCache::OnAddEntity( Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void
    {
        if ( !inst || !inst->GetIdentity() || !inst->GetIdentity()->entity )
            return;

        std::scoped_lock lock( m_mutex );

        auto* ent = inst->GetIdentity()->entity;
        const auto type = ClassifyEntity( ent );

        if ( type == EntityClassType::UNKNOWN )
            return;

        auto it = std::find_if( m_entries.begin() , m_entries.end() , [handle]( const CachedEntity& e ) {
            return e.handle == handle;
        } );

        if ( it == m_entries.end() )
        {
            m_entries.push_back( { handle, type, ent } );
        }
        else
        {
            it->type = type;
            it->entity_ptr = ent;
        }
    }

    auto EntityCache::OnRemoveEntity( Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void
    {
        std::scoped_lock lock( m_mutex );

        auto it = std::remove_if( m_entries.begin() , m_entries.end() , [handle]( const CachedEntity& e ) {
            return e.handle == handle;
        } );

        if ( it != m_entries.end() )
        {
            m_entries.erase( it , m_entries.end() );
        }
    }

    auto EntityCache::UpdateLiveEntities( Source2::IGameEntitySystem* entity_system ) -> void
    {
        if ( !entity_system )
            return;

        std::scoped_lock lock( m_mutex );
        m_entries.clear();

        for ( int i = 1; i <= 1024; ++i )
        {
            auto* ent = entity_system->GetEntityByIndex( i );
            if ( !ent || !ent->GetIdentity() )
                continue;

            const auto type = ClassifyEntity( ent );
            if ( type == EntityClassType::UNKNOWN )
                continue;

            Source2::CEntityHandle handle = ent->GetIdentity()->handle;
            if ( !handle.IsValid() )
            {
                handle.index = static_cast<uint32_t>( i );
            }
            m_entries.push_back( { handle , type , ent } );
        }
    }

    auto EntityCache::Clear() -> void
    {
        std::scoped_lock lock( m_mutex );
        m_entries.clear();
    }

    auto EntityCache::GetEntities() -> std::vector<CachedEntity>
    {
        std::scoped_lock lock( m_mutex );
        return m_entries;
    }
}
