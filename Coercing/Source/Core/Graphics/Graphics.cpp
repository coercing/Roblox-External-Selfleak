#include <dwmapi.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <d3d11.h>
#include <wincodec.h>
#include <vector>

#include "Graphics.h"
#include <Globals.hxx>


#include <imgui/addons/imgui_addons.h>

#include "../Features/Cheats/Visuals/Visuals.h"
#include "../Features/Cheats/World/World.h"
#include "../Features/Cheats/Misc/Explorer.h"
#include <ImGui/addons/colors/colors.h>
#include "Core/Features/Cheats/Aimbot/Silent/Silent.h"
#include "Core/Features/Cache/Cache.h"
#include "Core/Menu/Menu.h"
#include <Fonts/UiFont.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam);

LRESULT CALLBACK WndProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    if (ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, WParam, LParam))
    {
        return true;
    }

    switch (Msg)
    {
    case WM_SYSCOMMAND:
        if ((WParam & 0xfff0) == SC_KEYMENU)
        {
            return 0;
        }
        break;

    case WM_SYSKEYDOWN:
        if (WParam == VK_F4) {
            DestroyWindow(Hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        return 0;
    }

    return DefWindowProcA(Hwnd, Msg, WParam, LParam);
}

Graphics::Graphics()
{
    Detail = std::make_unique<detail_t>();
}

Graphics::~Graphics()
{
    Destroy_Imgui();
    Destroy_Window();
    Destroy_Device();
}

bool Graphics::Create_Window()
{
    Detail->WindowClass.cbSize = sizeof(Detail->WindowClass);
    Detail->WindowClass.style = CS_CLASSDC;
    Detail->WindowClass.lpszClassName = "scare.lol";
    Detail->WindowClass.hInstance = GetModuleHandleA(0);
    Detail->WindowClass.lpfnWndProc = WndProc;

    RegisterClassExA(&Detail->WindowClass);

    Detail->Window = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        Detail->WindowClass.lpszClassName, "scare.lol", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        0, 0, Detail->WindowClass.hInstance, 0);

    if (!Detail->Window)
    {
        return false;
    }

    // LWA_ALPHA at 255 keeps the window fully opaque at the OS level.
    // Transparency is handled by D3D11 clearing to alpha=0 and DWM compositing.
    SetLayeredWindowAttributes(Detail->Window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    RECT ClientArea{};
    RECT WindowArea{};

    GetClientRect(Detail->Window, &ClientArea);
    GetWindowRect(Detail->Window, &WindowArea);

    POINT Diff{};
    ClientToScreen(Detail->Window, &Diff);

    MARGINS Margins
    {
        WindowArea.left + (Diff.x - WindowArea.left),
        WindowArea.top + (Diff.y - WindowArea.top),
        WindowArea.right,
        WindowArea.bottom,
    };

    DwmExtendFrameIntoClientArea(Detail->Window, &Margins);

    ShowWindow(Detail->Window, SW_SHOW);
    UpdateWindow(Detail->Window);

    return true;
}

bool Graphics::Create_Device()
{
    DXGI_SWAP_CHAIN_DESC SwapChainDesc{};

    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

    SwapChainDesc.BufferDesc.Width = 0;
    SwapChainDesc.BufferDesc.Height = 0;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    SwapChainDesc.OutputWindow = Detail->Window;

    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDesc.Windowed = 1;

    SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;

    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    D3D_FEATURE_LEVEL FeatureLevel;
    D3D_FEATURE_LEVEL FeatureLevelList[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT Result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, FeatureLevelList, 2, D3D11_SDK_VERSION, &SwapChainDesc, &Detail->SwapChain, &Detail->Device, &FeatureLevel, &Detail->DeviceContext);

    if (Result == DXGI_ERROR_UNSUPPORTED)
    {
        Result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, FeatureLevelList, 2, D3D11_SDK_VERSION, &SwapChainDesc, &Detail->SwapChain, &Detail->Device, &FeatureLevel, &Detail->DeviceContext);
    }

    if (Result != S_OK)
    {
        MessageBoxA(nullptr, "This software can not run on your computer.", "Critical Problem", MB_ICONERROR | MB_OK);
    }

    ID3D11Texture2D* BackBuffer{ nullptr };
    Detail->SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));

    if (BackBuffer)
    {
        Detail->Device->CreateRenderTargetView(BackBuffer, nullptr, &Detail->GraphicsTargetView);
        BackBuffer->Release();

        return true;
    }

    return false;
}

