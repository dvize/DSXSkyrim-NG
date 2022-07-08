#include "HelperFunc.h"
//#include "Main.h"
#include "Events.h"
#include <string>
#include <nlohmann/json.hpp>

namespace DSXSkyrim
{
	
	using json = nlohmann::json;
	using socket_t = decltype(socket(0, 0, 0));

	extern DSXSkyrim::TriggersCollection userTriggers;
	extern socket_t mysocket;
	extern sockaddr_in server;
	extern vector<Packet> myPackets;
	bool nextequip = false;
	bool firstequip = true;
	string action1;
	string action2;
	
	bool EquipStartEventHandler::RegisterEquipStartEvent()
	{
		static EquipStartEventHandler g_equipstarthandler;

		auto ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();

		if (!ScriptEventSource) {
			logger::error("ScriptEventSource not found!");
			return false;
		}

		ScriptEventSource->AddEventSink(&g_equipstarthandler);

		logger::info("Register Equip Start Event Handler!");

		return true;
	}


	void sendToDSX(string &s)
	{
		//convert json to string and then char array

		char message[512];
		strcpy(message, s.c_str());
		for (int i = s.size(); i < 512; i++) {
			message[i] = '\0';
		}
		
		// send the message
		
		if (sendto(mysocket, message, strlen(message), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR) {
			logger::error("sendtodsx() failed with error code: %d", WSAGetLastError());
		}

		//lastSend1 = s;
	}


	void to_json(json& j, const Instruction& p)
	{
		j = {
			{ "type", p.type },
			{ "parameters", p.parameters }
		};
	}

	void from_json(const json& j, Instruction& p)
	{
		j.at("type").get<int>();
		j.at("parameters").get<std::string>();
	}

	
	void to_json(json& j, const Packet& p)
	{
		j = {
			{ "instructions", p.instructions}

		};
	}

	void from_json(const json& j, Packet& p)
	{
		j.at("instructions").get_to(p.instructions);
		
	}

	void ReplaceStringInPlace(std::string& subject, const std::string& search,
		const std::string& replace)
	{
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	string PacketToString(Packet& packet)
	{
		logger::info("PacketToString function called");

		json j = packet;
		std::string s = j.dump();
		
		ReplaceStringInPlace(s, "\"", ""); //removed from all to clear ints
		ReplaceStringInPlace(s, "instructions", "\"instructions\"");  // add back for instructions
		ReplaceStringInPlace(s, "type", "\"type\"");  // add back for type
		ReplaceStringInPlace(s, "parameters", "\"parameters\"");
		
		
		return s;  //what is packet data structure with wierd names?
	}

	Packet StringToPacket(json j)
	{
		logger::info("StringToPacket function called");
		auto conversion = j.get<DSXSkyrim::Packet>();
		return conversion;
	}

	RE::BSEventNotifyControl EquipStartEventHandler::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource)
	{
		if (!a_event || !a_eventSource) {
			logger::error("Event Source Not Found!");
			return RE::BSEventNotifyControl::kContinue;
		}

		logger::info("EquipStartEventHandler Process Event function called");


		//----------------------Check if Player-----------------------------------------------

		auto PlayerCheck = a_event->actor->IsPlayerRef();

		auto theobjectid = a_event->baseObject;
		auto player = RE::PlayerCharacter::GetSingleton();
		


		if (PlayerCheck && validScenario(theobjectid))
		{
			bool lefthand;
			bool righthand;
			RE::FormID lefthandID = 0;
			RE::FormID righthandID = 0;
			int weaponhands;  // 0 for left hand, 1 for right hand, 2 for both hands
			RE::FormID finalID = 0;
			std::string weaponname = "";

			//which hand was it? or both

			lefthand = player->GetEquippedObject(true);
			righthand = player->GetEquippedObject(false);

			if (lefthand) {
				lefthandID = player->GetEquippedObject(true)->GetFormID();
			}
			if (righthand) {
				righthandID = player->GetEquippedObject(false)->GetFormID();
			}

			if (a_event->equipped) 
			{
				if (!firstequip) {
					nextequip = true;
				}
				firstequip = false;

				// Set which hand its in and the ID to use from now on
				if (lefthandID == righthandID) {
					weaponhands = 2; //if its two hander or two one handed dual wielded.
					finalID = lefthandID;
				} else if ( theobjectid == lefthandID) {
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
				logger::debug("After unequip the case statement was reached"); //there is an issue if equipping runs the unequip and then equip directly (does not hit)

				switch (equipped_type) {
				case 0:
					//call the class trigger setting for hand melee and then use weaponhands to determine which trigger it is for.
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(0));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(1));
					} else {
						action1 = PacketToString(myPackets.at(0));
						action2 = PacketToString(myPackets.at(1));
					}
					weaponname = "HandToHandMelee";
					break;
				case 1:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(2));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(3));
					} else {
						action1 = PacketToString(myPackets.at(2));
						action2 = PacketToString(myPackets.at(3));
					}
					weaponname = "OneHandSword";
					break;
				case 2:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(4));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(5));
					} else {
						action1 = PacketToString(myPackets.at(4));
						action2 = PacketToString(myPackets.at(5));
					}
					weaponname = "OneHandDagger";
					break;
				case 3:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(6));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(7));
					} else {
						action1 = PacketToString(myPackets.at(6));
						action2 = PacketToString(myPackets.at(7));
					}
					weaponname = "OneHandAxe";
					break;
				case 4:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(8));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(9));
					} else {
						action1 = PacketToString(myPackets.at(8));
						action2 = PacketToString(myPackets.at(9));
					}
					weaponname = "OneHandMace";
					break;
				case 5:
					if (weaponhands == 2) {
						action1 = PacketToString(myPackets.at(17));  //Two Hand Sword Block
						action2 = PacketToString(myPackets.at(18));  //Two Hand Sword 
					}
					weaponname = "TwoHandSword";
					break;
				case 6:
					if(weaponhands == 2)
					{
						action1 = PacketToString(myPackets.at(19));  //Two Hand Axe Mace Block
						action2 = PacketToString(myPackets.at(20));  //Two Hand Axe Mace
					}
					weaponname = "TwoHandAxeMace";
					break;
				case 7:
					if (weaponhands == 2) {
						action1 = PacketToString(myPackets.at(21));  //Bow Bash Setting
						action2 = PacketToString(myPackets.at(22));  //Bow Draw Setting
					}
					weaponname = "Bow";
					break;
				case 8:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(10));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(11));
					} else {
						action1 = PacketToString(myPackets.at(10));
						action2 = PacketToString(myPackets.at(11));
					}
					weaponname = "Staff";
					break;
				case 9:
					if (weaponhands == 2) {
						action1 = PacketToString(myPackets.at(23));  //Left Trigger (aiming) crossbow
						action2 = PacketToString(myPackets.at(24));  //Right Trigger Crossbow
					}
					weaponname = "Crossbow";
					break;
				case 10:
					action1 = PacketToString(myPackets.at(14));  //always left hand.
					weaponname = "Shield";
					break;
				case 11:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(12));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(13));
					} else {
						action1 = PacketToString(myPackets.at(12));
						action2 = PacketToString(myPackets.at(13));
					}
					weaponname = "Magic";
					break;
				case 12:
					if (weaponhands == 0) {
						action1 = PacketToString(myPackets.at(15));
					} else if (weaponhands == 1) {
						action1 = PacketToString(myPackets.at(16));
					} else {
						action1 = PacketToString(myPackets.at(15));
						action2 = PacketToString(myPackets.at(16));
					}
					weaponname = "Torch";
					break;

				default:
					weaponname = "Not a Weapon";
					break;
				}


				if (action1.empty() && action2.empty()) {
					return RE::BSEventNotifyControl::kContinue;
				} else if (!action1.empty() && action2.empty()) {
					sendToDSX(action1);
				} else if (action1.empty() && !action2.empty()) {
					sendToDSX(action2);

				} else if (!action1.empty() && !action2.empty()) {
					sendToDSX(action1);
					sendToDSX(action2);
				}
				
				logger::debug("Player Equip Detected of Weapon: {0} and using WeaponHands of {1}", weaponname, weaponhands);

			} 
			else 
			{
				//what hand was unequipped.. reset controller trigger for the any hands that are null;
				bool lefthand;
				bool righthand;

				lefthand = player->GetEquippedObject(true);
				righthand = player->GetEquippedObject(false);

				if (!lefthand && !righthand) {
					//reset both triggers
					action1 = PacketToString(myPackets.at(0));
					sendToDSX(action1);
					action2 = PacketToString(myPackets.at(1));
					sendToDSX(action2);

					logger::debug("Player UnEquip Detected. Reset Both LeftHand and RightHand Triggers.");


				} else if (!lefthand && righthand) {
					//reset left trigger only
					logger::debug("Player UnEquip Detected. Reset LeftHand triggers only.");
					action1 = PacketToString(myPackets.at(0));
					sendToDSX(action1);

				} else if (lefthand && !righthand) {
					//reset right trigger only
					logger::debug("Player UnEquip Detected. Reset RightHand triggers only.");
					action1 = PacketToString(myPackets.at(1));
					sendToDSX(action1);

				}

				
			}
			nextequip = false;

		}
		
		return RE::BSEventNotifyControl::kContinue;
	}





}
