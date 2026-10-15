#include "hooks/SteamOverlayHook.hpp"
#include "core/MemoryEngine.hpp"
#include "core/HookEngine.hpp"
#include "core/Logger.hpp"
#include "ui/InkwellUI.hpp"
#include "features/Visuals.hpp"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam );

namespace Inkwell::Hooks
{
    auto SteamOverlayHook::InitializeDirectXFallback() -> bool
    {
        Logger::Log( "RENDER" , "Attempting direct DirectX 11 dummy SwapChain resolution (VenDeadlock architecture)..." );

        WNDCLASSEXA wc{ sizeof( WNDCLASSEXA ) , CS_CLASSDC , DefWindowProcA , 0 , 0 , GetModuleHandleA( nullptr ) , nullptr , nullptr , nullptr , nullptr , "InkwellDummyClass" , nullptr };
        RegisterClassExA( &wc );

        HWND dummy_hwnd = CreateWindowExA( 0 , wc.lpszClassName , "" , WS_OVERLAPPEDWINDOW , 0 , 0 , 100 , 100 , nullptr , nullptr , wc.hInstance , nullptr );
        if ( !dummy_hwnd )
        {
            UnregisterClassA( wc.lpszClassName , wc.hInstance );
            return false;
        }

        D3D_FEATURE_LEVEL feature_level;
        const D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 , D3D_FEATURE_LEVEL_10_0 };

        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = dummy_hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        IDXGISwapChain* swap_chain = nullptr;
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr , D3D_DRIVER_TYPE_HARDWARE , nullptr , 0 ,
            feature_levels , 2 , D3D11_SDK_VERSION ,
            &sd , &swap_chain , &device , &feature_level , &context );

        if ( FAILED( hr ) || !swap_chain )
        {
            DestroyWindow( dummy_hwnd );
            UnregisterClassA( wc.lpszClassName , wc.hInstance );
            Logger::Error( "Failed to create dummy D3D11 device and swap chain." );
            return false;
        }

        void** vtbl = *reinterpret_cast<void***>( swap_chain );
        void* present_addr = vtbl[8];
        void* resize_addr = vtbl[13];

        swap_chain->Release();
        device->Release();
        context->Release();
        DestroyWindow( dummy_hwnd );
        UnregisterClassA( wc.lpszClassName , wc.hInstance );

        auto& hook_mgr = Core::HookEngine::Get();

        if ( !hook_mgr.InstallDetour( present_addr , reinterpret_cast<void*>( &SteamOverlayHook::Hooked_Present ) , reinterpret_cast<void**>( &s_original_present ) ) )
        {
            Logger::Error( "Failed to detour DXGI Present." );
            return false;
        }

        if ( !hook_mgr.InstallDetour( resize_addr , reinterpret_cast<void*>( &SteamOverlayHook::Hooked_ResizeBuffers ) , reinterpret_cast<void**>( &s_original_resize_buffers ) ) )
        {
            Logger::Error( "Failed to detour DXGI ResizeBuffers." );
            return false;
        }

        Logger::Success( "DirectX 11 (DXGI) SwapChain Present & ResizeBuffers detoured natively." );
        return true;
    }

    auto SteamOverlayHook::Initialize() -> bool
    {
        const auto overlay_module = GetModuleHandleA( "gameoverlayrenderer64.dll" );
        if ( overlay_module )
        {
            auto* present_addr = Core::MemoryEngine::FindPattern(
                "gameoverlayrenderer64.dll" ,
                "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B ? 8B ? 4C 8B" );

            if ( !present_addr )
            {
                present_addr = Core::MemoryEngine::FindPattern(
                    "gameoverlayrenderer64.dll" ,
                    "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B E8" );
            }

            auto* resize_addr = Core::MemoryEngine::FindPattern(
                "gameoverlayrenderer64.dll" ,
                "40 53 55 56 57 41 54 41 56 41 57 48 83 EC ? 44 8B E2" );

            if ( present_addr && resize_addr )
            {
                auto& hook_mgr = Core::HookEngine::Get();

                if ( hook_mgr.InstallDetour( present_addr , reinterpret_cast<void*>( &Hooked_Present ) , reinterpret_cast<void**>( &s_original_present ) ) &&
                     hook_mgr.InstallDetour( resize_addr , reinterpret_cast<void*>( &Hooked_ResizeBuffers ) , reinterpret_cast<void**>( &s_original_resize_buffers ) ) )
                {
                    Logger::Success( "Steam Overlay Present and ResizeBuffers detoured successfully." );
                    return true;
                }
            }
            Logger::Warn( "Steam overlay pattern match or hook installation failed; falling back to DXGI." );
        }
        else
        {
            Logger::Warn( "gameoverlayrenderer64.dll not found in memory; engaging direct DXGI hook." );
        }

        return InitializeDirectXFallback();
    }

    auto SteamOverlayHook::Shutdown() -> void
    {
        if ( s_game_hwnd && s_original_wndproc )
        {
            if ( IsWindowUnicode( s_game_hwnd ) )
                SetWindowLongPtrW( s_game_hwnd , GWLP_WNDPROC , reinterpret_cast<LONG_PTR>( s_original_wndproc ) );
            else
                SetWindowLongPtrA( s_game_hwnd , GWLP_WNDPROC , reinterpret_cast<LONG_PTR>( s_original_wndproc ) );
            s_original_wndproc = nullptr;
        }

        if ( s_is_d3d_initialized )
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        UI::InkwellRenderer::Shutdown();

        if ( s_render_target_view )
        {
            s_render_target_view->Release();
            s_render_target_view = nullptr;
        }

        if ( s_d3d_context )
        {
            s_d3d_context->Release();
            s_d3d_context = nullptr;
        }

        if ( s_d3d_device )
        {
            s_d3d_device->Release();
            s_d3d_device = nullptr;
        }

        s_is_d3d_initialized = false;
    }

    auto CALLBACK SteamOverlayHook::Hooked_WndProc( HWND hwnd , UINT msg , WPARAM wparam , LPARAM lparam ) -> LRESULT
    {
        if ( msg == WM_KEYDOWN && wparam == VK_INSERT && !( lparam & 0x40000000 ) )
        {
            auto& cfg = GetConfig();
            cfg.menu_open = !cfg.menu_open;
            return 0;
        }

        if ( GetConfig().menu_open )
        {
            if ( ImGui_ImplWin32_WndProcHandler( hwnd , msg , wparam , lparam ) )
                return true;
        }

        if ( s_original_wndproc )
        {
            return IsWindowUnicode( hwnd ) ?
                CallWindowProcW( s_original_wndproc , hwnd , msg , wparam , lparam ) :
                CallWindowProcA( s_original_wndproc , hwnd , msg , wparam , lparam );
        }

        return IsWindowUnicode( hwnd ) ?
            DefWindowProcW( hwnd , msg , wparam , lparam ) :
            DefWindowProcA( hwnd , msg , wparam , lparam );
    }

    auto WINAPI SteamOverlayHook::Hooked_Present( IDXGISwapChain* swap_chain , UINT sync_interval , UINT flags ) -> HRESULT
    {
        if ( !s_is_d3d_initialized && swap_chain )
        {
            if ( SUCCEEDED( swap_chain->GetDevice( __uuidof( ID3D11Device ) , reinterpret_cast<void**>( &s_d3d_device ) ) ) )
            {
                s_d3d_device->GetImmediateContext( &s_d3d_context );

                ID3D11Texture2D* back_buffer = nullptr;
                if ( SUCCEEDED( swap_chain->GetBuffer( 0 , __uuidof( ID3D11Texture2D ) , reinterpret_cast<void**>( &back_buffer ) ) ) )
                {
                    D3D11_TEXTURE2D_DESC tex_desc{};
                    back_buffer->GetDesc( &tex_desc );

                    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{};
                    rtv_desc.Format = tex_desc.Format;
                    rtv_desc.ViewDimension = ( tex_desc.SampleDesc.Count > 1 ) ?
                        D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

                    s_d3d_device->CreateRenderTargetView( back_buffer , &rtv_desc , &s_render_target_view );
                    back_buffer->Release();
                }

                DXGI_SWAP_CHAIN_DESC desc{};
                HWND game_hwnd = nullptr;
                if ( SUCCEEDED( swap_chain->GetDesc( &desc ) ) )
                {
                    game_hwnd = desc.OutputWindow;
                }
                if ( !game_hwnd )
                {
                    EnumWindows( []( HWND hwnd , LPARAM lparam ) -> BOOL {
                        DWORD pid = 0;
                        GetWindowThreadProcessId( hwnd , &pid );
                        if ( pid == GetCurrentProcessId() && IsWindowVisible( hwnd ) )
                        {
                            *reinterpret_cast<HWND*>( lparam ) = hwnd;
                            return FALSE;
                        }
                        return TRUE;
                    } , reinterpret_cast<LPARAM>( &game_hwnd ) );
                }
                if ( !game_hwnd )
                {
                    game_hwnd = FindWindowA( "SDL_app" , "Deadlock" );
                }

                s_game_hwnd = game_hwnd;
                if ( s_game_hwnd )
                {
                    if ( IsWindowUnicode( s_game_hwnd ) )
                    {
                        s_original_wndproc = reinterpret_cast<WNDPROC>(
                            SetWindowLongPtrW( s_game_hwnd , GWLP_WNDPROC , reinterpret_cast<LONG_PTR>( &Hooked_WndProc ) ) );
                    }
                    else
                    {
                        s_original_wndproc = reinterpret_cast<WNDPROC>(
                            SetWindowLongPtrA( s_game_hwnd , GWLP_WNDPROC , reinterpret_cast<LONG_PTR>( &Hooked_WndProc ) ) );
                    }
                }

                ImGui::CreateContext();
                ImGui_ImplWin32_Init( game_hwnd );
                ImGui_ImplDX11_Init( s_d3d_device , s_d3d_context );

                UI::InkwellRenderer::Initialize( s_d3d_device , s_d3d_context );
                s_is_d3d_initialized = true;
            }
        }

        if ( s_is_d3d_initialized && swap_chain )
        {
            s_d3d_context->OMSetRenderTargets( 1 , &s_render_target_view , nullptr );

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            UI::InkwellRenderer::BeginFrame( swap_chain );

            const auto& cfg = GetConfig();

            if ( cfg.watermark_enabled )
            {
                UI::InkwellRenderer::RenderWatermark();
            }

            DXGI_SWAP_CHAIN_DESC desc{};
            float screen_w = 0.0f;
            float screen_h = 0.0f;
            if ( SUCCEEDED( swap_chain->GetDesc( &desc ) ) )
            {
                screen_w = static_cast<float>( desc.BufferDesc.Width );
                screen_h = static_cast<float>( desc.BufferDesc.Height );
            }

            if ( screen_w <= 0.0f || screen_h <= 0.0f )
            {
                RECT client_rect{};
                if ( s_game_hwnd && GetClientRect( s_game_hwnd , &client_rect ) )
                {
                    screen_w = static_cast<float>( client_rect.right - client_rect.left );
                    screen_h = static_cast<float>( client_rect.bottom - client_rect.top );
                }
            }

            if ( screen_w <= 0.0f || screen_h <= 0.0f )
            {
                screen_w = ImGui::GetIO().DisplaySize.x > 0.0f ? ImGui::GetIO().DisplaySize.x : 1920.0f;
                screen_h = ImGui::GetIO().DisplaySize.y > 0.0f ? ImGui::GetIO().DisplaySize.y : 1080.0f;
            }

            if ( cfg.fov_circle )
            {
                UI::InkwellRenderer::RenderFOVCircle( screen_w , screen_h , cfg.fov_radius );
            }

            // Dynamically resolve live ViewMatrix pointer from client.dll (dwViewMatrix)
            static const VMatrix* s_pLiveViewMatrix = nullptr;
            static bool s_view_matrix_searched = false;
            if ( !s_view_matrix_searched )
            {
                // Method 1: Deadlock unique dwViewMatrix signature (RVA 0x1926353 -> 0x3CE9160)
                auto mat_ref = reinterpret_cast<uintptr_t>(
                    Core::MemoryEngine::FindPattern( "client.dll" , "48 8D 0D ? ? ? ? 48 C1 E0 06 48 03 C1 C3" ) );
                if ( !mat_ref )
                {
                    mat_ref = reinterpret_cast<uintptr_t>(
                        Core::MemoryEngine::FindPattern( "client.dll" , "48 8D ? ? ? ? ? 48 C1 E0 06 48 03 C1 C3" ) );
                }

                if ( mat_ref )
                {
                    s_pLiveViewMatrix = Core::MemoryEngine::ResolveRelativeAddress<const VMatrix*>( mat_ref , 3 , 7 );
                }
                else
                {
                    // Fallback: Direct RVA from verified binary
                    auto client_base = reinterpret_cast<uintptr_t>( GetModuleHandleA( "client.dll" ) );
                    if ( client_base )
                    {
                        s_pLiveViewMatrix = reinterpret_cast<const VMatrix*>( client_base + 0x3CE9160 );
                    }
                }
                s_view_matrix_searched = true;
            }

            VMatrix view_matrix{};
            if ( s_pLiveViewMatrix )
            {
                uintptr_t ptr_val = reinterpret_cast<uintptr_t>( s_pLiveViewMatrix );
                if ( ptr_val >= 0x10000 && ptr_val < 0x7FFFFFFFFFFF )
                {
                    __try
                    {
                        view_matrix = *s_pLiveViewMatrix;
                    }
                    __except ( EXCEPTION_EXECUTE_HANDLER )
                    {
                    }
                }
            }

            Features::Visuals::Render( view_matrix , screen_w , screen_h );

            // Interactive ImGui Menu (toggled via INSERT)
            UI::InkwellRenderer::RenderMenu();

            UI::InkwellRenderer::EndFrame();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
        }

        return s_original_present ? s_original_present( swap_chain , sync_interval , flags ) : S_OK;
    }

    auto WINAPI SteamOverlayHook::Hooked_ResizeBuffers( IDXGISwapChain* swap_chain , UINT buffer_count , UINT width , UINT height , DXGI_FORMAT new_format , UINT swap_chain_flags ) -> HRESULT
    {
        if ( s_render_target_view )
        {
            s_render_target_view->Release();
            s_render_target_view = nullptr;
        }

        s_is_d3d_initialized = false;

        return s_original_resize_buffers ?
            s_original_resize_buffers( swap_chain , buffer_count , width , height , new_format , swap_chain_flags ) : S_OK;
    }
}