bool Graphics::Create_Imgui()
{
    using namespace ImGui;
    CreateContext();
    StyleColorsDark();

    ImGuiStyle& Style = ImGui::GetStyle();
    ImGuiIO& IO = ImGui::GetIO(); (void)IO;

    IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    // Load fonts matching original Menu eternal — Tahoma family at 16px with FontAwesome merged
    UiFont::LoadAll();

    // Dedicated bold font for ESP text rendering (Visuals.cpp uses Tahoma_BoldXP)
    Tahoma_BoldXP = UiFont::GetFont(1);

    if (!ImGui_ImplWin32_Init(Detail->Window))
    {
        return false;
    }

    if (!Detail->Device || !Detail->DeviceContext)
    {
        return false;
    }

    if (!ImGui_ImplDX11_Init(Detail->Device, Detail->DeviceContext))
    {
        return false;
    }

    return true;
}

void Graphics::Destroy_Device()
{
    if (Detail->GraphicsTargetView) Detail->GraphicsTargetView->Release();
    if (Detail->SwapChain) Detail->SwapChain->Release();
    if (Detail->DeviceContext) Detail->DeviceContext->Release();
    if (Detail->Device) Detail->Device->Release();
}

void Graphics::Destroy_Window()
{
    DestroyWindow(Detail->Window);
    UnregisterClassA(Detail->WindowClass.lpszClassName, Detail->WindowClass.hInstance);
}

