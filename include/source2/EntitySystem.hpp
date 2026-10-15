#pragma once

#include <windows.h>
#include "core/MemoryEngine.hpp"
#include "source2/SchemaEngine.hpp"
#include "core/Math.hpp"
#include <cstdint>

namespace Inkwell::Source2
{
    constexpr uint32_t INVALID_EHANDLE_INDEX = 0xFFFFFFFF;

    struct CEntityHandle
    {
        uint32_t index = INVALID_EHANDLE_INDEX;

        inline auto IsValid() const -> bool { return index != INVALID_EHANDLE_INDEX; }
        inline auto GetEntryIndex() const -> uint32_t { return index & 0x7FFF; }
        inline auto GetSerialNumber() const -> uint32_t { return index >> 15; }
        inline auto operator==( const CEntityHandle& other ) const -> bool { return index == other.index; }
    };

    class C_BaseEntity;

    class CEntityIdentity
    {
    public:
        C_BaseEntity* entity;          // 0x00
        void* unk_08;                  // 0x08
        CEntityHandle handle;          // 0x10
        int32_t name_stringable_idx;   // 0x14
        const char* name;              // 0x18 (CUtlSymbolLarge)
        const char* designer_name;     // 0x20 (CUtlSymbolLarge)
        uint32_t flags;                // 0x30

        inline auto GetDesignerName() const -> const char*
        {
            return designer_name ? designer_name : "";
        }

        inline auto GetName() const -> const char*
        {
            return name ? name : "";
        }
    };

    class CEntityInstance
    {
    public:
        virtual ~CEntityInstance() = default;

        inline auto GetIdentity() -> CEntityIdentity*
        {
            return *reinterpret_cast<CEntityIdentity**>( reinterpret_cast<uintptr_t>( this ) + 0x10 );
        }
    };

    class CSkeletonInstance;

    class CGameSceneNode
    {
    public:
        INK_SCHEMA( CGameSceneNode , m_vecAbsOrigin , Vector3 );
        INK_SCHEMA( CGameSceneNode , m_angAbsRotation , QAngle );
        INK_SCHEMA( CGameSceneNode , m_bDormant , bool );

        inline auto GetSkeletonInstance() -> CSkeletonInstance*
        {
            return reinterpret_cast<CSkeletonInstance*>( this );
        }
    };

    class C_BaseEntity : public CEntityInstance
    {
    public:
        INK_SCHEMA( C_BaseEntity , m_iHealth , int32_t );
        INK_SCHEMA( C_BaseEntity , m_iMaxHealth , int32_t );
        INK_SCHEMA( C_BaseEntity , m_iTeamNum , uint8_t );
        INK_SCHEMA( C_BaseEntity , m_lifeState , uint8_t );
        INK_PSCHEMA( C_BaseEntity , m_pGameSceneNode , CGameSceneNode );

        inline auto IsAlive() -> bool { return m_iHealth() > 0 && m_lifeState() == 0; }
    };

    class CCitadelPlayerPawn : public C_BaseEntity
    {
    public:
        INK_SCHEMA( C_CitadelPlayerPawn , m_angEyeAngles , QAngle );
    };

    class CCitadelPlayerController : public C_BaseEntity
    {
    public:
        INK_SCHEMA( CCitadelPlayerController , m_hHeroPawn , CEntityHandle );
    };

    class IGameEntitySystem
    {
    public:
        inline auto GetEntityByIndex( int index ) -> C_BaseEntity*
        {
            if ( index <= 0 || index > 0x7FFE )
                return nullptr;

            using GetBaseEntityFn = C_BaseEntity* ( __fastcall* )( void* , int );
            static GetBaseEntityFn s_pfn_get_base_entity = nullptr;
            static bool s_pattern_searched = false;

            if ( !s_pattern_searched )
            {
                auto fn = Core::MemoryEngine::FindPattern( "client.dll" , "4C 8D 49 ? 81 FA ? ? ? ? 77" );
                if ( fn )
                {
                    s_pfn_get_base_entity = reinterpret_cast<GetBaseEntityFn>( fn );
                }
                s_pattern_searched = true;
            }

            if ( s_pfn_get_base_entity )
            {
                __try
                {
                    return s_pfn_get_base_entity( this , index );
                }
                __except ( EXCEPTION_EXECUTE_HANDLER )
                {
                }
            }

            __try
            {
                const auto chunk_index = index >> 9;
                if ( chunk_index > 0x3F )
                    return nullptr;

                const auto* chunk_list = reinterpret_cast<const uintptr_t*>(
                    reinterpret_cast<uintptr_t>( this ) + 0x10 );
                const auto chunk_addr = chunk_list[chunk_index];
                if ( !chunk_addr )
                    return nullptr;

                // Native Deadlock client stride is 0x70 (imul rax, 0x70)
                const auto entry_addr = chunk_addr + ( ( index & 0x1FF ) * 0x70 );
                auto* identity = reinterpret_cast<CEntityIdentity*>( entry_addr );
                if ( identity && ( ( identity->handle.index & 0x7FFF ) == static_cast<uint32_t>( index ) ) )
                {
                    auto* ent = identity->entity;
                    if ( ent && reinterpret_cast<uintptr_t>( ent ) >= 0x10000 && reinterpret_cast<uintptr_t>( ent ) < 0x7FFFFFFFFFFF )
                    {
                        return ent;
                    }
                }

                // Stride 0x78 fallback for older versions
                const auto fallback_entry = chunk_addr + ( ( index & 0x1FF ) * 0x78 );
                auto* fb_identity = reinterpret_cast<CEntityIdentity*>( fallback_entry );
                if ( fb_identity && ( ( fb_identity->handle.index & 0x7FFF ) == static_cast<uint32_t>( index ) ) )
                {
                    auto* ent = fb_identity->entity;
                    if ( ent && reinterpret_cast<uintptr_t>( ent ) >= 0x10000 && reinterpret_cast<uintptr_t>( ent ) < 0x7FFFFFFFFFFF )
                    {
                        return ent;
                    }
                }

                return nullptr;
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
                return nullptr;
            }
        }
    };
}
