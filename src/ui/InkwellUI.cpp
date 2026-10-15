#include "ui/InkwellUI.hpp"
#include "core/Logger.hpp"
#include <imgui.h>

namespace Inkwell::UI
{
    auto InkwellRenderer::Initialize( ID3D11Device* device , ID3D11DeviceContext* context ) -> bool
    {
        if ( !device || !context )
            return false;

        s_initialized = true;
        Logger::Success( "Inkwell ImGui Canvas Renderer initialized." );
        return true;
    }

    auto InkwellRenderer::Shutdown() -> void
    {
        s_initialized = false;
    }

    auto InkwellRenderer::BeginFrame( IDXGISwapChain* swap_chain ) -> void
    {
        if ( !swap_chain ) return;

        DXGI_SWAP_CHAIN_DESC desc{};
        if ( SUCCEEDED( swap_chain->GetDesc( &desc ) ) )
        {
            s_screen_width = static_cast<float>( desc.BufferDesc.Width );
            s_screen_height = static_cast<float>( desc.BufferDesc.Height );
        }
    }

    auto InkwellRenderer::RenderWatermark() -> void
    {
        // Official Inkwell Purple & Black Glass Watermark Card
        const auto& cfg = GetConfig();
        const float card_x = 18.0f;
        const float card_y = 18.0f;
        const float card_w = 340.0f;
        const float card_h = 38.0f;

        // Pitch Black Glass Background
        DrawBox( card_x , card_y , card_w , card_h , cfg.color_background );

        // Neon Violet (#9447F2) Top Accent Bar
        DrawLine( card_x , card_y , card_x + card_w , card_y , cfg.color_accent );
        DrawLine( card_x , card_y + 1.0f , card_x + card_w , card_y + 1.0f , cfg.color_accent_active );

        // Watermark Label
        DrawText( card_x + 14.0f , card_y + 12.0f , "INKWELL  |  Citadel Core [v3.2 PROD]" , cfg.color_accent );
    }

    auto InkwellRenderer::RenderFOVCircle( float screen_w , float screen_h , float fov_radius ) -> void
    {
        const auto center_x = screen_w * 0.5f;
        const auto center_y = screen_h * 0.5f;
        const auto& cfg = GetConfig();

        auto* draw_list = ImGui::GetBackgroundDrawList();
        if ( !draw_list ) return;

        ImU32 col = IM_COL32( static_cast<int>( cfg.color_primary.r * 255.0f ) ,
                              static_cast<int>( cfg.color_primary.g * 255.0f ) ,
                              static_cast<int>( cfg.color_primary.b * 255.0f ) ,
                              static_cast<int>( cfg.color_primary.a * 255.0f ) );

        draw_list->AddCircle( ImVec2( center_x , center_y ) , fov_radius , col , 64 , 1.5f );
    }

