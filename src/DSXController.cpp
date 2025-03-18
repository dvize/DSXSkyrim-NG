#include "DSXController.hpp"

namespace DSX {
    // Packet implementation
    void Packet::AddAdaptiveTrigger(int controllerIndex, Trigger trigger, TriggerMode mode,
                                  const std::array<int, 4>& parameters) {
        std::vector<std::string> params = {
            std::to_string(controllerIndex),
            std::to_string(static_cast<int>(trigger)),
            std::to_string(static_cast<int>(mode))
        };
        for (int param : parameters) {
            params.push_back(std::to_string(param));
        }
        instructions.emplace_back(InstructionType::TriggerUpdate, params);
    }

    void Packet::AddCustomAdaptiveTrigger(int controllerIndex, Trigger trigger, TriggerMode mode,
                                        CustomTriggerValueMode valueMode, const std::array<int, 4>& parameters) {
        std::vector<std::string> params = {
            std::to_string(controllerIndex),
            std::to_string(static_cast<int>(trigger)),
            std::to_string(static_cast<int>(mode)),
            std::to_string(static_cast<int>(valueMode))
        };
        for (int param : parameters) {
            params.push_back(std::to_string(param));
        }
        instructions.emplace_back(InstructionType::TriggerUpdate, params);
    }

    void Packet::AddRGB(int controllerIndex, int r, int g, int b, int brightness) {
        std::vector<std::string> params = {
            std::to_string(controllerIndex),
            std::to_string(r),
            std::to_string(g),
            std::to_string(b),
            std::to_string(brightness)
        };
        instructions.emplace_back(InstructionType::RGBUpdate, params);
    }

    void Packet::AddPlayerLED(int controllerIndex, PlayerLEDNewRevision led) {
        std::vector<std::string> params = {
            std::to_string(controllerIndex),
            std::to_string(static_cast<int>(led))
        };
        instructions.emplace_back(InstructionType::PlayerLEDNewRevision, params);
    }

    void Packet::AddMicLED(int controllerIndex, MicLEDMode mode) {
        std::vector<std::string> params = {
            std::to_string(controllerIndex),
            std::to_string(static_cast<int>(mode))
        };
        instructions.emplace_back(InstructionType::MicLED, params);
    }

    void Packet::AddTriggerThreshold(int controllerIndex, Trigger trigger, int threshold) {
        std::vector<std::string> params = {
            std::to_string(controllerIndex),
            std::to_string(static_cast<int>(trigger)),
            std::to_string(threshold)
        };
        instructions.emplace_back(InstructionType::TriggerThreshold, params);
    }

    // NetworkManager implementation
    NetworkManager::NetworkManager() {}

    NetworkManager::~NetworkManager() {
        if (initialized_) {
            closesocket(socket_);
            WSACleanup();
            logger::info("Network connection closed");
        }
    }

    bool NetworkManager::Initialize() {
        logger::info("Initializing DSX network connection");
        
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            logger::error("WSAStartup failed: {}", WSAGetLastError());
            return false;
        }

        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ == INVALID_SOCKET) {
            logger::error("Socket creation failed: {}", WSAGetLastError());
            WSACleanup();
            return false;
        }

        memset(&serverAddr_, 0, sizeof(serverAddr_));
        serverAddr_.sin_family = AF_INET;
        serverAddr_.sin_port = htons(PORT);
        if (inet_pton(AF_INET, SERVER_IP, &serverAddr_.sin_addr) != 1) {
            logger::error("Invalid IP address: {}", WSAGetLastError());
            closesocket(socket_);
            WSACleanup();
            return false;
        }

        initialized_ = true;
        logger::info("Network initialized successfully");
        return true;
    }

    std::string NetworkManager::PacketToJson(const Packet& packet)
    {
        json j;
        to_json(j, packet);
        return j.dump();
    }

    bool NetworkManager::SendPacket(const Packet& packet)
    {
        if (!initialized_) {
            logger::error("Network not initialized");
            return false;
        }

        std::string jsonData = PacketToJson(packet);
        if (jsonData.length() >= MAX_MESSAGE_LENGTH) {
            logger::error("Packet too large: {} bytes", jsonData.length());
            return false;
        }

        int length = static_cast<int>(jsonData.length());
        int sent = sendto(socket_, jsonData.c_str(), length, 0,
                         (sockaddr*)&serverAddr_, sizeof(serverAddr_));
        if (sent == SOCKET_ERROR) {
            logger::error("Send failed: {}", WSAGetLastError());
            return false;
        }

        logger::debug("Sent packet: {}", jsonData);
        return true;
    }

    // JSON serialization
    void to_json(json& j, const Instruction& i) {
        j = {
            {"type", static_cast<int>(i.type)},
            {"parameters", i.parameters}
        };
    }

    void to_json(json& j, const Packet& p) {
        j = {
            {"instructions", p.instructions}
        };
    }
}
