#pragma once

#include "core/Math.hpp"
#include <cstdint>

namespace Inkwell::Source2
{
    enum InputBitMask_t : uint64_t
    {
        IN_NONE = 0x0000000000000000 ,
        IN_ALL = 0xFFFFFFFFFFFFFFFF ,
        IN_ATTACK = 0x0000000000000001 ,
        IN_JUMP = 0x0000000000000002 ,
        IN_DUCK = 0x0000000000000004 ,
        IN_FORWARD = 0x0000000000000008 ,
        IN_BACK = 0x0000000000000010 ,
        IN_USE = 0x0000000000000020 ,
        IN_TURNLEFT = 0x0000000000000080 ,
        IN_TURNRIGHT = 0x0000000000000100 ,
        IN_MOVELEFT = 0x0000000000000200 ,
        IN_MOVERIGHT = 0x0000000000000400 ,
        IN_ATTACK2 = 0x0000000000000800 ,
        IN_RELOAD = 0x0000000000002000 ,
        IN_SPEED = 0x0000000000010000 ,
        IN_WEAPON1 = 0x0000000100000000 ,
        IN_ABILITY1 = 0x0000000200000000 ,
        IN_ABILITY2 = 0x0000000400000000 ,
        IN_ABILITY3 = 0x0000000800000000 ,
        IN_ABILITY4 = 0x0000001000000000 ,
        IN_ZIPLINE = 0x0200000000000000 ,
        IN_CANCEL_ABILITY = 0x0100000000000000 ,
    };

    class CInButtonState
    {
    public:
        void* vtable;
        uint64_t buttonstate1;
        uint64_t buttonstate2;
        uint64_t buttonstate3;

        inline auto IsDown( InputBitMask_t bit ) const -> bool
        {
            return ( buttonstate1 & bit ) != 0;
        }

        inline auto SetDown( InputBitMask_t bit ) -> void
        {
            buttonstate1 |= bit;
        }

        inline auto Clear( InputBitMask_t bit ) -> void
        {
            buttonstate1 &= ~bit;
        }
    };

    struct CBaseUserCmdPB
    {
        uint8_t pad_00[0x18];
        uint32_t sequence_number;
        uint32_t tick_count;
        QAngle viewangles;
    };

    struct CCitadelUserCmdPB
    {
        uint8_t pad_00[0x20];
        CBaseUserCmdPB base;
        Vector3 vec_camera_position;
    };

#pragma pack(push, 1)
    class CUserCmd
    {
    public:
        uint8_t pad_00[0x10];
        CCitadelUserCmdPB cmd;
        CInButtonState button_states;
        uint8_t pad_01[0x18];
    };
#pragma pack(pop)

    class CCitadelInput
    {
    public:
        virtual void* Placeholder0() = 0;
    };
}