    auto InkwellRenderer::RenderMenu() -> void
    {
        auto& cfg = GetConfig();
        if ( !cfg.menu_open )
            return;

        ImGui::SetNextWindowSize( ImVec2( 580.0f , 460.0f ) , ImGuiCond_FirstUseEver );

        // Authentic Inkwell Purple & Black Theme styling
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 8.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 5.0f;
        style.ScrollbarRounding = 5.0f;
        style.GrabRounding = 5.0f;
        style.TabRounding = 5.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4( 0.05f , 0.04f , 0.08f , 0.96f );
        colors[ImGuiCol_Border] = ImVec4( 0.58f , 0.28f , 0.95f , 0.65f );
        colors[ImGuiCol_TitleBg] = ImVec4( 0.12f , 0.08f , 0.20f , 1.00f );
        colors[ImGuiCol_TitleBgActive] = ImVec4( 0.22f , 0.12f , 0.38f , 1.00f );
        colors[ImGuiCol_Tab] = ImVec4( 0.15f , 0.10f , 0.24f , 0.90f );
        colors[ImGuiCol_TabHovered] = ImVec4( 0.45f , 0.25f , 0.75f , 0.90f );
        colors[ImGuiCol_TabActive] = ImVec4( 0.58f , 0.28f , 0.95f , 1.00f );
        colors[ImGuiCol_FrameBg] = ImVec4( 0.12f , 0.09f , 0.18f , 1.00f );
        colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.25f , 0.16f , 0.38f , 1.00f );
        colors[ImGuiCol_FrameBgActive] = ImVec4( 0.40f , 0.20f , 0.65f , 1.00f );
        colors[ImGuiCol_CheckMark] = ImVec4( 0.75f , 0.45f , 1.00f , 1.00f );
        colors[ImGuiCol_SliderGrab] = ImVec4( 0.58f , 0.28f , 0.95f , 1.00f );
        colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.78f , 0.48f , 1.00f , 1.00f );
        colors[ImGuiCol_Button] = ImVec4( 0.30f , 0.15f , 0.50f , 0.85f );
        colors[ImGuiCol_ButtonHovered] = ImVec4( 0.48f , 0.24f , 0.80f , 1.00f );
        colors[ImGuiCol_ButtonActive] = ImVec4( 0.58f , 0.28f , 0.95f , 1.00f );
        colors[ImGuiCol_Header] = ImVec4( 0.35f , 0.18f , 0.58f , 0.80f );
        colors[ImGuiCol_HeaderHovered] = ImVec4( 0.50f , 0.26f , 0.82f , 0.90f );
        colors[ImGuiCol_HeaderActive] = ImVec4( 0.58f , 0.28f , 0.95f , 1.00f );

        if ( ImGui::Begin( "INKWELL CORE  ::  Citadel Instrumentation Engine" , &cfg.menu_open ) )
        {
            if ( ImGui::BeginTabBar( "##InkwellTabs" ) )
            {
                if ( ImGui::BeginTabItem( "Visuals" ) )
                {
                    ImGui::Spacing();
                    ImGui::Checkbox( "Master ESP" , &cfg.esp_enabled );
                    ImGui::Separator();
                    ImGui::Checkbox( "2D Bounding Boxes" , &cfg.esp_box );
                    ImGui::Checkbox( "Dynamic Combat Health Bars" , &cfg.esp_health_bar );
                    ImGui::Checkbox( "Hero Skeleton Joints" , &cfg.esp_skeleton );
                    ImGui::Checkbox( "Hero Name & Class Tokens" , &cfg.esp_name );
                    ImGui::Checkbox( "Target Distance Readouts" , &cfg.esp_distance );
                    ImGui::Checkbox( "Snaplines (Tracer Lines)" , &cfg.esp_snaplines );
                    ImGui::Checkbox( "Trooper & Neutral Minion ESP" , &cfg.esp_trooper );
                    ImGui::Checkbox( "Off-Screen Perimeter Radar" , &cfg.esp_offscreen_radar );
                    if ( cfg.esp_offscreen_radar )
                    {
                        ImGui::SliderFloat( "Radar Radius" , &cfg.offscreen_radius , 100.0f , 500.0f , "%.0f px" );
                    }
                    ImGui::Checkbox( "In-Flight Threat / Bebop Hook Alert" , &cfg.threat_warning_enabled );
                    ImGui::EndTabItem();
                }

                if ( ImGui::BeginTabItem( "Combat & Aim" ) )
                {
                    ImGui::Spacing();
                    ImGui::Checkbox( "Aim Assist (Humanized Tracking)" , &cfg.aim_assist_enabled );
                    if ( cfg.aim_assist_enabled )
                    {
                        ImGui::SliderFloat( "Aim Smoothing" , &cfg.aim_smoothing , 1.0f , 15.0f , "%.1f" );
                        ImGui::SliderFloat( "Max Target FOV" , &cfg.aim_max_fov , 1.0f , 45.0f , "%.1f deg" );
                    }
                    ImGui::Separator();
                    ImGui::Checkbox( "Recoil Compensation (RCS)" , &cfg.recoil_reduction_enabled );
                    if ( cfg.recoil_reduction_enabled )
                    {
                        ImGui::SliderFloat( "Pitch Dampening" , &cfg.recoil_compensation_pitch , 0.1f , 1.0f , "%.2f" );
                    }
                    ImGui::Separator();
                    ImGui::Checkbox( "Triggerbot (Auto-Fire Assist)" , &cfg.triggerbot_enabled );
                    if ( cfg.triggerbot_enabled )
                    {
                        ImGui::SliderFloat( "Triggerbot Deadzone" , &cfg.triggerbot_fov , 0.5f , 5.0f , "%.1f deg" );
                    }
                    ImGui::Separator();
                    ImGui::Checkbox( "CQB Auto-Melee Strike" , &cfg.auto_melee_enabled );
                    if ( cfg.auto_melee_enabled )
                    {
                        ImGui::SliderFloat( "Strike Distance" , &cfg.auto_melee_distance , 100.0f , 400.0f , "%.0f units" );
                    }
                    ImGui::EndTabItem();
                }

                if ( ImGui::BeginTabItem( "Radar & HUD" ) )
                {
                    ImGui::Spacing();
                    ImGui::Checkbox( "Minimap Fog-of-War Full Disclosure" , &cfg.unlock_minimap_fow );
                    ImGui::Checkbox( "Watermark Glass Card" , &cfg.watermark_enabled );
                    ImGui::Checkbox( "FOV Circle Crosshair Indicator" , &cfg.fov_circle );
                    if ( cfg.fov_circle )
                    {
                        ImGui::SliderFloat( "FOV Circle Radius" , &cfg.fov_radius , 20.0f , 300.0f , "%.0f px" );
                    }
                    ImGui::EndTabItem();
                }

                if ( ImGui::BeginTabItem( "System" ) )
                {
                    ImGui::Spacing();
                    ImGui::TextColored( ImVec4( 0.75f , 0.45f , 1.00f , 1.00f ) , "Core Engine Status: Operational (Active)" );
                    ImGui::Text( "Architecture: x64 Native C++20 | Toolset: v145" );
                    ImGui::Text( "Target Process: project8.exe (Valve Deadlock)" );
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    ImGui::Text( "Hotkeys: [INSERT] Menu | [F9] Live Schema Dump | [END] Clean Panic Unload" );
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    auto InkwellRenderer::EndFrame() -> void
    {
    }

    auto InkwellRenderer::DrawBox( float x , float y , float w , float h , ColorRGB color ) -> void
    {
        auto* draw_list = ImGui::GetBackgroundDrawList();
        if ( !draw_list ) return;

        ImU32 col = IM_COL32( static_cast<int>( color.r * 255.0f ) ,
                              static_cast<int>( color.g * 255.0f ) ,
                              static_cast<int>( color.b * 255.0f ) ,
                              static_cast<int>( color.a * 255.0f ) );

        draw_list->AddRect( ImVec2( x , y ) , ImVec2( x + w , y + h ) , col );
    }

    auto InkwellRenderer::DrawLine( float x1 , float y1 , float x2 , float y2 , ColorRGB color ) -> void
    {
        auto* draw_list = ImGui::GetBackgroundDrawList();
        if ( !draw_list ) return;

        ImU32 col = IM_COL32( static_cast<int>( color.r * 255.0f ) ,
                              static_cast<int>( color.g * 255.0f ) ,
                              static_cast<int>( color.b * 255.0f ) ,
                              static_cast<int>( color.a * 255.0f ) );

        draw_list->AddLine( ImVec2( x1 , y1 ) , ImVec2( x2 , y2 ) , col , 1.0f );
    }

    auto InkwellRenderer::DrawText( float x , float y , const char* text , ColorRGB color ) -> void
    {
        auto* draw_list = ImGui::GetBackgroundDrawList();
        if ( !draw_list || !text ) return;

        ImU32 col = IM_COL32( static_cast<int>( color.r * 255.0f ) ,
                              static_cast<int>( color.g * 255.0f ) ,
                              static_cast<int>( color.b * 255.0f ) ,
                              static_cast<int>( color.a * 255.0f ) );

        draw_list->AddText( ImVec2( x , y ) , col , text );
    }
}
