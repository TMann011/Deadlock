#pragma once

#include <windows.h>
#include <d3d11.h>
#include "core/Config.hpp"
#include "core/Math.hpp"

namespace Inkwell::UI
{
    class InkwellRenderer
    {
    public:
        static auto Initialize( ID3D11Device* device , ID3D11DeviceContext* context ) -> bool;
        static auto Shutdown() -> void;

        static auto BeginFrame( IDXGISwapChain* swap_chain ) -> void;
        static auto RenderWatermark() -> void;
        static auto RenderFOVCircle( float screen_w , float screen_h , float fov_radius ) -> void;
        static auto RenderMenu() -> void;
        static auto EndFrame() -> void;

        static auto DrawBox( float x , float y , float w , float h , ColorRGB color ) -> void;
        static auto DrawLine( float x1 , float y1 , float x2 , float y2 , ColorRGB color ) -> void;
        static auto DrawText( float x , float y , const char* text , ColorRGB color ) -> void;

    private:
        static inline bool s_initialized = false;
        static inline float s_screen_width = 1920.0f;
        static inline float s_screen_height = 1080.0f;
    };
}
