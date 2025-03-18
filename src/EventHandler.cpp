#include "EventHandler.h"
#include <spdlog/spdlog.h>
#include <unordered_map>

extern std::vector<DSX::TriggerSetting> userTriggers;
extern std::vector<DSX::Packet> triggerPackets;
extern DSX::NetworkManager networkManager;
extern DSX::Packet lastLeftPacket;
extern DSX::Packet lastRightPacket;
extern std::string actionLeft;
extern std::string actionRight;

namespace
{
	bool isPauseMenuActive = false;
	bool isInventoryMenuActive = false;  // Tied to TweenMenu
	bool isTutorialMenuActive = false;   // Tied to Journal Menu
	bool isMapMenuActive = false;        // Tied to MapMenu
	DSX::Packet prevLeftPacket;
	DSX::Packet prevRightPacket;

	static RE::TESForm* previousLeftItem = nullptr;
	static RE::TESForm* previousRightItem = nullptr;

	bool IsFormIDMatch(RE::TESForm* item, const std::string& formIDStr)
	{
		if (formIDStr.empty())
			return false;
		try {
			uint32_t inDec = std::stoul(formIDStr, nullptr, 16);
			return inDec == item->formID;
		} catch (const std::exception& e) {
			logger::error("Error converting FormID: {}", e.what());
			return false;
		}
	}

	DSX::Packet CreateDefaultPacket(int controllerIndex, DSX::Trigger trigger)
	{
		DSX::Packet packet;
		packet.AddAdaptiveTrigger(controllerIndex, trigger, DSX::TriggerMode::Normal, { 0, 0, 0, 0 });
		packet.AddTriggerThreshold(controllerIndex, trigger, 0);
		return packet;
	}

	DSX::Packet GetDefaultPacket(DSX::Trigger trigger)
	{
		std::string targetName = (trigger == DSX::Trigger::Left) ? "Default Left" : "Default Right";
		for (size_t i = 0; i < userTriggers.size(); i++) {
			if (userTriggers[i].name == targetName) {
				return triggerPackets[i];
			}
		}
		logger::warn("Default trigger setting for {} not found in configuration, using fallback", targetName);
		return CreateDefaultPacket(0, trigger);
	}

	void ApplyTriggerSettingsForHandUnarmed(DSX::Trigger triggerSide)
	{
		auto defaultPacket = GetDefaultPacket(triggerSide);
		if (triggerSide == DSX::Trigger::Left) {
			actionLeft = "HandToHandMelee_Left";
			lastLeftPacket = defaultPacket;
			networkManager.SendPacket(lastLeftPacket);
			logger::info("Applied unarmed settings to left trigger");
		} else {
			actionRight = "HandToHandMelee_Right";
			lastRightPacket = defaultPacket;
			networkManager.SendPacket(lastRightPacket);
			logger::info("Applied unarmed settings to right trigger");
		}
	}

