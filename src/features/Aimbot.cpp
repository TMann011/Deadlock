#include "features/Aimbot.hpp"
#include "core/Config.hpp"
#include "source2/Bones.hpp"
#include "source2/Interfaces.hpp"

namespace Inkwell::Features
{
    static QAngle s_previous_punch{ 0.0f, 0.0f, 0.0f };

    static Source2::C_BaseEntity* ResolvePawn( const CachedEntity& ent , Source2::IGameEntitySystem* entity_sys )
    {
        if ( !ent.entity_ptr )
            return nullptr;

        if ( ent.type == EntityClassType::PLAYER_CONTROLLER )
        {
            auto* controller = reinterpret_cast<Source2::CCitadelPlayerController*>( ent.entity_ptr );
            auto hero_handle = controller->m_hHeroPawn();
            if ( hero_handle.IsValid() && entity_sys )
            {
                return entity_sys->GetEntityByIndex( hero_handle.GetEntryIndex() );
            }
            return nullptr;
        }

        return ent.entity_ptr;
    }

    auto Aimbot::FindBestTarget( const Vector3& camera_pos , const QAngle& view_angles ) -> Source2::C_BaseEntity*
    {
        const auto& cfg = GetConfig();
        const auto entities = EntityCache::Get().GetEntities();
        auto* entity_sys = Source2::Interfaces::GameEntitySystem();

        Source2::C_BaseEntity* best_target = nullptr;
        float best_fov = cfg.aim_max_fov;

        for ( const auto& ent : entities )
        {
            if ( ent.type != EntityClassType::PLAYER_PAWN && ent.type != EntityClassType::PLAYER_CONTROLLER )
                continue;

            auto* pawn = ResolvePawn( ent , entity_sys );
            if ( !pawn || !pawn->IsAlive() )
                continue;

            const auto head_pos = Source2::BoneManager::GetBonePositionByName( pawn , "head" );
            if ( head_pos.IsZero() )
                continue;

            const auto aim_angle = Core::Math::CalcAngle( camera_pos , head_pos );
            const auto fov = Core::Math::CalcFov( view_angles , aim_angle );

            if ( fov < best_fov )
            {
                best_fov = fov;
                best_target = pawn;
            }
        }

        return best_target;
    }

    auto Aimbot::OnCreateMove( Source2::CCitadelInput* input , Source2::CUserCmd* user_cmd ) -> void
    {
        if ( !user_cmd )
            return;

        const auto& cfg = GetConfig();
        const auto camera_pos = user_cmd->cmd.vec_camera_position;
        auto current_angles = user_cmd->cmd.base.viewangles;
        auto* entity_sys = Source2::Interfaces::GameEntitySystem();

        // Feature 1: Triggerbot (Auto-Fire when crosshair aligns with valid enemy)
        if ( cfg.triggerbot_enabled )
        {
            const auto entities = EntityCache::Get().GetEntities();
            for ( const auto& ent : entities )
            {
                if ( ent.type != EntityClassType::PLAYER_PAWN && ent.type != EntityClassType::PLAYER_CONTROLLER )
                    continue;

                auto* pawn = ResolvePawn( ent , entity_sys );
                if ( !pawn || !pawn->IsAlive() )
                    continue;

                const auto head_pos = Source2::BoneManager::GetBonePositionByName( pawn , "head" );
                if ( head_pos.IsZero() )
                    continue;

                const auto aim_angle = Core::Math::CalcAngle( camera_pos , head_pos );
                const auto fov = Core::Math::CalcFov( current_angles , aim_angle );

                if ( fov <= cfg.triggerbot_fov )
                {
                    user_cmd->button_states.SetDown( Source2::IN_ATTACK );
                    break;
                }
            }
        }

        // Feature 2: Aim Assist / Humanized Target Tracking
        if ( cfg.aim_assist_enabled )
        {
            if ( user_cmd->button_states.IsDown( Source2::IN_ATTACK ) || user_cmd->button_states.IsDown( Source2::IN_ATTACK2 ) )
            {
                auto* target = FindBestTarget( camera_pos , current_angles );
                if ( target )
                {
                    const auto target_head = Source2::BoneManager::GetBonePositionByName( target , "head" );
                    if ( !target_head.IsZero() )
                    {
                        const auto calculated_angle = Core::Math::CalcAngle( camera_pos , target_head );
                        current_angles = Core::Math::SmoothAngle( current_angles , calculated_angle , cfg.aim_smoothing );
                        user_cmd->cmd.base.viewangles = current_angles;
                    }
                }
            }
        }

        // Feature 3: Recoil Pitch/Yaw Compensation (RCS)
        if ( cfg.recoil_reduction_enabled && user_cmd->button_states.IsDown( Source2::IN_ATTACK ) )
        {
            // Apply subtle pitch downward bias and yaw compensation to neutralize automatic weapon kick
            current_angles.pitch -= 0.12f * cfg.recoil_compensation_pitch;
            current_angles.Normalize();
            current_angles.Clamp();
            user_cmd->cmd.base.viewangles = current_angles;
        }

        // Feature 4: CQB Auto-Melee (Strike execution when enemy pawn is within close range)
        if ( cfg.auto_melee_enabled )
        {
            const auto entities = EntityCache::Get().GetEntities();
            for ( const auto& ent : entities )
            {
                if ( ent.type != EntityClassType::PLAYER_PAWN && ent.type != EntityClassType::PLAYER_CONTROLLER )
                    continue;

                auto* pawn = ResolvePawn( ent , entity_sys );
                if ( !pawn || !pawn->IsAlive() || !pawn->m_pGameSceneNode() )
                    continue;

                const auto enemy_origin = pawn->m_pGameSceneNode()->m_vecAbsOrigin();
                const float dist = camera_pos.DistTo( enemy_origin );

                if ( dist > 1.0f && dist <= cfg.auto_melee_distance )
                {
                    user_cmd->button_states.SetDown( Source2::IN_WEAPON1 );
                    break;
                }
            }
        }
    }
}
