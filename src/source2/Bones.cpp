#include "source2/Bones.hpp"
#include "source2/EntitySystem.hpp"
#include "core/MemoryEngine.hpp"

namespace Inkwell::Source2
{
    using CalcBonesFn = void ( __fastcall* )( CSkeletonInstance* , uint32_t );
    static CalcBonesFn s_calc_bones = nullptr;

    static bool s_calc_bones_searched = false;

    auto CSkeletonInstance::CalcWorldSpaceBones( uint32_t flags ) -> void
    {
        if ( !s_calc_bones && !s_calc_bones_searched )
        {
            auto fn = Core::MemoryEngine::FindPattern(
                "client.dll" ,
                "48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81" );
            s_calc_bones = reinterpret_cast<CalcBonesFn>( fn );
            s_calc_bones_searched = true;
        }

        if ( s_calc_bones )
        {
            __try
            {
                s_calc_bones( this , flags );
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
            }
        }
    }

    auto CSkeletonInstance::GetBonePosition( int32_t bone_index , Vector3& out_pos ) -> bool
    {
        if ( bone_index < 0 || bone_index > 256 )
            return false;

        uintptr_t bone_array = 0;
        __try
        {
            bone_array = *reinterpret_cast<const uintptr_t*>( reinterpret_cast<uintptr_t>( this ) + 0x1C0 );
            if ( !bone_array || bone_array < 0x10000 )
            {
                bone_array = *reinterpret_cast<const uintptr_t*>( reinterpret_cast<uintptr_t>( this ) + 0x1E0 );
            }
        }
        __except ( EXCEPTION_EXECUTE_HANDLER )
        {
            return false;
        }

        if ( !bone_array || bone_array < 0x10000 )
            return false;

        // Source 2 CModelState CBoneData layout (32 bytes per bone)
        __try
        {
            struct BoneData_t { Vector3 pos; float scale; Vector3 rot; };
            const auto* bone_data = reinterpret_cast<const BoneData_t*>( bone_array + bone_index * sizeof( BoneData_t ) );
            if ( bone_data && !bone_data->pos.IsZero() )
            {
                out_pos = bone_data->pos;
                return true;
            }

            // Alternative 3x4 matrix representation (48 bytes per bone)
            const auto* bone_transform = reinterpret_cast<const float*>( bone_array + bone_index * 48 );
            out_pos.x = bone_transform[3];
            out_pos.y = bone_transform[7];
            out_pos.z = bone_transform[11];

            return !out_pos.IsZero();
        }
        __except ( EXCEPTION_EXECUTE_HANDLER )
        {
            return false;
        }
    }

    auto BoneManager::GetBonePositionByName( C_BaseEntity* entity , std::string_view bone_name ) -> Vector3
    {
        Vector3 pos{};
        if ( !entity || !entity->m_pGameSceneNode() )
            return pos;

        auto* skeleton = entity->m_pGameSceneNode()->GetSkeletonInstance();
        if ( skeleton )
        {
            skeleton->CalcWorldSpaceBones();

            int32_t bone_index = 0;
            if ( bone_name == "head" ) bone_index = 6;
            else if ( bone_name == "pelvis" ) bone_index = 2;
            else if ( bone_name == "spine_1" ) bone_index = 4;
            else if ( bone_name == "arm_lower_R" ) bone_index = 8;
            else if ( bone_name == "arm_lower_L" ) bone_index = 9;

            if ( skeleton->GetBonePosition( bone_index , pos ) && !pos.IsZero() )
                return pos;
        }

        // Resilient origin-offset fallback (maintains continuous visual tracking)
        const auto root = entity->m_pGameSceneNode()->m_vecAbsOrigin();
        if ( bone_name == "head" ) return root + Vector3{ 0.0f, 0.0f, 72.0f };
        if ( bone_name == "pelvis" ) return root + Vector3{ 0.0f, 0.0f, 36.0f };
        if ( bone_name == "spine_1" ) return root + Vector3{ 0.0f, 0.0f, 48.0f };
        return root;
    }

    auto BoneManager::GetHeroSkeletonBones() -> const std::vector<std::pair<std::string_view , std::string_view>>&
    {
        static const std::vector<std::pair<std::string_view , std::string_view>> bones = {
            { "head", "spine_1" },
            { "spine_1", "pelvis" },
            { "spine_1", "arm_lower_R" },
            { "spine_1", "arm_lower_L" }
        };
        return bones;
    }
}
