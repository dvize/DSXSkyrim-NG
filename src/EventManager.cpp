#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include "UDPHelper.h"
#include "EventManager.h"
#include "HelperFunc.h"
#include "Main.h"


using namespace SKSE;
using json = nlohmann::json;

namespace DSXSkyrim 
{
    extern DSXSkyrim::TriggersCollection userTriggers;
    extern vector<DSXSkyrim::Packet> myPackets;
    std::string actionLeft;
    std::string actionRight;
    bool PlayerCheck;
    RE::FormID theobjectid;
    RE::PlayerCharacter* player;
    bool lefthand;
    bool righthand;
    RE::FormID lefthandID = 0;
    RE::FormID righthandID = 0;
    int weaponhands;  // 0 for left hand, 1 for right hand, 2 for both hands
    RE::FormID finalID = 0;
    std::string weaponname = "";

    bool EquipStartEventHandler::RegisterEquipStartEvent() {
        static EquipStartEventHandler g_equipstarthandler;

        auto ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();

        if (!ScriptEventSource) {
            log::error("ScriptEventSource not found!");
            return false;
        }

        ScriptEventSource->AddEventSink(&g_equipstarthandler);

        log::info("Register Equip Start Event Handler!");

        return true;
    }

    RE::BSEventNotifyControl EquipStartEventHandler::ProcessEvent(
        const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {
        if (!a_event || !a_eventSource) {
            log::error("Event Source Not Found!");
            return RE::BSEventNotifyControl::kContinue;
        }

        log::info("EquipStartEventHandler Process Event function called");

        //----------------------Check if Player-----------------------------------------------

        PlayerCheck = a_event->actor->IsPlayerRef();

        theobjectid = a_event->baseObject;
        player = RE::PlayerCharacter::GetSingleton();

        if (PlayerCheck && validScenario(theobjectid)) {
            bool formidcustomweap = false;

            // check if its a custom form first before switching to generic weapontype

            for (int i = 25; i < userTriggers.TriggersList.size(); i++) {
                if (!(userTriggers.TriggersList[i].customFormID.empty())) {
                    std::string strtempformid = userTriggers.TriggersList[i].customFormID;
                    std::istringstream converter{strtempformid};
                    long long int value = 0;
                    converter >> std::hex >> value;

                    log::info(FMT_STRING("Set vars for inDec"));

                    if (theobjectid == value) {
                        log::info(FMT_STRING("Found custom weapon of FormID: {}"), value);
                        formidcustomweap = true;

                        if (userTriggers.TriggersList[i].triggerSide == 1) {
                            // left trigger assign for custom
                            actionLeft = PacketToString(myPackets.at(i));

                        } else if (userTriggers.TriggersList[i].triggerSide == 2) {
                            // right trigger assign for custom
                            actionRight = PacketToString(myPackets.at(i));
                        }
                    }
                }
            }
            // which hand was it? or both

            lefthand = player->GetEquippedObject(true);
            righthand = player->GetEquippedObject(false);

            if (lefthand) {
                lefthandID = player->GetEquippedObject(true)->GetFormID();
            }
            if (righthand) {
                righthandID = player->GetEquippedObject(false)->GetFormID();
            }

            if (a_event->equipped) {
                // Set which hand its in and the ID to use from now on
                if (lefthandID == righthandID) {
                    weaponhands = 2;  // if its two hander or two one handed dual wielded.
                    finalID = lefthandID;
                } else if (theobjectid == lefthandID) {
                    weaponhands = 0;
                    finalID = lefthandID;
                } else if (theobjectid == righthandID) {
                    weaponhands = 1;
                    finalID = righthandID;
                }

                auto equipped_type = getObjectClassType(finalID);

                /*
                    kHandToHandMelee = 0,
                    kOneHandSword = 1,
                    kOneHandDagger = 2,
                    kOneHandAxe = 3,
                    kOneHandMace = 4,
                    kTwoHandSword = 5,
                    kTwoHandAxe = 6,
                    kBow = 7,
                    kStaff = 8,
                    kCrossbow = 9
                    kShield = 10
                    kMagic = 11
                    kTorch = 12
                */
                log::debug(
                    "After unequip the case statement was reached");  // there is an issue if equipping runs the unequip
                                                                      // and then equip directly (does not hit)

                if (!formidcustomweap) {
                    switch (equipped_type) {
                        case 0:
                            // call the class trigger setting for hand melee and then use weaponhands to determine which
                            // trigger it is for.
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(0));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(1));
                            } else {
                                actionLeft = PacketToString(myPackets.at(0));
                                actionRight = PacketToString(myPackets.at(1));
                            }
                            weaponname = "HandToHandMelee";
                            break;
                        case 1:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(2));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(3));
                            } else {
                                actionLeft = PacketToString(myPackets.at(2));
                                actionRight = PacketToString(myPackets.at(3));
                            }
                            weaponname = "OneHandSword";
                            break;
                        case 2:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(4));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(5));
                            } else {
                                actionLeft = PacketToString(myPackets.at(4));
                                actionRight = PacketToString(myPackets.at(5));
                            }
                            weaponname = "OneHandDagger";
                            break;
                        case 3:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(6));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(7));
                            } else {
                                actionLeft = PacketToString(myPackets.at(6));
                                actionRight = PacketToString(myPackets.at(7));
                            }
                            weaponname = "OneHandAxe";
                            break;
                        case 4:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(8));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(9));
                            } else {
                                actionLeft = PacketToString(myPackets.at(8));
                                actionRight = PacketToString(myPackets.at(9));
                            }
                            weaponname = "OneHandMace";
                            break;
                        case 5:
                            if (weaponhands == 2) {
                                actionLeft = PacketToString(myPackets.at(17));   // Two Hand Sword Block
                                actionRight = PacketToString(myPackets.at(18));  // Two Hand Sword
                            }
                            weaponname = "TwoHandSword";
                            break;
                        case 6:
                            if (weaponhands == 2) {
                                actionLeft = PacketToString(myPackets.at(19));   // Two Hand Axe Mace Block
                                actionRight = PacketToString(myPackets.at(20));  // Two Hand Axe Mace
                            }
                            weaponname = "TwoHandAxeMace";
                            break;
                        case 7:
                            if (weaponhands == 2) {
                                actionLeft = PacketToString(myPackets.at(21));   // Bow Bash Setting
                                actionRight = PacketToString(myPackets.at(22));  // Bow Draw Setting
                            }
                            weaponname = "Bow";
                            break;
                        case 8:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(10));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(11));
                            } else {
                                actionLeft = PacketToString(myPackets.at(10));
                                actionRight = PacketToString(myPackets.at(11));
                            }
                            weaponname = "Staff";
                            break;
                        case 9:
                            if (weaponhands == 2) {
                                actionLeft = PacketToString(myPackets.at(23));   // Left Trigger (aiming) crossbow
                                actionRight = PacketToString(myPackets.at(24));  // Right Trigger Crossbow
                            }
                            weaponname = "Crossbow";
                            break;
                        case 10:
                            actionLeft = PacketToString(myPackets.at(14));  // always left hand.
                            weaponname = "Shield";
                            break;
                        case 11:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(12));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(13));
                            } else {
                                actionLeft = PacketToString(myPackets.at(12));
                                actionRight = PacketToString(myPackets.at(13));
                            }
                            weaponname = "Magic";
                            break;
                        case 12:
                            if (weaponhands == 0) {
                                actionLeft = PacketToString(myPackets.at(15));
                            } else if (weaponhands == 1) {
                                actionRight = PacketToString(myPackets.at(16));
                            } else {
                                actionLeft = PacketToString(myPackets.at(15));
                                actionRight = PacketToString(myPackets.at(16));
                            }
                            weaponname = "Torch";
                            break;

                        default:
                            weaponname = "Not a Weapon";
                            break;
                    }
                }

                if (actionLeft.empty() && actionRight.empty()) {
                    return RE::BSEventNotifyControl::kContinue;
                } else if (!actionLeft.empty() && actionRight.empty()) {
                    sendToDSX(actionLeft);
                } else if (actionLeft.empty() && !actionRight.empty()) {
                    sendToDSX(actionRight);

                } else if (!actionLeft.empty() && !actionRight.empty()) {
                    sendToDSX(actionLeft);
                    sendToDSX(actionRight);
                }

                log::debug("Player Equip Detected of Weapon: {0} and using WeaponHands of {1}", weaponname,
                           weaponhands);

            } else {
                // what hand was unequipped.. reset controller trigger for the any hands that are null;
                bool lefthand;
                bool righthand;

                lefthand = player->GetEquippedObject(true);
                righthand = player->GetEquippedObject(false);

                if (!lefthand && !righthand) {
                    // reset both triggers
                    actionLeft = PacketToString(myPackets.at(0));
                    actionRight = PacketToString(myPackets.at(1));
                    sendToDSX(actionLeft);
                    sendToDSX(actionRight);

                    log::debug("Player UnEquip Detected. Reset Both LeftHand and RightHand Triggers.");

                } else if (!lefthand && righthand) {
                    log::debug("Player UnEquip Detected. Reset Left Hand triggers only.");
                    actionRight = PacketToString(myPackets.at(0));
                    sendToDSX(actionRight);

                } else if (lefthand && !righthand) {
                    log::debug("Player UnEquip Detected. Reset Right Hand triggers only.");
                    actionLeft = PacketToString(myPackets.at(1));
                    sendToDSX(actionLeft);
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    

    void to_json(json& j, const Instruction& p) { j = {{"type", p.type}, {"parameters", p.parameters}}; }

    void from_json(const json& j, Instruction& p) {
        j.at("type").get<int>();
        j.at("parameters").get<std::string>();
    }

    void to_json(json& j, const Packet& p) {
        j = {{"instructions", p.instructions}

        };
    }

    void from_json(const json& j, Packet& p) { j.at("instructions").get_to(p.instructions); }

    void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace) {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    std::string PacketToString(DSXSkyrim::Packet& packet) {
        log::info("PacketToString function called");

        json j = packet;
        std::string s = j.dump();

        ReplaceStringInPlace(s, "\"", "");                            // removed from all to clear ints
        ReplaceStringInPlace(s, "instructions", "\"instructions\"");  // add back for instructions
        ReplaceStringInPlace(s, "type", "\"type\"");                  // add back for type
        ReplaceStringInPlace(s, "parameters", "\"parameters\"");

        return s;  // what is packet data structure with wierd names?
    }

    DSXSkyrim::Packet StringToPacket(json j) {
        log::info("StringToPacket function called");
        return j.get<DSXSkyrim::Packet>();
    }

    
}