	void ApplyTriggerSettingsForHand(RE::TESForm* item, bool isEquipping, DSX::Trigger triggerSide)
	{
		if (!item || !isEquipping) {
			ApplyTriggerSettingsForHandUnarmed(triggerSide);  // Default to unarmed if no item
			return;
		}

		if (item->formType == RE::FormType::Weapon) {
			auto weapon = static_cast<RE::TESObjectWEAP*>(item);
			bool isLeft = triggerSide == DSX::Trigger::Left;

			bool foundCustom = false;
			for (size_t i = 25; i < userTriggers.size(); i++) {
				if (IsFormIDMatch(weapon, userTriggers[i].formID)) {
					logger::info("Found custom weapon FormID: {}", userTriggers[i].formID);
					foundCustom = true;
					if (userTriggers[i].triggerSide == static_cast<int>(triggerSide)) {
						if (isLeft) {
							actionLeft = "Custom_" + userTriggers[i].formID + "_Left";
							lastLeftPacket = triggerPackets[i];
							networkManager.SendPacket(lastLeftPacket);
						} else {
							actionRight = "Custom_" + userTriggers[i].formID + "_Right";
							lastRightPacket = triggerPackets[i];
							networkManager.SendPacket(lastRightPacket);
						}
					}
					break;
				}
			}

			if (!foundCustom) {
				RE::WEAPON_TYPE type = weapon->GetWeaponType();
				std::string category;
				bool isTwoHanded = false;

				switch (type) {
				case RE::WEAPON_TYPE::kTwoHandSword:
					category = "TwoHandSword";
					isTwoHanded = true;
					actionLeft = "TwoHandSword_Left";
					actionRight = "TwoHandSword_Right";
					break;
				case RE::WEAPON_TYPE::kTwoHandAxe:
					category = "TwoHandAxeMace";
					isTwoHanded = true;
					actionLeft = "TwoHandAxeMace_Left";
					actionRight = "TwoHandAxeMace_Right";
					break;
				case RE::WEAPON_TYPE::kBow:
					category = "Bow";
					isTwoHanded = true;
					actionLeft = "Bow_Left";
					actionRight = "Bow_Right";
					break;
				case RE::WEAPON_TYPE::kCrossbow:
					category = "Crossbow";
					isTwoHanded = true;
					actionLeft = "Crossbow_Left";
					actionRight = "Crossbow_Right";
					break;
				case RE::WEAPON_TYPE::kHandToHandMelee:
					category = "HandToHandMelee";
					if (isLeft)
						actionLeft = "HandToHandMelee_Left";
					else
						actionRight = "HandToHandMelee_Right";
					break;
				case RE::WEAPON_TYPE::kOneHandSword:
					category = "OneHandSword";
					if (isLeft)
						actionLeft = "OneHandSword_Left";
					else
						actionRight = "OneHandSword_Right";
					break;
				case RE::WEAPON_TYPE::kOneHandDagger:
					category = "OneHandDagger";
					if (isLeft)
						actionLeft = "OneHandDagger_Left";
					else
						actionRight = "OneHandDagger_Right";
					break;
				case RE::WEAPON_TYPE::kOneHandAxe:
					category = "OneHandAxe";
					if (isLeft)
						actionLeft = "OneHandAxe_Left";
					else
						actionRight = "OneHandAxe_Right";
					break;
				case RE::WEAPON_TYPE::kOneHandMace:
					category = "OneHandMace";
					if (isLeft)
						actionLeft = "OneHandMace_Left";
					else
						actionRight = "OneHandMace_Right";
					break;
				case RE::WEAPON_TYPE::kStaff:
					category = "Staff";
					if (isLeft)
						actionLeft = "Staff_Left";
					else
						actionRight = "Staff_Right";
					break;
				default:
					break;
				}

				if (isTwoHanded) {
					for (size_t i = 0; i < userTriggers.size(); i++) {
						std::string catActionLeft = userTriggers[i].category + "_Left";
						std::string catActionRight = userTriggers[i].category + "_Right";
						if (catActionLeft == actionLeft) {
							lastLeftPacket = triggerPackets[i];
							networkManager.SendPacket(lastLeftPacket);
						}
						if (catActionRight == actionRight) {
							lastRightPacket = triggerPackets[i];
							networkManager.SendPacket(lastRightPacket);
						}
					}
				} else {
					for (size_t i = 0; i < userTriggers.size(); i++) {
						std::string catAction = userTriggers[i].category + (userTriggers[i].triggerSide == static_cast<int>(DSX::Trigger::Left) ? "_Left" : "_Right");
						if (isLeft && catAction == actionLeft) {
							lastLeftPacket = triggerPackets[i];
							networkManager.SendPacket(lastLeftPacket);
						} else if (!isLeft && catAction == actionRight) {
							lastRightPacket = triggerPackets[i];
							networkManager.SendPacket(lastRightPacket);
						}
					}
				}
			}
		} else if (item->formType == RE::FormType::Spell) {
			auto spell = static_cast<RE::SpellItem*>(item);
			bool isLeft = triggerSide == DSX::Trigger::Left;

			bool foundCustom = false;
			for (size_t i = 25; i < userTriggers.size(); i++) {
				if (IsFormIDMatch(spell, userTriggers[i].formID)) {
					logger::info("Found custom spell FormID: {}", userTriggers[i].formID);
					foundCustom = true;
					if (userTriggers[i].triggerSide == static_cast<int>(triggerSide)) {
						if (isLeft) {
							actionLeft = "Custom_" + userTriggers[i].formID + "_Left";
							lastLeftPacket = triggerPackets[i];
							networkManager.SendPacket(lastLeftPacket);
						} else {
							actionRight = "Custom_" + userTriggers[i].formID + "_Right";
							lastRightPacket = triggerPackets[i];
							networkManager.SendPacket(lastRightPacket);
						}
					}
					break;
				}
			}

			if (!foundCustom) {
				if (isLeft) {
					actionLeft = "Left Handed Magic";
					for (size_t i = 0; i < userTriggers.size(); i++) {
						if (userTriggers[i].name == "Left Handed Magic") {
							lastLeftPacket = triggerPackets[i];
							networkManager.SendPacket(lastLeftPacket);
							break;
						}
					}
				} else {
					actionRight = "Right Handed Magic";
					for (size_t i = 0; i < userTriggers.size(); i++) {
						if (userTriggers[i].name == "Right Handed Magic") {
							lastRightPacket = triggerPackets[i];
							networkManager.SendPacket(lastRightPacket);
							break;
						}
					}
				}
			}
		}
	}
}

