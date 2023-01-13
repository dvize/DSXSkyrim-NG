#include <stddef.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <format>
#include <thread>
#include <chrono>
#include <WinSock2.h>
#include "Main.h"
#include "PCH.h"
#include "UDPHelper.h"
#include "EventManager.h"

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

using nlohmann::json;
using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::stl;

namespace DSXSkyrim
{
    using socket_t = decltype(socket(0, 0, 0));

    socket_t mysocket;
    sockaddr_in server;
    TriggersCollection userTriggers;
    vector<Packet> myPackets;

    extern std::string actionLeft;
    extern std::string actionRight;

    void InitializeMessaging() {
        if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
            switch (message->type) {
                case MessagingInterface::kDataLoaded: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
                    // It is now safe to access form data.
                    log::info("Data Load CallBack Trigger!");

                    if (EquipStartEventHandler::RegisterEquipStartEvent())
                        log::info("Register Equip Event!");
                    break;

                // Skyrim game events.
                case MessagingInterface::kPostLoadGame: // Player's selected save game has finished loading.
                    // Data will be a boolean indicating whether the load was successful.
                    log::info("Post Load Game CallBack Trigger!");

                    if (EquipStartEventHandler::RegisterEquipStartEvent())
                        log::info("Register Equip Event!");
                    break;
            }
        })) {
            report_and_fail("Unable to register message listener.");
        }
    }

    json to_json(json& j, const TriggerSetting& p) {
        j = {{"Name", p.name},
             {"CustomFormID", p.customFormID},
             {"Category", p.category},
             {"Description", p.description},
             {"TriggerSide", p.triggerSide},
             {"TriggerType", p.triggerType},
             {"customTriggerMode", p.customTriggerMode},
             {"playerLEDNewRev", p.playerLEDNewRev},
             {"MicLEDMode", p.micLEDMode},
             {"TriggerThreshold", p.triggerThresh},
             {"ControllerIndex", p.controllerIndex},
             {"TriggerParams", p.triggerParams},
             {"RGBUpdate", p.rgbUpdate},
             {"PlayerLED", p.playerLED}};

        return j;
    }

    void from_json(const json& j, TriggerSetting& p) {
        j.at("Name").get_to(p.name);
        j.at("CustomFormID").get_to(p.customFormID);
        j.at("Category").get_to(p.category);
        j.at("Description").get_to(p.description);
        j.at("TriggerSide").get_to(p.triggerSide);
        j.at("TriggerType").get_to(p.triggerType);
        j.at("customTriggerMode").get_to(p.customTriggerMode);
        j.at("playerLEDNewRev").get_to(p.playerLEDNewRev);
        j.at("MicLEDMode").get_to(p.micLEDMode);
        j.at("TriggerThreshold").get_to(p.triggerThresh);
        j.at("ControllerIndex").get_to(p.controllerIndex);
        j.at("TriggerParams").get_to(p.triggerParams);
        j.at("RGBUpdate").get_to(p.rgbUpdate);
        j.at("PlayerLED").get_to(p.playerLED);
    }

    void readFromConfig() {
        try {
            json j;
            std::ifstream stream(".\\Data\\SKSE\\Plugins\\DSXSkyrim\\DSXSkyrimConfig.json");
            stream >> j;
            log::info("JSON File Read from location");

            log::info("Try assign j");

            TriggerSetting conversion;

            for (int i = 0; i < j.size(); i++) {
                conversion = j.at(i).get<TriggerSetting>();
                userTriggers.TriggersList.push_back(conversion);
            }

            log::info("Breakpoint here to check value of userTriggers");

        } catch (exception e) {
            throw e;
        }
    }

    void setInstructionParameters(TriggerSetting& TempTrigger, Instruction& TempInstruction) {
        switch (TempInstruction.type) {
            case 1:  // TriggerUpdate
                switch (TempTrigger.triggerParams.size()) {
                    case 0:
                        TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                      std::to_string(TempTrigger.triggerSide),
                                                      std::to_string(TempTrigger.triggerType)};
                        break;

                    case 1:
                        TempInstruction.parameters = {
                            std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide),
                            std::to_string(TempTrigger.triggerType), std::to_string(TempTrigger.triggerParams.at(0))};
                        break;

                    case 2:
                        TempInstruction.parameters = {
                            std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.triggerSide),
                            std::to_string(TempTrigger.triggerType), std::to_string(TempTrigger.triggerParams.at(0)),
                            std::to_string(TempTrigger.triggerParams.at(1))};
                        break;

                    case 3:
                        TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                      std::to_string(TempTrigger.triggerSide),
                                                      std::to_string(TempTrigger.triggerType),
                                                      std::to_string(TempTrigger.triggerParams.at(0)),
                                                      std::to_string(TempTrigger.triggerParams.at(1)),
                                                      std::to_string(TempTrigger.triggerParams.at(2))};
                        break;

                    case 4:
                        TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                      std::to_string(TempTrigger.triggerSide),
                                                      std::to_string(TempTrigger.triggerType),
                                                      std::to_string(TempTrigger.triggerParams.at(0)),
                                                      std::to_string(TempTrigger.triggerParams.at(1)),
                                                      std::to_string(TempTrigger.triggerParams.at(2)),
                                                      std::to_string(TempTrigger.triggerParams.at(3))};
                        break;

                    case 5:
                        TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                      std::to_string(TempTrigger.triggerSide),
                                                      std::to_string(TempTrigger.triggerType),
                                                      std::to_string(TempTrigger.triggerParams.at(0)),
                                                      std::to_string(TempTrigger.triggerParams.at(1)),
                                                      std::to_string(TempTrigger.triggerParams.at(2)),
                                                      std::to_string(TempTrigger.triggerParams.at(3)),
                                                      std::to_string(TempTrigger.triggerParams.at(4))};
                        break;

                    case 6:
                        TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                      std::to_string(TempTrigger.triggerSide),
                                                      std::to_string(TempTrigger.triggerType),
                                                      std::to_string(TempTrigger.triggerParams.at(0)),
                                                      std::to_string(TempTrigger.triggerParams.at(1)),
                                                      std::to_string(TempTrigger.triggerParams.at(2)),
                                                      std::to_string(TempTrigger.triggerParams.at(3)),
                                                      std::to_string(TempTrigger.triggerParams.at(4)),
                                                      std::to_string(TempTrigger.triggerParams.at(5))};
                        break;

                    case 7:
                        if (TempTrigger.triggerType == 12) {
                            TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                          std::to_string(TempTrigger.triggerSide),
                                                          std::to_string(TempTrigger.triggerType),
                                                          std::to_string(TempTrigger.customTriggerMode),
                                                          std::to_string(TempTrigger.triggerParams.at(0)),
                                                          std::to_string(TempTrigger.triggerParams.at(1)),
                                                          std::to_string(TempTrigger.triggerParams.at(2)),
                                                          std::to_string(TempTrigger.triggerParams.at(3)),
                                                          std::to_string(TempTrigger.triggerParams.at(4)),
                                                          std::to_string(TempTrigger.triggerParams.at(5)),
                                                          std::to_string(TempTrigger.triggerParams.at(6))};
                            break;
                        } else {
                            TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                          std::to_string(TempTrigger.triggerSide),
                                                          std::to_string(TempTrigger.triggerType),
                                                          std::to_string(TempTrigger.triggerParams.at(0)),
                                                          std::to_string(TempTrigger.triggerParams.at(1)),
                                                          std::to_string(TempTrigger.triggerParams.at(2)),
                                                          std::to_string(TempTrigger.triggerParams.at(3)),
                                                          std::to_string(TempTrigger.triggerParams.at(4)),
                                                          std::to_string(TempTrigger.triggerParams.at(5)),
                                                          std::to_string(TempTrigger.triggerParams.at(6))};
                            break;
                        }

                    default:
                        TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                                      std::to_string(TempTrigger.triggerSide),
                                                      std::to_string(TempTrigger.triggerType)};
                        break;
                }
                break;

            case 2:  // RGBUpdate
                TempInstruction.parameters = {
                    std::to_string(TempTrigger.controllerIndex), std::to_string(TempTrigger.rgbUpdate.at(0)),
                    std::to_string(TempTrigger.rgbUpdate.at(1)), std::to_string(TempTrigger.rgbUpdate.at(2))};
                break;

            case 3:  // PlayerLED    --- parameters is set to vector<int> so the bool is not coming across. need to fix
                TempInstruction.parameters = {
                    std::to_string(TempTrigger.controllerIndex), "false", "false", "false", "false", "false"};
                break;

            case 4:  // TriggerThreshold
                TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                              std::to_string(TempTrigger.triggerSide),
                                              std::to_string(TempTrigger.triggerThresh)};
                break;

            case 5:  // InstructionType.MicLED
                TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                              std::to_string(TempTrigger.micLEDMode)};
                break;

            case 6:  // InstructionType.PlayerLEDNewRevision
                TempInstruction.parameters = {std::to_string(TempTrigger.controllerIndex),
                                              std::to_string(TempTrigger.playerLEDNewRev)};
                break;
        }
    }

    void generatePacketInfo(TriggersCollection& userTriggers, vector<Packet>& myPackets) {
        for (int i = 0; i < userTriggers.TriggersList.size(); i++) {
            Packet TempPacket;
            myPackets.push_back(TempPacket);

            myPackets.at(i).instructions[0].type = 1;  // InstructionType.TriggerUpdate
            setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[0]);

            myPackets.at(i).instructions[2].type = 2;  // InstructionType.RGBUpdate
            setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[2]);

            myPackets.at(i).instructions[3].type =
                3;  // InstructionType.PlayerLED - fk this contains bools and mixed int
            setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[3]);

            myPackets.at(i).instructions[1].type = 4;  // InstructionType.TriggerThreshold
            setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[1]);

            myPackets.at(i).instructions[5].type = 5;  // InstructionType.MicLED
            setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[5]);

            myPackets.at(i).instructions[4].type = 6;  // InstructionType.PlayerLEDNewRevision
            setInstructionParameters(userTriggers.TriggersList.at(i), myPackets.at(i).instructions[4]);
        }
    }

    void background(std::chrono::milliseconds interval) {
        while (1) {
            if (!actionLeft.empty()) {
                sendToDSX(actionLeft);
            }
            if (!actionRight.empty()) {
                sendToDSX(actionRight);
            }
            std::this_thread::sleep_for(interval);
        }
    }


SKSEPluginLoad(const LoadInterface* skse) {

    #ifndef NDEBUG
        while (!IsDebuggerPresent()) {};
    #endif

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    log::info("Loading JSON Files");
    DSXSkyrim::readFromConfig();

    log::info("Generate Packet Vector from Config");
    DSXSkyrim::generatePacketInfo(DSXSkyrim::userTriggers, DSXSkyrim::myPackets);

    log::info("Startup UDP Function");
    DSXSkyrim::StartupUDP();

    Init(skse);
    DSXSkyrim::InitializeMessaging();

    log::info("Running UDP Sender Loop Event");
    auto interval = std::chrono::milliseconds(10000);

    std::thread background_worker(&DSXSkyrim::background, interval);
    background_worker.detach();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}

}
