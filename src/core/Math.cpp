#include "core/Math.hpp"
#include "core/MemoryEngine.hpp"

namespace Inkwell::Core::Math
{
    auto CalcAngle( const Vector3& src , const Vector3& dst ) -> QAngle
    {
        const Vector3 delta = dst - src;
        const float hyp = delta.Length2D();

        QAngle angles;
        angles.pitch = -std::atan2( delta.z , hyp ) * INK_RAD2DEG;
        angles.yaw = std::atan2( delta.y , delta.x ) * INK_RAD2DEG;
        angles.roll = 0.0f;

        angles.Normalize();
        angles.Clamp();
        return angles;
    }

    auto AngleVectors( const QAngle& angles , Vector3& forward ) -> void
    {
        const float sp = std::sin( angles.pitch * INK_DEG2RAD );
        const float cp = std::cos( angles.pitch * INK_DEG2RAD );
        const float sy = std::sin( angles.yaw * INK_DEG2RAD );
        const float cy = std::cos( angles.yaw * INK_DEG2RAD );

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
    }

    auto SmoothAngle( const QAngle& current , const QAngle& target , float smoothing ) -> QAngle
    {
        if ( smoothing <= 1.0f )
            return target;

        QAngle delta = target - current;
        delta.Normalize();

        QAngle out = current + ( delta / smoothing );
        out.Normalize();
        out.Clamp();
        return out;
    }

    auto CalcFov( const QAngle& view_angles , const QAngle& aim_angles ) -> float
    {
        QAngle delta = aim_angles - view_angles;
        delta.Normalize();
        return std::sqrt( delta.pitch * delta.pitch + delta.yaw * delta.yaw );
    }

    using ScreenTransformFn = bool ( __fastcall* )( const Vector3& , Vector3& );
    static ScreenTransformFn s_screen_transform = nullptr;

    auto WorldToScreen( const Vector3& world , Vector2& screen , const VMatrix& view_matrix , float screen_w , float screen_h ) -> bool
    {
        // Primary: Direct ViewMatrix 3D->2D projection (hardware accelerated, zero detour overhead)
        const float w = view_matrix.m[3][0] * world.x + view_matrix.m[3][1] * world.y + view_matrix.m[3][2] * world.z + view_matrix.m[3][3];

        if ( w > 0.001f )
        {
            const float inv_w = 1.0f / w;
            const float x = ( view_matrix.m[0][0] * world.x + view_matrix.m[0][1] * world.y + view_matrix.m[0][2] * world.z + view_matrix.m[0][3] ) * inv_w;
            const float y = ( view_matrix.m[1][0] * world.x + view_matrix.m[1][1] * world.y + view_matrix.m[1][2] * world.z + view_matrix.m[1][3] ) * inv_w;

            screen.x = ( screen_w * 0.5f ) + ( 0.5f * x * screen_w );
            screen.y = ( screen_h * 0.5f ) - ( 0.5f * y * screen_h );
            return true;
        }

        // Secondary Fallback: Game engine ScreenTransform function
        static bool s_screen_transform_searched = false;
        if ( !s_screen_transform && !s_screen_transform_searched )
        {
            auto fn = MemoryEngine::FindPattern( "client.dll" , "33 C0 48 39 05 ? ? ? ? 0F 84" );
            if ( fn )
            {
                s_screen_transform = reinterpret_cast<ScreenTransformFn>( fn );
            }
            s_screen_transform_searched = true;
        }

        if ( s_screen_transform )
        {
            Vector3 clip_pos{};
            __try
            {
                if ( !s_screen_transform( world , clip_pos ) )
                {
                    screen.x = ( ( clip_pos.x + 1.0f ) * 0.5f ) * screen_w;
                    screen.y = screen_h - ( ( ( clip_pos.y + 1.0f ) * 0.5f ) * screen_h );
                    return true;
                }
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
            }
        }

        return false;
    }
}
