#pragma once

#include "core/Math.hpp"
#include <string_view>
#include <vector>
#include <utility>

namespace Inkwell::Source2
{
    class C_BaseEntity;

    class CSkeletonInstance
    {
    public:
        // Source 2 bone matrix calculation flags
        constexpr static uint32_t FLAG_BONE_CALC_ALL = 0x1FF;

        auto CalcWorldSpaceBones( uint32_t flags = FLAG_BONE_CALC_ALL ) -> void;
        auto GetBonePosition( int32_t bone_index , Vector3& out_pos ) -> bool;
    };

    class BoneManager
    {
    public:
        static auto GetBonePositionByName( C_BaseEntity* entity , std::string_view bone_name ) -> Vector3;

        // Skeleton topology for hero models
        static auto GetHeroSkeletonBones() -> const std::vector<std::pair<std::string_view , std::string_view>>&;
    };
}
