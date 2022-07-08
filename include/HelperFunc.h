#include <magic_enum.hpp>

using namespace RE;

namespace DSXSkyrim
{

	RE::FormType getFormTypeFromID(RE::FormID id)
	{
		const auto base = RE::TESForm::LookupByID(id)->GetFormType();

		return base;
	}

	RE::TESForm* getTESFormFromID(RE::FormID id)
	{
		const auto base = RE::TESForm::LookupByID(id);


		return base;
	}

	bool validScenario(RE::FormID theobjectid)
	{
		auto theItem = getTESFormFromID(theobjectid);
		bool validCheck = false;

		//have to filter out equipped items that are not what we want: ammo, food, potions, scrolls
		if ((theItem->IsWeapon())){
			return true;

		} else {
					if (theItem->GetFormType() == RE::FormType::Light) {
						auto equippedLight = static_cast<TESObjectLIGH*>(theItem);
						//not sure what distingushes torches from something like a lantern;
						return true;
					}
					else if (theItem->GetFormType() == RE::FormType::Armor) {
						//and if shield
						auto equippedArmor = static_cast<TESObjectARMO*>(theItem);

						if (equippedArmor->IsShield())
						{
							return true;
						}

					}
					else if (theItem->GetFormType() == RE::FormType::Spell) {

						return true;

					}
				}

		return false;
	}

	int getObjectClassType(RE::FormID id)
	{
		// Generate the object weap class, armo class, or just torch or melee

		auto myformtype = RE::TESForm::LookupByID(id)->GetFormType();
		auto base = RE::TESForm::LookupByID(id);

		int result;

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

		if (myformtype == FormType::Weapon) {

			auto equippedWeapon = static_cast<TESObjectWEAP*>(base);
			auto result = magic_enum::enum_integer(equippedWeapon->GetWeaponType());

			return result;
		} 
		else if (myformtype == FormType::Light) {

			return 12;
		} else if (myformtype == FormType::Armor) {

			return 10;
		} else if (myformtype == FormType::Spell) {

			return 11;
		}
		

		return -1; //no weapons
	}

	
}



