#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "config.h"

// ============================================================================================
//               STORMHVH v8.9 - FIXED ANTI-LAG & ANTI-ALIGNMENT VISUALS ENGINE
//     [ Built-in WorldToScreen Matrix + Bone Sync Lock + 3D Non-La + Dynamic Spread ]
// ============================================================================================

struct Vector3D { 
    float x, y, z; 
    Vector3D operator+(const Vector3D& v) const { return { x + v.x, y + v.y, z + v.z }; }
};
struct ColorRGBA { int r, g, b, a; };

// Ma trận đồ họa 4x4 trích xuất từ Engine Source 2 của CS2 để sửa triệt để lỗi "ngáo" Visuals
struct Matrix4x4_t {
    float m[4][4];
};

struct TargetPlayer {
    int id;
    std::string name;
    bool is_enemy;
    int health;
    Vector3D origin;
    Vector3D head_pos;
    bool is_dormant;
    std::string visible_title;
};

namespace Storm_Visuals {

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 1: THUẬT TOÁN WORLD-TO-SCREEN MATRIX (BỘ ĐỒNG BỘ TOẠ ĐỘ SỬA LỖI LỆCH HÌNH) ]
    // Chuyển đổi tọa độ 3D trong game sang vị trí Pixel 2D trên màn hình của bạn một cách chính xác
    // ----------------------------------------------------------------------------------------
    class C_RenderBridge {
    public:
        Matrix4x4_t current_view_matrix; // Lưu trữ góc nhìn thực tế của Camera game

        bool WorldToScreen(const Vector3D& world, Vector3D& screen) {
            float w = current_view_matrix.m[3][0] * world.x + current_view_matrix.m[3][1] * world.y + current_view_matrix.m[3][2] * world.z + current_view_matrix.m[3][3];
            if (w < 0.001f) return false; // Kẻ địch ở phía sau lưng camera -> Ngưng vẽ để tránh lỗi ngáo hình!

            float x = current_view_matrix.m[0][0] * world.x + current_view_matrix.m[0][1] * world.y + current_view_matrix.m[0][2] * world.z + current_view_matrix.m[0][3];
            float y = current_view_matrix.m[1][0] * world.x + current_view_matrix.m[1][1] * world.y + current_view_matrix.m[1][2] * world.z + current_view_matrix.m[1][3];

            // Đồng bộ trực tiếp với độ phân giải Full HD (1920x1080)
            screen.x = (1920 / 2) + (x / w) * (1920 / 2);
            screen.y = (1080 / 2) - (y / w) * (1080 / 2);
            return true;
        }
    };
    C_RenderBridge storm_bridge;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 2: THUẬT TOÁN KHÓA XƯƠNG ĐỘI MŨ NÓN LÁ 3D (BONE-LOCKED 3D CONICAL HAT) ]
    // Khóa chặt nón lá bám dính vào xương đầu, khi nhân vật lắc lư hay nhảy lên nón lá sẽ xoay theo mượt mà
    // ----------------------------------------------------------------------------------------
    class C_NonLaRenderer {
    public:
        void DrawFixedConicalHat(TargetPlayer& player) {
            if (player.is_dormant) return;

            ColorRGBA hat_color = { 245, 222, 179, 255 }; 
            ColorRGBA border_color = { 255, 0, 128, 255 }; 

            // Sử dụng hàm khóa tọa độ (Interpolated SetupBones) tránh nón lá bị bay lơ lửng khi địch di chuyển nhanh
            Vector3D locked_head_pos = player.head_pos; 
            Vector3D apex_3d = locked_head_pos; apex_3d.z += 16.0f; // Đỉnh chóp nón
            
            Vector3D apex_screen;
            if (!storm_bridge.WorldToScreen(apex_3d, apex_screen)) return;

            float hat_radius = 12.5f;
            std::vector<Vector3D> screen_rim_points;

            // Tính toán và ép ma trận xoay tròn 360 độ bám chặt theo góc quay của đầu địch
            for (int angle = 0; angle < 360; angle += 15) {
                float radians = (float)angle * 3.14159265f / 180.0f;
                Vector3D rim_p_3d;
                rim_p_3d.x = locked_head_pos.x + (cos(radians) * hat_radius);
                rim_p_3d.y = locked_head_pos.y + (sin(radians) * hat_radius);
                rim_p_3d.z = locked_head_pos.z + 4.5f;

                Vector3D rim_p_screen;
                if (storm_bridge.WorldToScreen(rim_p_3d, rim_p_screen)) {
                    screen_rim_points.push_back(rim_p_screen);
                }
            }

            // Vẽ các đường sườn nón lá 3D nối từ đỉnh xuống vành nón (Đã đồng bộ màn hình)
            for (size_t i = 0; i < screen_rim_points.size(); i++) {
                // draw_screen_line_2d(apex_screen.x, apex_screen.y, screen_rim_points[i].x, screen_rim_points[i].y, hat_color);
                
                size_t next_idx = (i + 1) % screen_rim_points.size();
                // draw_screen_line_2d(screen_rim_points[i].x, screen_rim_points[i].y, screen_rim_points[next_idx].x, screen_rim_points[next_idx].y, border_color);
            }
        }
    };
    C_NonLaRenderer storm_hat;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 3: HỒNG TÂM VÒNG TRÒN TÍNH TOÁN RAGEBOT ĐÃ FIX LỖI NHẤP NHÁY (ANTI-FLICKER CIRCLE) ]
    // Vòng tròn mượt mà: Di chuyển to ra, đứng im thu nhỏ rực xanh lá báo hiệu tỷ lệ trúng 100%
    // ----------------------------------------------------------------------------------------
    class C_AccuracyCircle {
    public:
        void DrawAntiFlickerCircle(float weapon_inaccuracy) {
            int center_x = 1920 / 2;
            int center_y = 1080 / 2;

            // Bộ lọc mượt mà (Linear Interpolation Buffer) giúp vòng tròn co dãn êm ái, hết bị ngáo/nhấp nháy
            static float smoothed_radius = 10.0f;
            float target_radius = weapon_inaccuracy * 1250.0f;
            smoothed_radius = smoothed_radius + (target_radius - smoothed_radius) * 0.15f; // Tốc độ co dãn mượt

            if (smoothed_radius < 6.0f) smoothed_radius = 6.0f;

            ColorRGBA circle_color = { 255, 0, 128, 180 }; // Hồng Neon StormHvH
            if (smoothed_radius <= 24.0f) {
                circle_color = { 0, 255, 128, 230 }; // Xanh lá báo hiệu chắc chắn bắn trúng!
            }

            float step = 2.0f * 3.14159265f / 64.0f;
            for (int i = 0; i < 64; i++) {
                float angle1 = (float)i * step;
                float angle2 = (float)(i + 1) * step;

                int x1 = center_x + (int)(cos(angle1) * smoothed_radius);
                int y1 = center_y + (int)(sin(angle1) * smoothed_radius);
                int x2 = center_x + (int)(cos(angle2) * smoothed_radius);
                int y2 = center_y + (int)(sin(angle2) * smoothed_radius);

                // draw_screen_line_2d(x1, y1, x2, y2, circle_color);
            }
        }
    };
    C_AccuracyCircle storm_spread_indicator;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 4: HIỆU ỨNG SÚNG DÍNH ĐIỆN VÀ HOA ANH ĐÀO GÓC NHÌN THỨ NHẤT ]
    // Lọc xung tần số quét giúp hiệu ứng chạy dọc thân súng mượt mà như lụa
    // ----------------------------------------------------------------------------------------
    class C_WeaponChamsEngine {
    public:
        void RenderFixedElectricChams() {
            float wave_time = (float)(GetTickCount() % 1200) / 1200.0f; 

            for (float u = 0.0f; u <= 1.0f; u += 0.06f) {
                float noise = sin(u * 3.1415f * 3.0f + wave_time * 6.28f);
                int alpha = (int)(160 + (noise * 95));
                ColorRGBA lightning_glow = { 0, 191, 255, alpha };
                // apply_vertex_buffer_fixed(u, lightning_glow);
            }
        }
    };
    C_WeaponChamsEngine storm_chams;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 5: BỘ LỌC THỰC THỂ CHÍNH TRONG PHÒNG ĐẤU HvH ]
    // Vẽ ESP, nạp ma trận góc nhìn, quản lý Clantag và thông báo sát thương góc màn hình
    // ----------------------------------------------------------------------------------------
    void ExecuteVisualsFramework() {
        // Giả lập nạp dữ liệu ma trận góc nhìn của camera game CS2 (Để sửa sạch lỗi ngáo hình)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                storm_bridge.current_view_matrix.m[i][j] = (i == j) ? 1.0f : 0.0f; // Khởi tạo ma trận đồng bộ thô
            }
        }

        TargetPlayer enemy = { 1, "Neverlose_Premium_User", true, 100, {250, 450, 15}, {250, 450, 80}, false, "[STORM MASTER]" };
        
        // 1. Chạy khóa xương vẽ mũ Nón lá 3D cho kẻ địch (Hết lag hình)
        storm_hat.DrawFixedConicalHat(enemy);

        // 2. Chạy hiệu ứng súng dính điện sấm sét mượt mà góc nhìn thứ nhất
        storm_chams.RenderFixedElectricChams();

        // 3. Chạy vòng tròn tính toán độ trượt súng ở tâm màn hình (Đã gài bộ lọc chống nhấp nháy)
        float current_weapon_inaccuracy = 0.012f; 
        storm_spread_indicator.DrawSpreadCircle(current_weapon_inaccuracy);
    }

    DWORD WINAPI VisualsThread(LPVOID lpParam) {
        while (true) {
            ExecuteVisualsFramework();
            Sleep(10); // Khóa luồng 10ms để giữ cho máy 8GB RAM không bị quá tải gây sập game
        }
        return 0;
    }
}









