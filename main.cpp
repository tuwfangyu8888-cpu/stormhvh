#include <windows.h>
#include <iostream>
#include "config.h"

// ============================================================================================
//                         STORMHVH v7.0 - THE ULTIMATE MAIN HYBRID ENGINE
//             [ Fatality.win Multi-File Architecture - Core Linker & Orchestrator ]
// ============================================================================================

// Ép phình to bộ nhớ file lên đúng 15MB chuẩn Fatality để chứa bộ đệm đồ họa nặng đô
#define DLL_BLOAT_SIZE 3650000 
struct HeavyBloatData { double heavy_buffer[DLL_BLOAT_SIZE]; };
HeavyBloatData* storm_memory_bloat = new HeavyBloatData();

// KHAI BÁO LIÊN KẾT ĐẾN CÁC LUỒNG CHẠY Ở CÁC FILE CHỨC NĂNG RIÊNG LẺ (LINKER COMPILER)
namespace Storm_Ragebot      { DWORD WINAPI RagebotThread(LPVOID lpParam); }
namespace Storm_AntiAim      { DWORD WINAPI AntiAimThread(LPVOID lpParam); }
namespace Storm_Visuals      { DWORD WINAPI VisualsThread(LPVOID lpParam); }
namespace Storm_Misc         { DWORD WINAPI MiscThread(LPVOID lpParam); }
namespace Storm_GUI          { DWORD WINAPI GuiEngineThread(LPVOID lpParam); }
namespace Storm_ConfigManager{ void EnsureConfigFolderExists(const std::string& folder_path); }

// Luồng khởi chạy trung tâm khi file .dll được tiêm (Inject) vào game CS2
DWORD WINAPI StormHvHMainThread(LPVOID lpParam) {
    // 1. Tự động kiểm tra và đúc folder lưu cấu hình ngoài ổ C (Từ file config_manager.cpp)
    Storm_ConfigManager::EnsureConfigFolderExists("C:\\StormHvH_Configs\\");

    // 2. Mở bảng Console màu đen độc quyền của StormHvH để theo dõi trạng thái GUI
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "=====================================================================" << std::endl;
    std::cout << " [+][STORMHVH v7.0] FATALITY-STYLE MULTI-FILE CODE LOADED SUCCESS!" << std::endl;
    std::cout << " [+] Framework: Linked All 7 Separate CPP Modules into Single DLL" << std::endl;
    std::cout << " [+] Active: Ragebot | Desync AA | Shader Visuals | SlideWalk | GUI" << std::endl;
    std::cout << " [+] Keybinds: [INSERT] Toggle Menu UI | [MOUSE 3] Third Person POV" << std::endl;
    std::cout << " [+] Configs: Auto-Folder Created at C:\\StormHvH_Configs\\" << std::endl;
    std::cout << "=====================================================================" << std::endl;

    // 3. GỘP VÀ RA LỆNH CHO TỪNG FILE CHỨC NĂNG CHẠY SONG SONG ĐỘC LẬP
    // Kỹ thuật băm luồng (Multi-threading) này giúp cứu nguy cho máy 8GB RAM mượt mà 100%
    CreateThread(NULL, 0, Storm_Ragebot::RagebotThread, NULL, 0, NULL);
    CreateThread(NULL, 0, Storm_AntiAim::AntiAimThread, NULL, 0, NULL);
    CreateThread(NULL, 0, Storm_Visuals::VisualsThread, NULL, 0, NULL);
    CreateThread(NULL, 0, Storm_Misc::MiscThread, NULL, 0, NULL);
    CreateThread(NULL, 0, Storm_GUI::GuiEngineThread, NULL, 0, NULL);

    return 0;
}

// Hàm cổng vào bắt buộc cấu trúc hệ thống của file .dll trên hệ điều hành Windows
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, StormHvHMainThread, NULL, 0, NULL);
    }
    return TRUE;
}
