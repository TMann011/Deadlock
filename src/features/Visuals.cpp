#include "features/Visuals.hpp"
#include "ui/InkwellUI.hpp"
#include "source2/Bones.hpp"
#include "source2/Interfaces.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace Inkwell::Features
{
    auto Visuals::ResolveHeroName( Source2::C_BaseEntity* ent ) -> const char*
    {
        if ( !ent || !ent->GetIdentity() )
            return "Hero";

        const char* class_name = ent->GetIdentity()->GetName();
        const char* designer_name = ent->GetIdentity()->GetDesignerName();

        // Check designer name first, then class name
        auto check = [class_name, designer_name]( const char* pattern ) -> bool {
            return ( designer_name && std::strstr( designer_name , pattern ) != nullptr ) ||
                   ( class_name && std::strstr( class_name , pattern ) != nullptr );
        };

        if ( check( "bebop" ) ) return "Bebop";
        if ( check( "dynamo" ) ) return "Dynamo";
        if ( check( "grey_talon" ) || check( "archer" ) ) return "Grey Talon";
        if ( check( "haze" ) ) return "Haze";
        if ( check( "infernus" ) ) return "Infernus";
        if ( check( "ivy" ) || check( "tengu" ) ) return "Ivy";
        if ( check( "kelvin" ) || check( "frost" ) ) return "Kelvin";
        if ( check( "lady_geist" ) || check( "ghost" ) ) return "Lady Geist";
        if ( check( "mcginnis" ) || check( "forge" ) ) return "McGinnis";
        if ( check( "mirage" ) ) return "Mirage";
        if ( check( "mo_krill" ) || check( "digger" ) ) return "Mo & Krill";
        if ( check( "paradox" ) || check( "chrono" ) ) return "Paradox";
        if ( check( "pocket" ) ) return "Pocket";
        if ( check( "seven" ) || check( "storm" ) ) return "Seven";
        if ( check( "shiv" ) ) return "Shiv";
        if ( check( "vindicta" ) || check( "sniper" ) ) return "Vindicta";
        if ( check( "viscous" ) || check( "goo" ) ) return "Viscous";
        if ( check( "warden" ) ) return "Warden";
        if ( check( "wraith" ) ) return "Wraith";
        if ( check( "yamato" ) ) return "Yamato";

        return "Hero";
    }

    auto Visuals::RenderOffScreenIndicator( const Vector3& enemy_origin , const VMatrix& view_matrix , float screen_w , float screen_h , float distance_m ) -> void
    {
        const auto& cfg = GetConfig();
        const float center_x = screen_w * 0.5f;
        const float center_y = screen_h * 0.5f;
        const float radius = cfg.offscreen_radius;

        // Extract view vectors from matrix
        const float dir_x = enemy_origin.x * view_matrix.m[0][0] + enemy_origin.y * view_matrix.m[0][1] + enemy_origin.z * view_matrix.m[0][2];
        const float dir_y = enemy_origin.x * view_matrix.m[1][0] + enemy_origin.y * view_matrix.m[1][1] + enemy_origin.z * view_matrix.m[1][2];

        const float angle = std::atan2( -dir_y , dir_x );
        const float pt_x = center_x + std::cos( angle ) * radius;
        const float pt_y = center_y + std::sin( angle ) * radius;

        // Render diamond indicator
        UI::InkwellRenderer::DrawBox( pt_x - 5.0f , pt_y - 5.0f , 10.0f , 10.0f , cfg.color_accent );

        if ( cfg.esp_distance )
        {
            char dist_buf[16];
            snprintf( dist_buf , sizeof( dist_buf ) , "%.0fm" , distance_m );
            UI::InkwellRenderer::DrawText( pt_x + 8.0f , pt_y - 4.0f , dist_buf , cfg.color_accent_active );
        }
    }

    auto Visuals::Render( const VMatrix& view_matrix , float screen_w , float screen_h ) -> void
    {
        const auto& cfg = GetConfig();
        if ( !cfg.esp_enabled )
            return;

        // Synchronize entity cache at 10 Hz to maintain peak render framerates
        if ( auto* entity_sys = Source2::Interfaces::GameEntitySystem() )
        {
            static auto s_last_sync = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if ( std::chrono::duration_cast<std::chrono::milliseconds>( now - s_last_sync ).count() >= 100 )
            {
                EntityCache::Get().UpdateLiveEntities( entity_sys );
                s_last_sync = now;
            }
        }

        const auto entities = EntityCache::Get().GetEntities();

        for ( const auto& ent : entities )
        {
            if ( !ent.entity_ptr )
                continue;

            Source2::C_BaseEntity* render_ent = ent.entity_ptr;

            // In Deadlock, CCitadelPlayerController holds the m_hHeroPawn handle pointing to the spatial pawn
            if ( ent.type == EntityClassType::PLAYER_CONTROLLER )
            {
                auto* controller = reinterpret_cast<Source2::CCitadelPlayerController*>( ent.entity_ptr );
                auto hero_handle = controller->m_hHeroPawn();
                if ( hero_handle.IsValid() && Source2::Interfaces::GameEntitySystem() )
                {
                    auto* pawn = Source2::Interfaces::GameEntitySystem()->GetEntityByIndex( hero_handle.GetEntryIndex() );
                    if ( pawn )
                    {
                        render_ent = pawn;
                    }
                }
            }

            auto* scene_node = render_ent->m_pGameSceneNode();
            if ( !scene_node || scene_node->m_bDormant() )
                continue;

            // Health filter strictly applies to heroes and troopers; projectiles and orbs have no health
            if ( ent.type != EntityClassType::PROJECTILE_HOOK && ent.type != EntityClassType::ITEM_ORB )
            {
                if ( render_ent->m_iHealth() <= 0 )
                    continue;
            }

            CachedEntity render_cached = ent;
            render_cached.entity_ptr = render_ent;

            switch ( ent.type )
            {
                case EntityClassType::PLAYER_CONTROLLER:
                case EntityClassType::PLAYER_PAWN:
                    RenderHeroESP( render_cached , view_matrix , screen_w , screen_h );
                    break;

                case EntityClassType::NPC_TROOPER:
                case EntityClassType::NPC_TROOPER_NEUTRAL:
                    RenderTrooperESP( render_cached , view_matrix , screen_w , screen_h );
                    break;

                case EntityClassType::PROJECTILE_HOOK:
                    RenderThreatAlert( render_cached , view_matrix , screen_w , screen_h );
                    break;

                default:
                    break;
            }
        }
    }

    auto Visuals::RenderHeroESP( const CachedEntity& entry , const VMatrix& view_matrix , float screen_w , float screen_h ) -> void
    {
        const auto& cfg = GetConfig();
        const auto origin = entry.entity_ptr->m_pGameSceneNode()->m_vecAbsOrigin();
        const auto head_origin = origin + Vector3{ 0.0f, 0.0f, 72.0f };
        Vector3 cam_pos{
            -( view_matrix.m[0][0] * view_matrix.m[0][3] + view_matrix.m[1][0] * view_matrix.m[1][3] + view_matrix.m[2][0] * view_matrix.m[2][3] ),
            -( view_matrix.m[0][1] * view_matrix.m[0][3] + view_matrix.m[1][1] * view_matrix.m[1][3] + view_matrix.m[2][1] * view_matrix.m[2][3] ),
            -( view_matrix.m[0][2] * view_matrix.m[0][3] + view_matrix.m[1][2] * view_matrix.m[1][3] + view_matrix.m[2][2] * view_matrix.m[2][3] )
        };
        const float dist_m = ( cam_pos.Length() > 1.0f ) ? ( origin.DistTo( cam_pos ) / 50.0f ) : ( origin.Length() / 50.0f );

        Vector2 screen_bottom , screen_top;
        if ( !Core::Math::WorldToScreen( origin , screen_bottom , view_matrix , screen_w , screen_h ) ||
             !Core::Math::WorldToScreen( head_origin , screen_top , view_matrix , screen_w , screen_h ) )
        {
            // Enemy is off-screen: render off-screen indicator if enabled
            if ( cfg.esp_offscreen_radar )
            {
                RenderOffScreenIndicator( origin , view_matrix , screen_w , screen_h , dist_m );
            }
            return;
        }

        const float box_height = std::abs( screen_bottom.y - screen_top.y );
        const float box_width = box_height * 0.45f;
        const float box_x = screen_top.x - ( box_width * 0.5f );
        const float box_y = screen_top.y;

        // Snaplines (Tracer Lines)
        if ( cfg.esp_snaplines )
        {
            UI::InkwellRenderer::DrawLine( screen_w * 0.5f , screen_h , screen_bottom.x , screen_bottom.y , cfg.color_border );
        }

        // Bounding Box
        if ( cfg.esp_box )
        {
            UI::InkwellRenderer::DrawBox( box_x , box_y , box_width , box_height , cfg.color_enemy_vis );
        }

        // Dynamic Health Bar
        if ( cfg.esp_health_bar )
        {
            const int health = entry.entity_ptr->m_iHealth();
            const int max_health = entry.entity_ptr->m_iMaxHealth() > 0 ? entry.entity_ptr->m_iMaxHealth() : 1000;
            const float hp_frac = std::clamp( static_cast<float>( health ) / static_cast<float>( max_health ) , 0.0f , 1.0f );

            const float bar_w = 4.0f;
            const float bar_x = box_x - bar_w - 3.0f;
            const float bar_h = box_height;

            // Background
            UI::InkwellRenderer::DrawBox( bar_x , box_y , bar_w , bar_h , cfg.color_background );

            // Dynamic color grading based on health
            ColorRGB hp_color = cfg.color_enemy_vis;
            if ( hp_frac <= 0.35f )
                hp_color = cfg.color_enemy_invis;
            else if ( hp_frac <= 0.65f )
                hp_color = cfg.color_neutral;

            const float fill_h = bar_h * hp_frac;
            UI::InkwellRenderer::DrawBox( bar_x , box_y + ( bar_h - fill_h ) , bar_w , fill_h , hp_color );
        }

        // Hero Name and Distance
        if ( cfg.esp_name )
        {
            const char* hero_title = ResolveHeroName( entry.entity_ptr );
            char label[96];
            snprintf( label , sizeof( label ) , "%s [%d HP | %.0fm]" , hero_title , entry.entity_ptr->m_iHealth() , dist_m );
            UI::InkwellRenderer::DrawText( box_x , box_y - 14.0f , label , cfg.color_accent );
        }

        // Skeleton ESP
        if ( cfg.esp_skeleton )
        {
            const auto& bones = Source2::BoneManager::GetHeroSkeletonBones();
            for ( const auto& [b1, b2] : bones )
            {
                const auto pos1 = Source2::BoneManager::GetBonePositionByName( entry.entity_ptr , b1 );
                const auto pos2 = Source2::BoneManager::GetBonePositionByName( entry.entity_ptr , b2 );

                Vector2 pt_start{};
                Vector2 pt_end{};
                if ( !pos1.IsZero() && !pos2.IsZero() &&
                     Core::Math::WorldToScreen( pos1 , pt_start , view_matrix , screen_w , screen_h ) &&
                     Core::Math::WorldToScreen( pos2 , pt_end , view_matrix , screen_w , screen_h ) )
                {
                    UI::InkwellRenderer::DrawLine( pt_start.x , pt_start.y , pt_end.x , pt_end.y , cfg.color_primary );
                }
            }
        }
    }

    auto Visuals::RenderTrooperESP( const CachedEntity& entry , const VMatrix& view_matrix , float screen_w , float screen_h ) -> void
    {
        const auto& cfg = GetConfig();
        if ( !cfg.esp_trooper )
            return;

        const auto origin = entry.entity_ptr->m_pGameSceneNode()->m_vecAbsOrigin();
        Vector2 screen_pos;
        if ( !Core::Math::WorldToScreen( origin , screen_pos , view_matrix , screen_w , screen_h ) )
            return;

        UI::InkwellRenderer::DrawBox( screen_pos.x - 10.0f , screen_pos.y - 20.0f , 20.0f , 20.0f , cfg.color_neutral );
    }

    auto Visuals::RenderThreatAlert( const CachedEntity& entry , const VMatrix& view_matrix , float screen_w , float screen_h ) -> void
    {
        const auto& cfg = GetConfig();
        if ( !cfg.threat_warning_enabled || !entry.entity_ptr || !entry.entity_ptr->m_pGameSceneNode() )
            return;

        const auto hook_pos = entry.entity_ptr->m_pGameSceneNode()->m_vecAbsOrigin();
        Vector3 cam_pos{
            -( view_matrix.m[0][0] * view_matrix.m[0][3] + view_matrix.m[1][0] * view_matrix.m[1][3] + view_matrix.m[2][0] * view_matrix.m[2][3] ),
            -( view_matrix.m[0][1] * view_matrix.m[0][3] + view_matrix.m[1][1] * view_matrix.m[1][3] + view_matrix.m[2][1] * view_matrix.m[2][3] ),
            -( view_matrix.m[0][2] * view_matrix.m[0][3] + view_matrix.m[1][2] * view_matrix.m[1][3] + view_matrix.m[2][2] * view_matrix.m[2][3] )
        };
        const float dist_m = ( cam_pos.Length() > 1.0f ) ? ( hook_pos.DistTo( cam_pos ) / 50.0f ) : ( hook_pos.Length() / 50.0f );

        Vector2 screen_pos;
        if ( Core::Math::WorldToScreen( hook_pos , screen_pos , view_matrix , screen_w , screen_h ) )
        {
            // Draw hook trajectory marker in 3D screen space
            UI::InkwellRenderer::DrawBox( screen_pos.x - 14.0f , screen_pos.y - 14.0f , 28.0f , 28.0f , cfg.color_enemy_invis );
            char hook_lbl[48];
            snprintf( hook_lbl , sizeof( hook_lbl ) , "[HOOK: %.0fm]" , dist_m );
            UI::InkwellRenderer::DrawText( screen_pos.x + 18.0f , screen_pos.y - 6.0f , hook_lbl , cfg.color_enemy_invis );
        }

        // Prominent HUD Alert Banner (top center)
        const float banner_w = 340.0f;
        const float banner_h = 36.0f;
        const float banner_x = ( screen_w * 0.5f ) - ( banner_w * 0.5f );
        const float banner_y = 75.0f;

        UI::InkwellRenderer::DrawBox( banner_x , banner_y , banner_w , banner_h , cfg.color_background );
        UI::InkwellRenderer::DrawBox( banner_x , banner_y , banner_w , 2.0f , cfg.color_enemy_invis ); // Top warning accent line
        char banner_txt[64];
        snprintf( banner_txt , sizeof( banner_txt ) , "! BEBOP HOOK DETECTED [%.0fm] !" , dist_m );
        UI::InkwellRenderer::DrawText( banner_x + 20.0f , banner_y + 9.0f , banner_txt , cfg.color_enemy_invis );
    }
}
