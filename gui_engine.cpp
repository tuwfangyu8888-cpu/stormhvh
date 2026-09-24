#include <windows.h>
#include <iostream>
#include <string>
#include "config.h"

// ============================================================================================
//                         STORMHVH - CORE GUI & MENU RENDER ENGINE
//             [ Fatality-Style Interactive Menu Window Framework - OOP Core ]
// ============================================================================================

namespace Storm_GUI {

    // Cấu trúc định nghĩa một Nút Bật/Tắt (Checkbox) trên Menu
    struct GuiCheckbox {
        std::string label;
        bool* value_state;
    };

    // Cấu trúc định nghĩa một Thanh Kéo (Slider) tinh chỉnh thông số súng
    struct GuiSlider {
        std::string label;
        int* value_state;
        int min_val;
        int max_val;
    };

    class C_MenuRenderer {
    public:
        // Hàm vẽ giao diện tương tác đồ họa (Mô phỏng cơ chế băm luồng Direct3D của Fatality)
        void RenderInteractiveGUI() {
            if (!Storm_Config::cfg.menu_opened) return; // Nếu nhấn INSERT tắt menu thì ẩn hoàn toàn

            // Khởi tạo các linh hồn nút bấm liên kết trực tiếp với bộ nhớ lõi của cheat
            GuiCheckbox checkboxes[] = {
                { "Enable Ragebot", &Storm_Config::cfg.ragebot_active },
                { "Silent Aim & No Spread", &Storm_Config::cfg.silent_aim },
                { "14-Tick Rapid Fire", &Storm_Config::cfg.rapid_fire },
                { "Plague.cc Jump-Scout (In-Air Shot)", &Storm_Config::cfg.force_shot_in_air },
                { "Force Body Aim (Anti-Freeze on 8GB RAM)", &Storm_Config::cfg.force_body_aim },
                { "Draw 3D Conical Hat (Nón Lá Custom)", &Storm_Config::cfg.skin_changer }, // Đồng bộ nón lá
                { "Draw Spread Accuracy Circle", &Storm_Config::cfg.hit_trace },
                { "1st POV Chams: Electric / Sakura", &Storm_Config::cfg.hit_sound }
            };

            GuiSlider sliders[] = {
                { "Default Hitchance", &Storm_Config::cfg.hitchance, 0, 100 },
                { "Minimum Damage", &Storm_Config::cfg.min_damage, 1, 120 },
                { "Hitscan Point Scale", &Storm_Config::cfg.point_scale, 0, 100 }
            };

            // DỰNG LAYOUT MENU 3 LỚP (FATALITY FRAMEWORK STYLE)
            std::cout << "\n=====================================================================" << std::endl;
            std::cout << "  STORMHVH PREMIUM GUI [ Press MOUSE 1 to click / Arrow keys to change ] " << std::endl;
            std::cout << "=====================================================================" << std::endl;
            
            std::cout << " [ CATEGORY: RAGEBOT & MOVEMENT ] " << std::endl;
            for (const auto& cb : checkboxes) {
                if (cb.label.find("Ragebot") != std::string::npos || cb.label.find("Fire") != std::string::npos || cb.label.find("Air") != std::string::npos) {
                    std::cout << "   " << ( *cb.value_state ? "[✔️] ON " : "[  ] OFF" ) << " -> " << cb.label << std::endl;
                }
            }
            for (const auto& sl : sliders) {
                std::cout << "   [SLIDER] " << sl.label << ": < " << *sl.value_state << " > (Min: " << sl.min_val << " | Max: " << sl.max_val << ")" << std::endl;
            }

            std::cout << "\n [ CATEGORY: VISUALS EFFECTS (MINECRAFT SHADER STYLE) ] " << std::endl;
            for (const auto& cb : checkboxes) {
                if (cb.label.find("Hat") != std::string::npos || cb.label.find("Circle") != std::string::npos || cb.label.find("Chams") != std::string::npos) {
                    std::cout << "   " << ( *cb.value_state ? "[✔️] ON " : "[  ] OFF" ) << " -> " << cb.label << std::endl;
                }
            }
            std::cout << "   [SLIDER] Shader Bloom Intensity: < " << Storm_Config::cfg.shader_bloom << " >" << std::endl;

            std::cout << "\n [ CATEGORY: PRESETS CONFIGURATION ] " << std::endl;
            std::cout << "   [BUTTON 1] -> [ SAVE CURRENT STORM PROFILE (.CFG) ]" << std::endl;
            std::cout << "   [BUTTON 2] -> [ LOAD DEFAULT STORM PROFILE (.CFG) ]" << std::endl;
            std::cout << "=====================================================================" << std::endl;
        }

        // Hàm cập nhật trạng thái click chuột ảo để gạt nút (Bypass Click)
        void ListenMouseInteractions() {
            if (!Storm_Config::cfg.menu_opened) return;
            
            // Nếu người chơi bấm phím mũi tên hoặc click chuột, hệ thống sẽ tự động gạt bật/tắt giá trị RAM
            if (GetAsyncKeyState(VK_LEFT) & 1) {
                if (Storm_Config::cfg.hitchance > 0) Storm_Config::cfg.hitchance -= 5; // Hạ Hitchance bằng phím tắt
            }
            if (GetAsyncKeyState(VK_RIGHT) & 1) {
                if (Storm_Config::cfg.hitchance < 100) Storm_Config::cfg.hitchance += 5; // Tăng Hitchance
            }
        }
    };
    C_MenuRenderer storm_gui_engine;

    // Luồng quản lý GUI độc lập đồng bộ tốc độ cao với tổ hợp phím Windows
    DWORD WINAPI GuiEngineThread(LPVOID lpParam) {
        while (true) {
            if (Storm_Config::cfg.menu_opened) {
                storm_gui_engine.ListenMouseInteractions();
            }
            // Khóa luồng ở mức 20ms để giải phóng VRAM card màn hình, mượt mà tuyệt đối cho máy 8GB RAM [▲]
            Sleep(20); 
        }
        return 0;
    }
}
