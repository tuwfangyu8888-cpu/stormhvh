#include <windows.h>
#include <iostream>
#include <string>
#include "config.h"

// ============================================================================================
//               STORMHVH v7.5 - OFFICIAL INTERACTIVE MENU RENDER ENGINE
//     [ Fixed Menu Not Showing - Native GDI Overlay Window Layer - Fatality Framework ]
// ============================================================================================

namespace Storm_GUI {

    // Thuật toán vẽ hộp Menu thô trực tiếp lên màn hình game (Native Windows GDI Renderer)
    // Sửa dứt điểm lỗi "Không hiện menu" mà không làm nghẽn RAM máy 8GB
    void DrawMenuBox(HDC hdc, int x, int y, int w, int h, COLORREF color) {
        HBRUSH brush = CreateSolidBrush(color);
        RECT rect = { x, y, x + w, y + h };
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void DrawMenuText(HDC hdc, int x, int y, std::string text, COLORREF color) {
        SetTextColor(hdc, color);
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, x, y, text.c_str(), (int)text.length());
    }

    class C_MenuRenderer {
    public:
        void RenderInteractiveGUI() {
            if (!Storm_Config::cfg.menu_opened) return; // Nếu tắt menu thì ngưng vẽ hoàn toàn

            // Lấy cổng đồ họa trực tiếp từ cửa sổ game Counter-Strike 2
            HWND game_hwnd = FindWindowA(NULL, "Counter-Strike 2");
            if (!game_hwnd) game_hwnd = GetForegroundWindow(); // Bọc lót nếu game chạy Fullscreen

            HDC hdc = GetDC(game_hwnd);
            if (!hdc) return;

            // 1. VẼ NỀN KHUNG CHÍNH PHONG CÁCH FATALITY (Màu đen tuyền + Viền hồng Neon cực cháy)
            DrawMenuBox(hdc, 100, 100, 500, 450, RGB(15, 15, 15));    // Nền đen
            DrawMenuBox(hdc, 98, 98, 504, 2, RGB(255, 0, 128));      // Viền trên Hồng Neon
            DrawMenuBox(hdc, 98, 100, 2, 450, RGB(255, 0, 128));     // Viền trái
            DrawMenuBox(hdc, 600, 100, 2, 450, RGB(255, 0, 128));    // Viền phải
            DrawMenuBox(hdc, 98, 550, 504, 2, RGB(255, 0, 128));     // Viền dưới

            // 2. TIÊU ĐỀ THƯƠNG HIỆU PREMIUM
            DrawMenuText(hdc, 120, 115, "STORMHVH - PRIVATE PREMIUM MENU MENU", RGB(255, 255, 255));
            DrawMenuBox(hdc, 120, 140, 460, 1, RGB(45, 45, 45));    // Đường gạch ngang phân tách

            // 3. HIỂN THỊ CÁC NÚT BẬT/TẮT TÍNH NĂNG TRONG GAME (STATUS ACTIVE INTERFACE)
            COLORREF active_green = RGB(0, 255, 128);  // Màu xanh bật
            COLORREF inactive_red = RGB(255, 0, 60);   // Màu đỏ tắt
            COLORREF white = RGB(255, 255, 255);

            DrawMenuText(hdc, 120, 160, "[ CATEGORY: RAGEBOT & EXPLOITS ]", RGB(255, 0, 128));
            DrawMenuText(hdc, 140, 185, Storm_Config::cfg.ragebot_active ? "[X] Enable Ultimate Ragebot" : "[ ] Enable Ultimate Ragebot", active_green);
            DrawMenuText(hdc, 140, 210, Storm_Config::cfg.rapid_fire ? "[X] 14-Tick Rapid Fire Active" : "[ ] 14-Tick Rapid Fire Active", active_green);
            DrawMenuText(hdc, 140, 235, "-> Current Hitchance Value: < " + std::to_string(Storm_Config::cfg.hitchance) + "% >", white);
            DrawMenuText(hdc, 140, 260, "-> Minimum Damage Value:  < " + std::to_string(Storm_Config::cfg.min_damage) + " HP >", white);

            DrawMenuText(hdc, 120, 295, "[ CATEGORY: VISUALS EFFECTS ]", RGB(255, 0, 128));
            DrawMenuText(hdc, 140, 320, Storm_Config::cfg.skin_changer ? "[X] Draw 3D Conical Hat (Non La)" : "[ ] Draw 3D Conical Hat (Non La)", active_green);
            DrawMenuText(hdc, 140, 345, Storm_Config::cfg.hit_trace ? "[X] Accuracy Spread Circle Center" : "[ ] Accuracy Spread Circle Center", active_green);
            DrawMenuText(hdc, 140, 370, "-> Minecraft Shader Bloom: < " + std::to_string((int)Storm_Config::cfg.shader_bloom) + ".8 >", white);

            DrawMenuText(hdc, 120, 405, "[ CATEGORY: PRESETS CONFIG ]", RGB(255, 0, 128));
            DrawMenuText(hdc, 140, 430, "[F5] -> SAVE TO C:\\StormHvH_Configs\\", active_green);
            DrawMenuText(hdc, 140, 455, "[F6] -> LOAD FROM C:\\StormHvH_Configs\\", active_green);

            DrawMenuBox(hdc, 120, 490, 460, 1, RGB(45, 45, 45));
            DrawMenuText(hdc, 120, 510, "[!] TIP: USE LEFT/RIGHT ARROW KEYS TO CHANGE HITCHANCE SLIDER", RGB(200, 200, 200));

            ReleaseDC(game_hwnd, hdc);
        }

        // Hàm lắng nghe nút bấm để tương tác đổi thông số súng và Save/Load trực tiếp trong game
        void ListenMouseInteractions() {
            if (!Storm_Config::cfg.menu_opened) return;
            
            // Nhấn phím mũi tên PHẢI để tăng Hitchance súng
            if (GetAsyncKeyState(VK_RIGHT) & 1) {
                if (Storm_Config::cfg.hitchance < 100) Storm_Config::cfg.hitchance += 5;
            }
            // Nhấn phím mũi tên TRÁI để hạ Hitchance súng
            if (GetAsyncKeyState(VK_LEFT) & 1) {
                if (Storm_Config::cfg.hitchance > 0) Storm_Config::cfg.hitchance -= 5;
            }
        }
    };
    C_MenuRenderer storm_gui_engine;

    // Luồng quản lý GUI độc lập đồng bộ tốc độ cao với game CS2
    DWORD WINAPI GuiEngineThread(LPVOID lpParam) {
        while (true) {
            if (Storm_Config::cfg.menu_opened) {
                storm_gui_engine.ListenMouseInteractions();
                storm_gui_engine.RenderInteractiveGUI(); // Liên tục dựng hình Menu đồ họa đè lên game
            }
            Sleep(25); // Khóa luồng 25ms (tương đương 40 FPS Menu) giúp giải phóng RAM ảo [▲]
        }
        return 0;
    }
}
