#include <windows.h>
#include <cmath>
#include <iostream>
#include "config.h"

// ============================================================================================
//                         STORMHVH - CORE ANTI-AIM MODULE (DESYNC ENGINE)
//             [ Advanced Pitch/Yaw Jitter + Fake Lag Synchronizer + Desync Matrix ]
// ============================================================================================

struct UserCmd {
    int command_number;
    int tick_count;
    float viewangles_x;
    float viewangles_y;
    float viewangles_z;
    float forwardmove;
    float sidemove;
};

namespace Storm_AntiAim {

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 1: BỘ TÍNH TOÁN GÓC QUAY THỰC VÀ GIẢ (REAL YAW & FAKE YAW MATRIX) ]
    // Tinh hoa bẻ hướng từ Neverlose giúp tách đôi bóng nhân vật để đánh lừa Resolver địch
    // ----------------------------------------------------------------------------------------
    class C_AntiAimEngine {
    public:
        bool flip_jitter = false;

        // Thuật toán gập đầu (Pitch) chống Headshot cố định
        float GetOptimalPitch() {
            // Ép đầu nhân vật cắm xuống đất góc 89 độ giống Fatality.win (Down Pitch)
            return 89.0f; 
        }

        // Thuật toán lắc hướng người (Yaw Jitter & Desync)
        void CalculateYaw(UserCmd* cmd) {
            float base_yaw = cmd->viewangles_y + 180.0f; // Quay lưng về phía địch để giấu đầu
            flip_jitter = !flip_jitter;                  // Đảo trạng thái sau mỗi tick

            // 1. Chế độ Lắc tâm (Jitter Mode từ Fatality)
            if (flip_jitter) {
                base_yaw += 35.0f; // Lắc sang trái 35 độ
            } else {
                base_yaw -= 35.0f; // Lắc sang phải 35 độ
            }

            // 2. Kỹ thuật tạo bóng giả (Desync Góc Quay - Bản quyền từ Neverlose)
            // Ép góc quay gửi lên Server (Real) lệch hoàn toàn so với góc quay địch nhìn thấy (Fake)
            float desync_delta = 58.0f; // Góc bẻ tối đa của Engine Source 2
            if (flip_jitter) {
                cmd->viewangles_y = base_yaw + desync_delta; // Real Yaw
            } else {
                cmd->viewangles_y = base_yaw - desync_delta; // Fake Yaw
            }
        }
    };
    C_AntiAimEngine storm_aa;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 2: HỆ THỐNG ĐIỀU KHIỂN ĐỘ TRỄ GÓI TIN (FAKE LAG CONTROLLER) ]
    // Giúp nhân vật của bạn di chuyển kiểu "bóng ma" giật cục trên màn hình địch, cực khó bắn
    // ----------------------------------------------------------------------------------------
    class C_FakeLag {
    public:
        int choked_ticks = 0;
        int max_choke_limit = 14; // Giới hạn 14 Ticks tối ưu từ Fatality cho máy 8GB RAM

        void HandleFakeLag(bool* send_packet) {
            choked_ticks++;

            if (choked_ticks >= max_choke_limit) {
                *send_packet = true;   // Ra lệnh gửi gói tin đi (Gia tốc vị trí mới)
                choked_ticks = 0;      // Reset bộ đếm
            } else {
                *send_packet = false;  // Giữ gói tin lại (Choke) để tạo bóng ma di chuyển giật cục
            }
        }
    };
    C_FakeLag storm_fakelag;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 3: ĐỒNG BỘ DI CHUYỂN VÀ HOTKEYS TỪ MENU CHÍNH ]
    // Liên kết với phím tắt để bạn chủ động bẻ hướng né đạn thủ công (Manual Anti-Aim)
    // ----------------------------------------------------------------------------------------
    void ProcessManualAA(UserCmd* cmd, bool* send_packet) {
        // Mặc định Anti-Aim tự động gập đầu xuống đất
        cmd->viewangles_x = storm_aa.GetOptimalPitch();

        // Tính toán góc né đạn Desync Jitter
        storm_aa.CalculateYaw(cmd);

        // Xử lý tạo độ trễ bóng ma Fake Lag
        storm_fakelag.HandleFakeLag(send_packet);

        // TỐI ƯU CHO MÁY 8GB RAM: Nếu game đang xử lý giao tranh nặng, giảm tối đa số Tick FakeLag xuống
        if (Storm_Config::cfg.force_body_aim) {
            storm_fakelag.max_choke_limit = 4; // Hạ xuống 4 Ticks để CPU không bị đơ đứng hình
        }
    }

    // Luồng chạy độc lập quản lý Anti-Aim chạy ngầm đồng bộ với game CS2
    DWORD WINAPI AntiAimThread(LPVOID lpParam) {
        while (true) {
            // Luồng liên tục lắng nghe và bẻ hướng tọa độ di chuyển của nhân vật
            // Giả lập truyền cấu trúc UserCmd vào hệ thống game
            UserCmd dummy_cmd;
            bool dummy_send_packet = true;

            if (Storm_Config::cfg.ragebot_active) {
                ProcessManualAA(&dummy_cmd, &dummy_send_packet);
            }
            Sleep(1); // Giữ CPU ổn định ở mức 1ms
        }
        return 0;
    }
}
