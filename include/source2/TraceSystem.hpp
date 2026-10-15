#pragma once

#include "core/Math.hpp"
#include <cstdint>

namespace Inkwell::Source2
{
    class C_BaseEntity;

    struct Ray_t
    {
        Vector3 start;
        Vector3 end;
        Vector3 mins;
        Vector3 maxs;
        uint8_t pad[0x20];
    };

    struct CGameTrace
    {
        void* surface;
        C_BaseEntity* hit_entity;
        void* hitbox;
        uint8_t pad_00[0x38];
        float fraction;
        uint8_t pad_01[0x20];

        inline auto IsVisible() const -> bool { return fraction >= 0.999f; }
    };

    class CTraceFilter
    {
    public:
        uint64_t trace_mask;
        const C_BaseEntity* skip_entity;
        uint8_t collision_group;
        uint8_t pad_00[0x37];

        CTraceFilter( uint64_t mask , const C_BaseEntity* skip , uint8_t group = 3 )
            : trace_mask( mask ) , skip_entity( skip ) , collision_group( group )
        {
        }
    };

    using TraceShapeFn = bool ( __fastcall* )( void* vphys_world , Ray_t* ray , const Vector3* start , const Vector3* end , CTraceFilter* filter , CGameTrace* trace );
}
