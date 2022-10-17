#pragma once

using namespace std;


namespace DSXSkyrim {
    
    class TriggerSetting {
    public:
        std::string name;
        std::string customFormID;
        std::string category;
        std::string description;
        int triggerSide;
        int triggerType;
        int customTriggerMode;
        int playerLEDNewRev;
        int micLEDMode;
        int triggerThresh;
        int controllerIndex;
        vector<int> triggerParams;
        vector<int> rgbUpdate;
        vector<bool> playerLED;

        TriggerSetting() {
            controllerIndex = 0;
            name = "Default";
            customFormID = "";
            category = "One-Handed Bow";
            description = "Drawing Hand";
            triggerSide = 2;
            triggerType = 14;
            customTriggerMode = 0;
            triggerParams = {0, 8, 2, 5};
            rgbUpdate = {0, 0, 0};
            playerLED = {false, false, false, false, false};
            playerLEDNewRev = 5;
            micLEDMode = 2;
            triggerThresh = 0;
        }
    };

    struct Instruction {
        int type;
        vector<std::string> parameters;
    };

    class Packet {
    public:
        Instruction instructions[6];

        Packet() {
            instructions[0].type = 1;
            instructions[0].parameters = {};

            instructions[2].type = 2;
            instructions[2].parameters = {};

            instructions[3].type = 3;
            instructions[3].parameters = {};

            instructions[1].type = 4;
            instructions[1].parameters = {};

            instructions[5].type = 5;
            instructions[5].parameters = {};

            instructions[4].type = 6;
            instructions[4].parameters = {};
        }
    };

    class TriggersCollection {
    public:
        vector<TriggerSetting> TriggersList;
    };

    class EquipStartEventHandler : public RE::BSTEventSink<RE::TESEquipEvent> {
    public:
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource);
        static bool RegisterEquipStartEvent();

    private:
        EquipStartEventHandler() = default;

        ~EquipStartEventHandler() = default;

        EquipStartEventHandler(const EquipStartEventHandler&) = delete;

        EquipStartEventHandler(EquipStartEventHandler&&) = delete;

        EquipStartEventHandler& operator=(const EquipStartEventHandler&) = delete;

        EquipStartEventHandler& operator=(EquipStartEventHandler&&) = delete;
    };
}

