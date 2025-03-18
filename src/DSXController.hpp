#pragma once

#include <Winsock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#pragma comment(lib, "Ws2_32.lib")

namespace DSX {
    using json = nlohmann::json;

    enum class TriggerMode {
        Normal = 0,
        GameCube = 1,
        VerySoft = 2,
        Soft = 3,
        Hard = 4,
        VeryHard = 5,
        Hardest = 6,
        Rigid = 7,
        VibrateTrigger = 8,
        Choppy = 9,
        Medium = 10,
        VibrateTriggerPulse = 11,
        CustomTriggerValue = 12,
        Resistance = 13,
        Bow = 14,
        Galloping = 15,
        SemiAutomaticGun = 16,
        AutomaticGun = 17,
        Machine = 18
    };

    enum class CustomTriggerValueMode {
        OFF = 0,
        Rigid = 1,
        RigidA = 2,
        RigidB = 3,
        RigidAB = 4,
        Pulse = 5,
        PulseA = 6,
        PulseB = 7,
        PulseAB = 8
    };

    enum class Trigger {
        Invalid = 0,
        Left = 1,
        Right = 2
    };

    enum class InstructionType {
        TriggerUpdate = 1,
        RGBUpdate = 2,
        PlayerLEDNewRevision = 6,
        MicLED = 5,
        TriggerThreshold = 4,
        ResetToUserSettings = 7,
        GetDSXStatus = 0
    };

    enum class PlayerLEDNewRevision {
        One = 0,
        Two = 1,
        Three = 2,
        Four = 3,
        Five = 4,
        AllOff = 5
    };

    enum class MicLEDMode {
        On = 0,
        Pulse = 1,
        Off = 2
    };

    // Constants
    constexpr const char* SERVER_IP = "127.0.0.1";
    constexpr int PORT = 6969;
    constexpr int BUFLEN = 512;

    struct Instruction {
        InstructionType type;
        std::vector<std::string> parameters;

        Instruction(InstructionType _type = InstructionType::TriggerUpdate, 
                   const std::vector<std::string>& _params = {}) 
            : type(_type), parameters(_params) {}
    };

    class Packet {
    public:
        std::vector<Instruction> instructions;

        void AddAdaptiveTrigger(int controllerIndex, Trigger trigger, TriggerMode mode, 
                              const std::array<int, 4>& parameters);
        void AddCustomAdaptiveTrigger(int controllerIndex, Trigger trigger, TriggerMode mode,
                                    CustomTriggerValueMode valueMode, const std::array<int, 4>& parameters);
        void AddRGB(int controllerIndex, int r, int g, int b, int brightness = 255);
        void AddPlayerLED(int controllerIndex, PlayerLEDNewRevision led);
        void AddMicLED(int controllerIndex, MicLEDMode mode);
        void AddTriggerThreshold(int controllerIndex, Trigger trigger, int threshold);
    };

    struct TriggerSetting {
        int controllerIndex = 0;
        std::string name = "Default";
        std::string formID;  // CustomFormID in Skyrim
        std::string category = "Default";
        int triggerSide = static_cast<int>(Trigger::Right);
        int triggerType = static_cast<int>(TriggerMode::Normal);
        int customTriggerMode = static_cast<int>(CustomTriggerValueMode::OFF);
        std::array<int, 4> triggerParams = {0, 0, 0, 0};  // Skyrim uses up to 4 params
        std::array<int, 3> rgbUpdate = {0, 0, 0};
        int playerLEDNewRev = static_cast<int>(PlayerLEDNewRevision::One);
        int micLEDMode = static_cast<int>(MicLEDMode::Off);
        int triggerThresh = 0;
    };

    class NetworkManager {
    public:
        NetworkManager();
        ~NetworkManager();
        
        bool Initialize();
        bool SendPacket(const Packet& packet);
        
    private:
        static constexpr int MAX_MESSAGE_LENGTH = 4096;
        bool initialized_ = false;
        SOCKET socket_ = INVALID_SOCKET;
        sockaddr_in serverAddr_;

        std::string PacketToJson(const Packet& packet);
    };

    // JSON serialization
    void to_json(json& j, const Instruction& i);
    void to_json(json& j, const Packet& p);
}
