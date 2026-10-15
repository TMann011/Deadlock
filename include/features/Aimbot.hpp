#pragma once

#include "source2/UserCmd.hpp"
#include "features/EntityCache.hpp"

namespace Inkwell::Features
{
    class Aimbot
    {
    public:
        static auto OnCreateMove( Source2::CCitadelInput* input , Source2::CUserCmd* user_cmd ) -> void;

    private:
        static auto FindBestTarget( const Vector3& camera_pos , const QAngle& view_angles ) -> Source2::C_BaseEntity*;
    };
}
