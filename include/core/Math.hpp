#pragma once

#include <cmath>
#include <algorithm>

constexpr float INK_DEG2RAD = 0.01745329251f;
constexpr float INK_RAD2DEG = 57.295779513082f;

struct Vector2
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    inline auto IsZero() const -> bool { return x == 0.0f && y == 0.0f && z == 0.0f; }
    inline auto Length() const -> float { return std::sqrt( x * x + y * y + z * z ); }
    inline auto Length2D() const -> float { return std::sqrt( x * x + y * y ); }
    inline auto DistTo( const Vector3& o ) const -> float { return ( *this - o ).Length(); }

    inline auto operator+( const Vector3& o ) const -> Vector3 { return { x + o.x, y + o.y, z + o.z }; }
    inline auto operator-( const Vector3& o ) const -> Vector3 { return { x - o.x, y - o.y, z - o.z }; }
    inline auto operator*( float s ) const -> Vector3 { return { x * s, y * s, z * s }; }
    inline auto operator/( float s ) const -> Vector3 { return { x / s, y / s, z / s }; }
};

struct QAngle
{
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;

    inline auto Normalize() -> void
    {
        pitch = std::fmod( pitch , 360.0f );
        if ( pitch > 180.0f ) pitch -= 360.0f;
        else if ( pitch < -180.0f ) pitch += 360.0f;

        yaw = std::fmod( yaw , 360.0f );
        if ( yaw > 180.0f ) yaw -= 360.0f;
        else if ( yaw < -180.0f ) yaw += 360.0f;

        roll = 0.0f;
    }

    inline auto Clamp() -> void
    {
        pitch = std::clamp( pitch , -89.0f , 89.0f );
        yaw = std::clamp( yaw , -180.0f , 180.0f );
        roll = 0.0f;
    }

    inline auto operator+( const QAngle& o ) const -> QAngle { return { pitch + o.pitch, yaw + o.yaw, roll + o.roll }; }
    inline auto operator-( const QAngle& o ) const -> QAngle { return { pitch - o.pitch, yaw - o.yaw, roll - o.roll }; }
    inline auto operator*( float s ) const -> QAngle { return { pitch * s, yaw * s, roll * s }; }
    inline auto operator/( float s ) const -> QAngle { return { pitch / s, yaw / s, roll / s }; }
};

struct VMatrix
{
    float m[4][4] = { 0 };
};

namespace Inkwell::Core::Math
{
    auto CalcAngle( const Vector3& src , const Vector3& dst ) -> QAngle;
    auto AngleVectors( const QAngle& angles , Vector3& forward ) -> void;
    auto SmoothAngle( const QAngle& current , const QAngle& target , float smoothing ) -> QAngle;
    auto CalcFov( const QAngle& view_angles , const QAngle& aim_angles ) -> float;
    auto WorldToScreen( const Vector3& world , Vector2& screen , const VMatrix& view_matrix , float screen_w , float screen_h ) -> bool;
}
