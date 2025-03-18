#pragma once

#include "DSXController.hpp"
#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>

namespace DSX {
    void RegisterEventHandlers();
    void CheckWeaponOnGameLoad();
    void HandleMenuStateChange(bool isAnyMenuActive, bool wasAnyMenuActive);

    class EquipEventHandler : public RE::BSTEventSink<RE::TESEquipEvent> {
    public:
        static EquipEventHandler* GetSingleton();
        RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event,
                                              RE::BSTEventSource<RE::TESEquipEvent>* a_source) override;

    private:
        EquipEventHandler() = default;
        EquipEventHandler(const EquipEventHandler&) = delete;
        EquipEventHandler(EquipEventHandler&&) = delete;
        EquipEventHandler& operator=(const EquipEventHandler&) = delete;
        EquipEventHandler& operator=(EquipEventHandler&&) = delete;
    };

    class MenuEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static MenuEventHandler* GetSingleton();
		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;

	private:
		MenuEventHandler() = default;
		MenuEventHandler(const MenuEventHandler&) = delete;
		MenuEventHandler(MenuEventHandler&&) = delete;
		MenuEventHandler& operator=(const MenuEventHandler&) = delete;
		MenuEventHandler& operator=(MenuEventHandler&&) = delete;
	};
}