namespace DSX
{
	EquipEventHandler* EquipEventHandler::GetSingleton()
	{
		static EquipEventHandler singleton;
		logger::info("EquipEventHandler singleton created");
		return &singleton;
	}

	RE::BSEventNotifyControl EquipEventHandler::ProcessEvent(const RE::TESEquipEvent* a_event,
		RE::BSTEventSource<RE::TESEquipEvent>*)
	{
		if (!a_event || !a_event->actor || !a_event->actor->IsPlayerRef()) {
			logger::debug("EquipEventHandler: Ignoring event (null or not player)");
			return RE::BSEventNotifyControl::kContinue;
		}

		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			logger::error("EquipEventHandler: Player singleton is null");
			return RE::BSEventNotifyControl::kContinue;
		}

		RE::TESForm* item = RE::TESForm::LookupByID(a_event->baseObject);
		const char* itemName = item ? item->GetName() : "Unknown Item";
		bool isEquipping = a_event->equipped;

		logger::info("EquipEvent: {} '{}'", isEquipping ? "Equipping" : "Unequipping", itemName);

		auto currentLeftItem = player->GetEquippedObject(true);
		auto currentRightItem = player->GetEquippedObject(false);

		if (isEquipping) {
			if (currentLeftItem == item) {
				logger::info("Equipped '{}' to LEFT hand", itemName);
				ApplyTriggerSettingsForHand(item, true, DSX::Trigger::Left);
			}
			if (currentRightItem == item) {
				logger::info("Equipped '{}' to RIGHT hand", itemName);
				ApplyTriggerSettingsForHand(item, true, DSX::Trigger::Right);
			}
		} else {
			if (previousLeftItem == item && !currentLeftItem) {
				logger::info("Unequipped '{}' from LEFT hand", itemName);
				ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Left);
			}
			if (previousRightItem == item && !currentRightItem) {
				logger::info("Unequipped '{}' from RIGHT hand", itemName);
				ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Right);
			}
		}

		previousLeftItem = currentLeftItem;
		previousRightItem = currentRightItem;

		return RE::BSEventNotifyControl::kContinue;
	}

	MenuEventHandler* MenuEventHandler::GetSingleton()
	{
		static MenuEventHandler singleton;
		logger::info("MenuEventHandler singleton created");
		return &singleton;
	}

	RE::BSEventNotifyControl MenuEventHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
		RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		if (!a_event) {
			logger::debug("MenuEventHandler: Ignoring null event");
			return RE::BSEventNotifyControl::kContinue;
		}

		logger::info("Menu event: {} - {}", a_event->opening ? "Opening" : "Closing", a_event->menuName.c_str());

		bool prevAnyMenuActive = isPauseMenuActive || isInventoryMenuActive || isTutorialMenuActive || isMapMenuActive;

		if (a_event->menuName == "PauseMenu") {
			isPauseMenuActive = a_event->opening;
		} else if (a_event->menuName == "TweenMenu") {
			isInventoryMenuActive = a_event->opening;
		} else if (a_event->menuName == "Journal Menu") {
			isTutorialMenuActive = a_event->opening;
		} else if (a_event->menuName == "MapMenu") {
			isMapMenuActive = a_event->opening;
		}

		bool isAnyMenuActive = isPauseMenuActive || isInventoryMenuActive || isTutorialMenuActive || isMapMenuActive;
		HandleMenuStateChange(isAnyMenuActive, prevAnyMenuActive);

		return RE::BSEventNotifyControl::kContinue;
	}

	void HandleMenuStateChange(bool isAnyMenuActive, bool wasAnyMenuActive)
	{
		logger::info("HandleMenuStateChange called: isAnyMenuActive={}, wasAnyMenuActive={}",
			isAnyMenuActive, wasAnyMenuActive);

		if (isAnyMenuActive != wasAnyMenuActive) {
			if (isAnyMenuActive) {
				prevLeftPacket = lastLeftPacket;
				prevRightPacket = lastRightPacket;
				lastLeftPacket = GetDefaultPacket(DSX::Trigger::Left);
				lastRightPacket = GetDefaultPacket(DSX::Trigger::Right);
				networkManager.SendPacket(lastLeftPacket);
				networkManager.SendPacket(lastRightPacket);
				actionLeft = "Default_Left";
				actionRight = "Default_Right";
				logger::info("Applied default triggers for menu open");
			} else {
				lastLeftPacket = prevLeftPacket;
				lastRightPacket = prevRightPacket;
				networkManager.SendPacket(lastLeftPacket);
				networkManager.SendPacket(lastRightPacket);
				logger::info("Restored previous trigger settings after all menus closed");

				auto player = RE::PlayerCharacter::GetSingleton();
				if (player) {
					auto leftItem = player->GetEquippedObject(true);
					auto rightItem = player->GetEquippedObject(false);
					if (leftItem) {
						ApplyTriggerSettingsForHand(leftItem, true, DSX::Trigger::Left);
					} else {
						ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Left);
					}
					if (rightItem) {
						ApplyTriggerSettingsForHand(rightItem, true, DSX::Trigger::Right);
					} else {
						ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Right);
					}
				}
			}
		}
	}

	void RegisterEventHandlers()
	{
		logger::info("RegisterEventHandlers: Registering event handlers");

		// Register Menu Event Handler
		auto ui = RE::UI::GetSingleton();
		if (ui) {
			ui->AddEventSink(MenuEventHandler::GetSingleton());
			logger::info("Registered MenuEventHandler successfully");
		} else {
			logger::error("Failed to get UI singleton");
		}

		// Register Equip Event Handler
		auto eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (eventSourceHolder) {
			eventSourceHolder->AddEventSink(EquipEventHandler::GetSingleton());
			logger::info("Registered EquipEventHandler successfully");
		} else {
			logger::error("Failed to get ScriptEventSourceHolder singleton");
		}

		logger::info("RegisterEventHandlers: Event handlers registered");
	}

	void CheckWeaponOnGameLoad()
	{
		logger::info("Game loaded, checking equipped items");
		auto player = RE::PlayerCharacter::GetSingleton();
		if (player) {
			previousLeftItem = player->GetEquippedObject(true);
			previousRightItem = player->GetEquippedObject(false);

			if (previousLeftItem) {
				ApplyTriggerSettingsForHand(previousLeftItem, true, DSX::Trigger::Left);
			} else {
				ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Left);
			}

			if (previousRightItem) {
				ApplyTriggerSettingsForHand(previousRightItem, true, DSX::Trigger::Right);
			} else {
				ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Right);
			}
		} else {
			logger::error("CheckWeaponOnGameLoad: Player singleton is null");
			ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Left);
			ApplyTriggerSettingsForHandUnarmed(DSX::Trigger::Right);
		}
	}
}