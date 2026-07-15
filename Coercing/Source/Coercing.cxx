#include <iostream>
#include <thread>
#include <Windows.h>
#include <algorithm>
#include <ShlObj.h>

#include "Driver/Driver.h"
#include "Globals.hxx"
#include "Miscellaneous/Output/Output.hpp"
#include "Core/Graphics/Graphics.h"
#include "Engine/Engine.h"
#include "Core/Features/Cache/Cache.h"
#include "Core/Features/Cheats/Misc/Misc.h"
#include "Core/Features/Cheats/World/World.h"
#include "Core/Features/Cheats/Aimbot/Aimbot.h"
#include "Core/Features/Cheats/Aimbot/Silent/Silent.h"
#include "Core/Features/Cheats/Misc/WallCheck.h"
#include "Core/Features/Cheats/Movement/Movement.h"
#include "Engine/Offsets/OffsetFetcher.h"

#pragma comment(lib, "Shell32.lib")

std::vector<std::string> GetRunningAntiCheats()
{
    std::vector<std::string> detected;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return detected;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            std::string processName = pe32.szExeFile;

            for (char& c : processName)
                c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

            if (processName.find("vanguard.exe") != std::string::npos ||
                processName.find("vgtray.exe") != std::string::npos)
                detected.push_back("Vanguard");

            if (processName.find("easyanticheat") != std::string::npos ||
                processName.find("eaclauncher") != std::string::npos ||
                processName.find("eac") != std::string::npos)
                detected.push_back("Easy Anti-Cheat");

            if (processName.find("beservice") != std::string::npos ||
                processName.find("belauncher") != std::string::npos ||
                processName.find("battleye") != std::string::npos)
                detected.push_back("BattlEye");

            if (processName.find("faceitac") != std::string::npos ||
                processName.find("faceit") != std::string::npos)
                detected.push_back("FACEIT");

            if (processName.find("ricochet") != std::string::npos)
                detected.push_back("Ricochet");

        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return detected;
}

void PerformACCheck()
{
    auto acs = GetRunningAntiCheats();

    if (!acs.empty())
    {
        for (const auto& ac : acs)
        {
            Output::Warning("Anti-Cheat detected: {}", ac);
        }

        Output::Warning("Restart your PC after using this to avoid detection/bans.");
        Sleep(3500);
    }
}

// fixed the arguments definition sigs
std::int32_t main(std::int32_t argc, char* argv[])
{
    if (!Output::Initialize_Console())
    {
        std::printf("[!] Console framework initialization failure.\n");
        return 1;
    }

    PerformACCheck();
	
    //just for testing the console. not needed for the actual product
    //Output::Info("This is an info message - for well.. info");
    //Output::Error("This is an error message - for well.. errors");
    //Output::Warning("This is a warning message - for well.. warnings");
    //Output::Success("This is a success message - for well.. successes");

    static constexpr const char* BINARY_NAME = { "RobloxPlayerBeta.exe" };

    Driver->Find_Process(BINARY_NAME);
    Driver->Attach_Process(BINARY_NAME);
    Driver->Find_Module(BINARY_NAME);

    OffsetFetcher::Fetch();

    auto FakeDataModel = Driver->Read<std::uint64_t>(Driver->Get_Module() + Offsets::FakeDataModel::Pointer);
    Globals::Datamodel.Address = Driver->Read<std::uint64_t>(FakeDataModel + Offsets::FakeDataModel::RealDataModel);
    Globals::VisualEngine.Address = Driver->Read<std::uint64_t>(Driver->Get_Module() + Offsets::VisualEngine::Pointer);
    Globals::Players.Address = Globals::Datamodel.Find_First_Child_Of_Class("Players").Address;
    Globals::Workspace.Address = Globals::Datamodel.Find_First_Child_Of_Class("Workspace").Address;
    Globals::Camera.Address = Globals::Workspace.Find_First_Child_Of_Class("Camera").Address;
    auto Lightin = Globals::Datamodel.Find_First_Child_Of_Class("Lighting");
    Globals::Lighting = SDK::Lighting(Lightin.Address);

    Output::Success("Attached -> PID {} | Handle 0x{:x} | Base 0x{:x}",
        Driver->Get_Process(), (std::uint64_t)Driver->Get_Handle(), Driver->Get_Module());

    std::thread(Cache::RunService).detach();
    std::thread(World::RunService).detach();
    std::thread(Aimbot::RunService).detach();
    std::thread(Silent::RunService).detach();
    std::thread(Movement::RunService).detach();

    wallcheck::initialize(); // wallcheck auto-rescans on game change via Cache::Rescan

    Graphic->Create_Window();
    Graphic->Create_Device();
    Graphic->Create_Imgui();

    for (;;) {
        Graphic->Start_Render();
        Graphic->Render_Visuals();
        Graphic->Render_Menu();
        Graphic->End_Render();
    }

    return 0;
}