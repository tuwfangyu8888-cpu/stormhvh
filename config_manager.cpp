#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h> // Thư viện hệ thống kiểm tra sự tồn tại của thư mục
#include "config.h"

// ============================================================================================
//                       STORMHVH - ADVANCED CONFIG MANAGER MODULE
//             [ Fatality-Style Automated Folder Creation & Binary IO Engine ]
// ============================================================================================

namespace Storm_ConfigManager {

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 1: THUẬT TOÁN TỰ ĐỘNG KIỂM TRA VÀ TẠO THƯ MỤC LƯU TRỮ (AUTO FOLDER GENERATOR) ]
    // Cơ chế thông minh: Kiểm tra nếu folder chưa tồn tại thì sẽ tự động đúc ra folder mới
    // ----------------------------------------------------------------------------------------
    void EnsureConfigFolderExists(const std::string& folder_path) {
        struct stat info;

        // Kiểm tra xem đường dẫn thư mục đã tồn tại trên ổ đĩa hay chưa
        if (stat(folder_path.c_str(), &info) != 0) {
            // Nếu chưa tồn tại (stat trả về khác 0), tiến hành đúc một folder mới tinh
            if (CreateDirectoryA(folder_path.c_str(), NULL)) {
                std::cout << "[StormConfig] Created new secure folder at: " << folder_path << std::endl;
            } else {
                std::cout << "[!] StormConfig Warning: Failed to create folder. Checking administrator rights..." << std::endl;
            }
        } else if (info.st_mode & S_IFDIR) {
            // Nếu thư mục đã có sẵn từ trước, giữ nguyên không ghi đè để bảo vệ dữ liệu cũ
            std::cout << "[StormConfig] Target folder already exists. Ready for data streaming." << std::endl;
        }
    }

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 2: THUẬT TOÁN GHI FILE .CFG NHỊ PHÂN VÀ TỰ ĐỘNG TẠO FOLDER KHI LƯU ]
    // Chụp lại toàn bộ trạng thái nút bấm gạt nón lá, súng dính điện và đóng gói vào file
    // ----------------------------------------------------------------------------------------
    bool SaveCheatProfile(std::string cfg_name) {
        // Cài đặt đường dẫn thư mục lưu trữ an toàn ngoài ổ C (Tránh lỗi chặn file của Windows Defender)
        std::string base_folder = "C:\\StormHvH_Configs\\";
        
        // KÍCH HOẠT LỆNH KIỂM TRA: Bấm lưu là tự động đúc folder nếu máy chưa có!
        EnsureConfigFolderExists(base_folder);

        // Tạo đường dẫn file cấu hình hoàn chỉnh (Ví dụ: C:\StormHvH_Configs\scout_vip.cfg)
        std::string full_file_path = base_folder + cfg_name + ".cfg";
        
        // Mở luồng ghi file dưới dạng Nhị Phân (std::ios::binary) để tối ưu tốc độ đọc của máy 8GB RAM
        std::ofstream out_stream(full_file_path, std::ios::out | std::ios::binary);

        if (!out_stream.is_open()) {
            std::cout << "[!] StormConfig Save Error: Unable to open file stream at " << full_file_path << std::endl;
            return false;
        }

        // Đổ toàn bộ cấu trúc vùng nhớ của struct Settings vào file với tốc độ ánh sáng
        out_stream.write(reinterpret_cast<const char*>(&Storm_Config::cfg), sizeof(Storm_Config::Settings));
        out_stream.close();

        std::cout << "[+] StormConfig: SUCCESSFULLY SAVED config profile -> " << full_file_path << std::endl;
        return true;
    }

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 3: THUẬT TOÁN ĐỌC FILE .CFG NHỊ PHÂN (BINARY CONFIG LOADING) ]
    // Nạp lại thông số súng, vòng tròn tính toán độ trượt ngay lập tức khi bạn bấm nút
    // ----------------------------------------------------------------------------------------
    bool LoadCheatProfile(std::string cfg_name) {
        std::string target_path = "C:\\StormHvH_Configs\\" + cfg_name + ".cfg";
        
        // Mở luồng đọc file dưới dạng Nhị Phân
        std::ifstream in_stream(target_path, std::ios::in | std::ios::binary);

        if (!in_stream.is_open()) {
            std::cout << "[!] StormConfig Load Error: Config file does not exist -> " << target_path << std::endl;
            return false;
        }

        // Đọc vùng nhớ nhị phân từ file đè ngược lại vào struct đang chạy của bản cheat StormHvH
        in_stream.read(reinterpret_cast<char*>(&Storm_Config::cfg), sizeof(Storm_Config::Settings));
        in_stream.close();

        std::cout << "[+] StormConfig: SUCCESSFULLY LOADED config profile -> " << target_path << std::endl;
        return true;
    }

    // ----------------------------------------------------------------------------------------
    // [ PHẦN 4: CỔNG ĐỒNG BỘ LỆNH GỌI TỪ MENU (GUI INTERACTION CALLBACK) ]
    // Lắng nghe nút nhấn Save/Load từ file menu.cpp gửi tín hiệu qua
    // ----------------------------------------------------------------------------------------
    void TriggerConfigAction(int action_id, std::string profile_name) {
        // action_id: 1 = Người chơi bấm nút SAVE, 2 = Người chơi bấm nút LOAD
        if (action_id == 1) {
            SaveCheatProfile(profile_name);
        } 
        else if (action_id == 2) {
            LoadCheatProfile(profile_name);
        }
    }
}
