#pragma once
#include <windows.h>
#include <string>

namespace Storm_Config {
    struct Settings {
        // Ragebot & Exploits (Fatality 14-tick)
        bool ragebot_active = true;
        bool silent_aim = true;
        bool no_spread = true;
        bool rapid_fire = true;
        int rapid_fire_ticks = 14; 
        bool force_shot_in_air = true;
        bool force_body_aim = true;
        int point_scale = 55;
        int hitchance = 55;
        int hc_override = 38;
        int min_damage = 45;
        
        // Menu GUI & Visuals (Nón lá & Vòng tròn độ trượt)
        bool menu_opened = true;
        int menu_key = VK_INSERT;            // Phím INSERT ẩn/hiện GUI
        bool third_person = false;
        int third_person_key = 0x04;         // Nút cuộn chuột giữa bật 3rd POV
        bool hit_trace = true;
        bool hit_sound = true;
        float shader_bloom = 1.8f;
        bool skin_changer = true;
        int skin_id = 624;                   // Mã skin Gungnir/Dragon Lore
    };
    
    // Khai báo biến global chạy xuyên suốt tất cả các file .cpp của bạn
    inline Settings cfg;
}
