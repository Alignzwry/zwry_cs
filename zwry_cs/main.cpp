#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "cs.h"
#include <chrono>
#include <d3dx11.h>
#include <dwmapi.h>
#include <thread>
#include "util/font.h"
cs2 _cs;

long frametime = 1000000000.f / 144.f;

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
int fps = 0, amount = 0;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Fps() {
    while (true) {
        Sleep(500);
        fps = amount * 2;
        amount = 0;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int iCmdShow)
{
#ifndef NDEBUG
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    HWND Handle = GetConsoleWindow();
    freopen("CON", "w", stdout);
#endif
    _cs.Init();
    RECT Rect{};
    POINT Point{};
    GetClientRect(_cs.counter_strike_hwnd, &Rect);
    ClientToScreen(_cs.counter_strike_hwnd, &Point);

    _cs.screen = Vec2(static_cast<float>(Rect.right), static_cast<float>(Rect.bottom));
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"zwry", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, wc.lpszClassName, L"zwry", WS_POPUP, Point.x, Point.y, _cs.screen.x, _cs.screen.y, nullptr, nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 0;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    std::thread t2(&Fps);
    t2.detach();
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 0.f);

    ImGui::CreateContext();
    io.Fonts->AddFontDefault();

    ImFontAtlas* fontAtlas = new ImFontAtlas();
    ImFontConfig arialConfig;
    arialConfig.FontDataOwnedByAtlas = false;
    ImFont* arialFont = fontAtlas->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyhbd.ttc", 16.0f, &arialConfig, io.Fonts->GetGlyphRangesChineseFull());
    io.Fonts = fontAtlas;

    while (!(GetKeyState(VK_DELETE) & 0x8000))
    {
        auto start = std::chrono::high_resolution_clock::now();

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }


        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        amount++;
        _cs.UpdateLocalPlayer();
        _cs.update();
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if (GetForegroundWindow() == _cs.counter_strike_hwnd) {
            ImGui::GetBackgroundDrawList()->AddText(ImVec2(10, 10), ImColor(0.3, 0.9, 1.f), std::to_string(fps).c_str());
            for (const Entity& p : _cs.BonePosList) {
                ImColor col;
                if (p.teamID == _cs.LocalPlayer.teamID) {
                        if (!_cs.LocalPlayer.alive)
                            continue;
                    col = ImColor(0.f, 1.f, 0.f);
                }
                else
                    col = ImColor(1.f, 0.f, 0.f);
                float size = fabs(p.bones[28].ScreenPos.y - p.bones[6].ScreenPos.y);
                for (int i = 0; i < 6; i++) {
                    if (p.bones[i].IsVisible && p.bones[i + 1].IsVisible)
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[i].ScreenPos.x, p.bones[i].ScreenPos.y), ImVec2(p.bones[i + 1].ScreenPos.x, p.bones[i + 1].ScreenPos.y), col, 1);
                }
                for (int i = 8; i < 11; i++) {
                    if (p.bones[i].IsVisible && p.bones[i + 1].IsVisible)
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[i].ScreenPos.x, p.bones[i].ScreenPos.y), ImVec2(p.bones[i + 1].ScreenPos.x, p.bones[i + 1].ScreenPos.y), col, 1);
                }
                for (int i = 13; i < 16; i++) {
                    if (p.bones[i].IsVisible && p.bones[i + 1].IsVisible)
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[i].ScreenPos.x, p.bones[i].ScreenPos.y), ImVec2(p.bones[i + 1].ScreenPos.x, p.bones[i + 1].ScreenPos.y), col, 1);
                }
                for (int i = 22; i < 24; i++) {
                    if (p.bones[i].IsVisible && p.bones[i + 1].IsVisible)
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[i].ScreenPos.x, p.bones[i].ScreenPos.y), ImVec2(p.bones[i + 1].ScreenPos.x, p.bones[i + 1].ScreenPos.y), col, 1);
                }
                for (int i = 25; i < 27; i++) {
                    if (p.bones[i].IsVisible && p.bones[i + 1].IsVisible)
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[i].ScreenPos.x, p.bones[i].ScreenPos.y), ImVec2(p.bones[i + 1].ScreenPos.x, p.bones[i + 1].ScreenPos.y), col, 1);
                }
                if (p.bones[0].IsVisible && p.bones[25].IsVisible)
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[0].ScreenPos.x, p.bones[0].ScreenPos.y), ImVec2(p.bones[25].ScreenPos.x, p.bones[25].ScreenPos.y), col, 1);
                if (p.bones[0].IsVisible && p.bones[22].IsVisible)
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[0].ScreenPos.x, p.bones[0].ScreenPos.y), ImVec2(p.bones[22].ScreenPos.x, p.bones[22].ScreenPos.y), col, 1);

                if (p.bones[5].IsVisible && p.bones[13].IsVisible)
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[5].ScreenPos.x, p.bones[5].ScreenPos.y), ImVec2(p.bones[13].ScreenPos.x, p.bones[13].ScreenPos.y), col, 1);
                if (p.bones[5].IsVisible && p.bones[8].IsVisible)
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[5].ScreenPos.x, p.bones[5].ScreenPos.y), ImVec2(p.bones[8].ScreenPos.x, p.bones[8].ScreenPos.y), col, 1);

                if (!p.bones[6].IsVisible || !p.bones[28].IsVisible)
                    continue;
                if (p.teamID != _cs.LocalPlayer.teamID && _cs.LocalPlayer.alive) {
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(p.bones[28].ScreenPos.x, p.bones[28].ScreenPos.y), ImVec2(_cs.screen.x / 2, _cs.screen.y), ImColor(0.95, 0.3, 1.f, 0.8), 1);
                }
                ImVec2 p1 = ImVec2(p.bones[6].ScreenPos.x + size / 3, p.bones[6].ScreenPos.y);
                ImVec2 p2 = ImVec2(p.bones[28].ScreenPos.x + size / 3, p.bones[28].ScreenPos.y);
                float rat = (float)p.health / (float)p.maxHealth;
                ImVec2 p3 = ImVec2((1 - rat) * p2.x + rat * p1.x, (1 - rat) * p2.y + rat * p1.y);
                ImGui::GetBackgroundDrawList()->AddLine(p3, p2, ImColor(0.f, 1.f, 0.f), 2);
                ImGui::GetBackgroundDrawList()->AddLine(p1, p3, ImColor(1.f, 1.f, 1.f), 2);
                ImVec2 textSize = ImGui::CalcTextSize(p.name);
                ImGui::GetBackgroundDrawList()->AddText(ImVec2(p.namePos.x - textSize.x / 2.0f, p.namePos.y - textSize.y / 2.0f), ImColor(0.6, 0.95, 1.f), p.name);
            }
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(0, 0);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        while (duration.count() < frametime) {
            stop = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 144;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
