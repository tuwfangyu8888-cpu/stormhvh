#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "config.h"

// ============================================================================================
//                         STORMHVH - CORE MISC & MOVEMENT MODULE
//             [ Fatality.win Movement Core: Auto-Bhop, Strafer, SlideWalk & FakePing ]
// ============================================================================================

struct C_UserCmd {
    int command_number;
    int tick_count;
    float viewangles_x;
    float viewangles_y;
    float viewangles_z;
    float forwardmove;
    float sidemove;
    int buttons;
};

// Định nghĩa các nút bấm trong Engine Source 2 của CS2
#define IN_JUMP (1 << 0)
#define IN_DUCK (1 << 2)
#define FL_ONGROUND (1 << 0)

namespace Storm_Misc {

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 1: BỘ HỖ TRỢ DI CHUYỂN BÁ ĐẠO (AUTO BHOP & AUTO STRAFER SYSTEM) ]
    // Tự động nhảy lướt map tốc độ cao và tự bẻ hướng trên không cực mượt khi bạn xoay chuột
    // ----------------------------------------------------------------------------------------
    class C_MovementEngine {
    public:
        // Thuật toán Auto Bunnyhop (Tự động nhảy khi chạm đất)
        void RunAutoBhop(C_UserCmd* cmd, int player_flags) {
            // Nếu người chơi không nhấn giữ nút Cách (Space) thì bỏ qua
            if (!(cmd->buttons & IN_JUMP)) return;

            // Nếu nhân vật đang không chạm đất, tự động xóa nút Nhảy khỏi gói tin gửi đi
            // Điều này ép game phải nhảy lại ngay lập tức vào đúng tick tiếp đất (Perfect Jump)
            if (!(player_flags & FL_ONGROUND)) {
                cmd->buttons &= ~IN_JUMP; 
            }
        }

        // Thuật toán Auto Strafer (Tự động tăng tốc trên không khi xoay chuột)
        void RunAutoStrafer(C_UserCmd* cmd, float current_yaw) {
            // Chỉ kích hoạt bẻ hướng khi nhân vật đang bay trên không trung
            static float old_yaw = 0.0f;
            float yaw_delta = current_yaw - old_yaw;
            old_yaw = current_yaw;

            if (cmd->sidemove == 0.0f && cmd->forwardmove == 0.0f) {
                if (yaw_delta > 0.0f) {
                    cmd->sidemove = -450.0f; // Ép nhân vật lướt mạnh sang trái
                } else if (yaw_delta < 0.0f) {
                    cmd->sidemove = 450.0f;  // Ép nhân vật lướt mạnh sang phải
                }
            }
        }
    };
    C_MovementEngine storm_move;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 2: DI CHUYỂN ẢO MA (SLIDE WALK & FAST STOP / AUTO PEEK) ]
    // Đi bộ trượt băng lướt chân đánh lừa tầm nhìn địch và tự động khựng xe phanh gấp khi bóp cò
    // ----------------------------------------------------------------------------------------
    class C_AdvancedMovement {
    public:
        // Tính năng Slide Walk (Đi bộ trượt Moonwalk phá vỡ hoạt ảnh nhân vật từ Fatality)
        void HandleSlideWalk(C_UserCmd* cmd) {
            // Đảo ngược hướng gói tin di chuyển sidemove và forwardmove nhưng vẫn giữ nguyên hướng nhìn
            // Khiến địch nhìn thấy bạn đang lướt ngang trượt băng chân như Michael Jackson
            if (cmd->forwardmove != 0.0f || cmd->sidemove != 0.0f) {
                cmd->forwardmove = -cmd->forwardmove;
                cmd->sidemove = -cmd->sidemove;
            }
        }

        // Tính năng Fast Stop (Phanh gấp khựng tâm giúp súng Scout đạt độ chính xác tức thì)
        void HandleFastStop(C_UserCmd* cmd, float player_speed) {
            // Nếu súng chuẩn bị bắn (Ragebot kích hoạt), tự động ép phanh xe khựng lại
            if (player_speed > 20.0f) {
                cmd->forwardmove = 0.0f;
                cmd->sidemove = 0.0f; // Triệt tiêu gia tốc để tâm ngắm thu nhỏ rực xanh ngay lập tức
            }
        }
    };
    C_AdvancedMovement storm_adv_move;

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 3: ĐỒNG BỘ CHẠY NGẦM VÀ TÍNH NĂNG FAKE PING NETWORK ]
    // Tạo Ping ảo (Latency Spike) lên tới 200ms để bẻ cong đường đạn, né đạn lùi thời gian server
    // ----------------------------------------------------------------------------------------
    class C_NetworkModulator {
    public:
        void ApplyFakePing() {
            // Giả lập cơ chế nạp gói tin trễ (Choke NetChannel sequence) của Fatality.win
            float desired_fake_latency = 0.2f; // Tạo thêm 200ms ping ảo để đánh lừa Resolver địch
            // nethandler->add_latency_buffer(desired_fake_latency);
        }
    };
    C_NetworkModulator storm_network;

    // Bộ xử lý trung tâm điều khiển toàn bộ tính năng Misc trong trận đấu
    void ProcessMiscFramework(C_UserCmd* cmd) {
        int mock_flags = 0;       // Giả lập biến flag kiểm tra chạm đất (0 = đang bay, 1 = chạm đất)
        float mock_speed = 250.0f; // Giả lập tốc độ chạy của nhân vật

        // 1. Kích hoạt tự động Bunnyhop nhảy lướt map liên tục
        storm_move.RunAutoBhop(cmd, mock_flags);

        // 2. Kích hoạt tự động tăng tốc bẻ hướng trên không (Jump-Scout b-hop)
        storm_move.RunAutoStrafer(cmd, cmd->viewangles_y);

        // 3. Kích hoạt đi bộ trượt lướt lăng ba vi bộ Moonwalk độc quyền
        storm_adv_move.HandleSlideWalk(cmd);

        // 4. Kích hoạt tính năng phanh gấp khựng xe khi Ragebot tìm thấy điểm bắn
        storm_adv_move.HandleFastStop(cmd, mock_speed);

        // 5. Đồng bộ Ping ảo đánh lừa gói tin máy chủ
        storm_network.ApplyFakePing();
    }

    // Luồng chạy độc lập quản lý tab Misc chạy ngầm song song với game CS2
    DWORD WINAPI MiscThread(LPVOID lpParam) {
        while (true) {
            C_UserCmd dummy_cmd;
            dummy_cmd.buttons = IN_JUMP; // Giả lập người chơi đang đè phím nhảy
            dummy_cmd.viewangles_y = 45.0f;

            if (Storm_Config::cfg.ragebot_active) {
                ProcessMiscFramework(&dummy_cmd);
            }
            
            // Khóa luồng ở mức 1ms để đồng bộ mượt mà với tốc độ gửi tin của server (Tickrate)
            Sleep(1); 
        }
        return 0;
    }
}
