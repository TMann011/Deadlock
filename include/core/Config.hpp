#pragma once

#include <cstdint>
#include <string_view>

namespace Inkwell
{
    // Authentic Inkwell Core Brand Metadata
    constexpr std::string_view PROJECT_NAME = "INKWELL";
    constexpr std::string_view PROJECT_SUITE = "Citadel Instrumentation Core [Source 2]";
    constexpr std::string_view PROJECT_VERSION = "v3.2 PROD";
    constexpr std::string_view BUILD_TAG = "Release_v145";
    constexpr std::string_view LICENSE_TAG = "INK-CORE-7749";

    struct ColorRGB
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        constexpr uint32_t ToRGBA() const
        {
            const auto uR = static_cast<uint32_t>( r * 255.0f ) & 0xFF;
            const auto uG = static_cast<uint32_t>( g * 255.0f ) & 0xFF;
            const auto uB = static_cast<uint32_t>( b * 255.0f ) & 0xFF;
            const auto uA = static_cast<uint32_t>( a * 255.0f ) & 0xFF;
            return ( uA << 24 ) | ( uB << 16 ) | ( uG << 8 ) | uR;
        }
    };

    struct Settings
    {
        // Visuals
        bool esp_enabled = true;
        bool esp_box = true;
        bool esp_name = true;
        bool esp_health = true;
        bool esp_skeleton = true;
        bool esp_trooper = true;
        bool esp_neutral = true;
        bool esp_orbs = true;
        bool fov_circle = true;
        float fov_radius = 90.0f;

        // GUI & Interactive Menu
        bool menu_open = false;
        bool watermark_enabled = true;

        // Minimap / Radar
        bool unlock_minimap_fow = true;

        // Movement / Aim Assist & Combat
        bool aim_assist_enabled = true;
        float aim_smoothing = 4.5f;
        float aim_max_fov = 15.0f;
        bool silent_hook_bebop = false;
        bool recoil_reduction_enabled = true;
        float recoil_compensation_pitch = 0.70f;
        float recoil_compensation_yaw = 0.70f;
        bool triggerbot_enabled = true;
        float triggerbot_fov = 2.2f;

        // Visuals Enhancements (Health Bar, HP Numbers, Threat Distance)
        bool esp_health_bar = true;
        bool esp_distance = true;
        bool esp_snaplines = true;
        bool esp_offscreen_radar = true;
        float offscreen_radius = 240.0f;

        // Threat / In-Flight Warning (VenDeadlock Parity)
        bool threat_warning_enabled = true;
        float threat_warning_radius = 2500.0f;

        // Advanced Combat (CQB Auto-Melee)
        bool auto_melee_enabled = true;
        float auto_melee_distance = 250.0f; // in-game units (~3.5m)

        // Official Inkwell Purple & Black Branding Palette
        ColorRGB color_accent = { 0.58f, 0.28f, 0.95f, 1.00f };      // Neon Violet (#9447F2)
        ColorRGB color_accent_active = { 0.75f, 0.45f, 1.00f, 1.00f };// Bright Purple
        ColorRGB color_primary = { 0.38f, 0.45f, 0.98f, 1.00f };     // Electric Indigo
        ColorRGB color_background = { 0.04f, 0.03f, 0.06f, 0.96f };  // Pitch Black Glass
        ColorRGB color_panel_bg = { 0.08f, 0.06f, 0.12f, 0.85f };    // Dark Violet Card
        ColorRGB color_border = { 0.24f, 0.15f, 0.38f, 0.55f };      // Muted Violet Border
        ColorRGB color_text_primary = { 0.96f, 0.95f, 0.98f, 1.00f }; // Pure White
        ColorRGB color_text_secondary = { 0.65f, 0.58f, 0.75f, 1.00f }; // Lavender Gray Muted
        ColorRGB color_enemy_vis = { 0.20f, 0.85f, 0.55f, 1.00f };   // Emerald Visible
        ColorRGB color_enemy_invis = { 0.95f, 0.30f, 0.30f, 1.00f }; // Crimson Error
        ColorRGB color_neutral = { 0.95f, 0.75f, 0.20f, 1.00f };     // Amber Gold
    };

    auto GetConfig() -> Settings&;
}
