#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace Inkwell::Hooks
{
    class SteamOverlayHook
    {
    public:
        static auto Initialize() -> bool;
        static auto Shutdown() -> void;

        static auto WINAPI Hooked_Present( IDXGISwapChain* swap_chain , UINT sync_interval , UINT flags ) -> HRESULT;
        static auto WINAPI Hooked_ResizeBuffers( IDXGISwapChain* swap_chain , UINT buffer_count , UINT width , UINT height , DXGI_FORMAT new_format , UINT swap_chain_flags ) -> HRESULT;
        static auto CALLBACK Hooked_WndProc( HWND hwnd , UINT msg , WPARAM wparam , LPARAM lparam ) -> LRESULT;

        static auto GetDevice() -> ID3D11Device* { return s_d3d_device; }
        static auto GetDeviceContext() -> ID3D11DeviceContext* { return s_d3d_context; }
        static auto GetRenderTarget() -> ID3D11RenderTargetView* { return s_render_target_view; }

    private:
        static auto InitializeDirectXFallback() -> bool;

        using PresentFn = HRESULT( WINAPI* )( IDXGISwapChain* , UINT , UINT );
        using ResizeBuffersFn = HRESULT( WINAPI* )( IDXGISwapChain* , UINT , UINT , UINT , DXGI_FORMAT , UINT );

        static inline PresentFn s_original_present = nullptr;
        static inline ResizeBuffersFn s_original_resize_buffers = nullptr;
        static inline WNDPROC s_original_wndproc = nullptr;
        static inline HWND s_game_hwnd = nullptr;

        static inline bool s_is_d3d_initialized = false;
        static inline ID3D11Device* s_d3d_device = nullptr;
        static inline ID3D11DeviceContext* s_d3d_context = nullptr;
        static inline ID3D11RenderTargetView* s_render_target_view = nullptr;
    };
}