void Graphics::Destroy_Imgui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Graphics::Start_Render()
{

    MSG Msg;
    while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    if (Globals::Settings::Streamproof)
    {
        SetWindowDisplayAffinity(Detail->Window, WDA_EXCLUDEFROMCAPTURE);
    }
    else
    {
        SetWindowDisplayAffinity(Detail->Window, WDA_NONE);
    }

    static bool LastConsoleState = false;
    if (Globals::Settings::Hide_Console != LastConsoleState)
    {
        HWND ConsoleWindow = GetConsoleWindow();
        if (ConsoleWindow && IsWindow(ConsoleWindow))
        {
            ShowWindow(ConsoleWindow, Globals::Settings::Hide_Console ? SW_HIDE : SW_SHOW);
        }
        LastConsoleState = Globals::Settings::Hide_Console;
    }

    int ExitKeyVk = ImGuiKeyToVK(Globals::Settings::Exit_Key);
    if (ExitKeyVk && (GetAsyncKeyState(ExitKeyVk) & 1))
    {
        World::RestoreAll();
        Globals::Aimbot::Enabled    = false;
        Globals::Silent::Enabled    = false;
        Globals::Movement::Speed    = false;
        Globals::Movement::JumpPower = false;
        Globals::Movement::Macro    = false;
        Globals::Visuals::Enabled   = false;
        Globals::Misc::Fly          = false;
        exit(0);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // Font switching is handled by UiFont::SwitchTo() in Menu.cpp — no rebuild needed

    ImGui::NewFrame();

    {
        auto& c = Globals::Settings::AccentColor;
        Menu::Accent = IM_COL32((int)(c[0]*255), (int)(c[1]*255), (int)(c[2]*255), (int)(c[3]*255));
        Menu::DarkAccent = IM_COL32((int)(c[0]*64), (int)(c[1]*64), (int)(c[2]*64), 255);
    }

    HWND RobloxWnd = FindWindowA(nullptr, "Roblox");
    if (RobloxWnd) {
        RECT r{};
        GetClientRect(RobloxWnd, &r);

        POINT tl{ r.left, r.top }, br{ r.right, r.bottom };
        ClientToScreen(RobloxWnd, &tl);
        ClientToScreen(RobloxWnd, &br);
        Detail->RobloxRect = { tl.x, tl.y, br.x, br.y };

        HWND ForegroundWnd = GetForegroundWindow();
        bool RobloxFocused = (ForegroundWnd == RobloxWnd || ForegroundWnd == Detail->Window);
        SetWindowPos(Detail->Window,
            RobloxFocused ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    g_MenuOpen = Running;

    int MenuToggleVk = ImGuiKeyToVK(Globals::Settings::Menu_Toggle_Key);
    if (MenuToggleVk && (GetAsyncKeyState(MenuToggleVk) & 1))
    {
        Running = !Running;
        g_MenuOpen = Running;

        if (Running)
        {
            SetWindowLong(Detail->Window, GWL_EXSTYLE,
                WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        else
        {
            SetWindowLong(Detail->Window, GWL_EXSTYLE,
                WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
    }
}

void Graphics::End_Render()
{
    ImGui::Render();

    float ClearColor[4]{ 0, 0, 0, 0 };
    Detail->DeviceContext->OMSetRenderTargets(1, &Detail->GraphicsTargetView, nullptr);
    Detail->DeviceContext->ClearRenderTargetView(Detail->GraphicsTargetView, ClearColor);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (Globals::Settings::Performance_Mode == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        Detail->SwapChain->Present(0, 0);
    }
    else if (Globals::Settings::Performance_Mode == 1)
    {
        Detail->SwapChain->Present(1, 0);
    }
    else
    {
        Detail->SwapChain->Present(0, 0);
    }
}

static void DrawCursor()
{
    if (!SilentAimInstance.Address)
    {
        return;
    }

    bool is_visible = false;
    is_visible = Driver->Read<bool>(SilentAimInstance.Address + Offsets::GuiObject::Visible);

    if (!is_visible)
    {
        return;
    }

    POINT pt;
    if (!GetCursorPos(&pt))
    {
        return;
    }

    bool right_click_held = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
    float gap = right_click_held ? 4.0f : 10.0f;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImU32 col = IM_COL32(255, 255, 255, 255);
    float dot_size = 4.0f;
    float line_width = 2.0f;
    float line_length = 10.0f;
    ImVec2 center = { (float)pt.x, (float)pt.y };
    ImVec2 dot_min(center.x - dot_size * 0.5f, center.y - dot_size * 0.5f);
    ImVec2 dot_max(center.x + dot_size * 0.5f, center.y + dot_size * 0.5f);
    draw->AddRectFilled(dot_min, dot_max, col, 0.0f);
    ImVec2 top_min(center.x - line_width * 0.5f, center.y - gap - line_length);
    ImVec2 top_max(center.x + line_width * 0.5f, center.y - gap);
    draw->AddRectFilled(top_min, top_max, col, 0.0f);
    ImVec2 bottom_min(center.x - line_width * 0.5f, center.y + gap);
    ImVec2 bottom_max(center.x + line_width * 0.5f, center.y + gap + line_length);
    draw->AddRectFilled(bottom_min, bottom_max, col, 0.0f);
    ImVec2 left_min(center.x - gap - line_length, center.y - line_width * 0.5f);
    ImVec2 left_max(center.x - gap, center.y + line_width * 0.5f);
    draw->AddRectFilled(left_min, left_max, col, 0.0f);
    ImVec2 right_min(center.x + gap, center.y - line_width * 0.5f);
    ImVec2 right_max(center.x + gap + line_length, center.y + line_width * 0.5f);
    draw->AddRectFilled(right_min, right_max, col, 0.0f);
}

void Graphics::Render_Menu()
{
    // ── Overlay fade — tied only to the main menu toggle ──
    static float overlayFade = 0.f;
    float dt = ImGui::GetIO().DeltaTime;
    float rate = g_MenuOpen ? 6.0f : 7.5f;
    overlayFade += ((g_MenuOpen ? 1.0f : 0.0f) - overlayFade) * (1.0f - std::exp(-rate * dt));
    if (overlayFade > 0.999f) overlayFade = 1.0f;

    if (overlayFade > 0.001f)
    {
        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        ImVec2 screen = ImGui::GetIO().DisplaySize;
        float dark = Menu::Theme::GetOverlayDarkness();
        bg->AddRectFilled(ImVec2(0, 0), screen,
            IM_COL32(0, 0, 0, static_cast<int>(dark * 255.f * overlayFade)));
    }

    Menu::Render();
    ExtExplorer::RenderWindow(Detail->Device);
}

void Graphics::Render_Visuals()
{
    // Don't render ESP while reattaching to a new Roblox process
    if (Cache::Reattaching.load()) return;

    DrawCursor();
    Visuals::RunService();
}