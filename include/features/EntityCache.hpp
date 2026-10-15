#pragma once

#include "source2/EntitySystem.hpp"
#include <vector>
#include <mutex>

namespace Inkwell::Features
{
    enum class EntityClassType : uint32_t
    {
        UNKNOWN = 0 ,
        PLAYER_CONTROLLER ,
        PLAYER_PAWN ,
        NPC_TROOPER ,
        NPC_TROOPER_NEUTRAL ,
        ITEM_ORB ,
        PROJECTILE_HOOK
    };

    struct CachedEntity
    {
        Source2::CEntityHandle handle;
        EntityClassType type = EntityClassType::UNKNOWN;
        Source2::C_BaseEntity* entity_ptr = nullptr;
    };

    class EntityCache
    {
    public:
        static auto Get() -> EntityCache&;

        auto OnAddEntity( Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void;
        auto OnRemoveEntity( Source2::CEntityInstance* inst , Source2::CEntityHandle handle ) -> void;
        auto UpdateLiveEntities( Source2::IGameEntitySystem* entity_system ) -> void;
        auto Clear() -> void;

        auto GetEntities() -> std::vector<CachedEntity>;

    private:
        EntityCache() = default;

        static auto ClassifyEntity( Source2::C_BaseEntity* ent ) -> EntityClassType;

        std::vector<CachedEntity> m_entries;
        std::mutex m_mutex;
    };
}
