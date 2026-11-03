/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "Look.h"

#include "Config.h"

namespace Look
{
	namespace
	{
		constexpr auto kInstincts = "Instincts - Animal Taming in Skyrim.esp"sv;
		constexpr auto kVariants = "DynamicAnimalVariantsPlus.esp"sv;

		std::map<RE::FormID, RE::TESRace*> petRaceOf;  // wild race -> Instincts pet race

		bool FromVariants(const RE::TESForm* a_form)
		{
			const auto* file = a_form ? a_form->GetFile(0) : nullptr;
			return file && std::string_view(file->GetFilename()) == kVariants;
		}

		bool Names(const RE::TESObjectARMA* a_arma, const RE::TESRace* a_race)
		{
			if (a_arma->race == a_race) {
				return true;
			}
			return std::ranges::find(a_arma->additionalRaces, a_race) != a_arma->additionalRaces.end();
		}

		// the armor is drawn on a_race only if one of its addons names that race
		bool ShowsOn(const RE::TESObjectARMO* a_armor, const RE::TESRace* a_race)
		{
			for (const auto* arma : a_armor->armorAddons) {
				if (arma && Names(arma, a_race)) {
					return true;
				}
			}
			return false;
		}

		// main thread
		void Apply(RE::FormID a_wild, RE::FormID a_pet)
		{
			auto* wild = RE::TESForm::LookupByID<RE::Actor>(a_wild);
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			auto* equip = RE::ActorEquipManager::GetSingleton();
			if (!wild || !pet || !equip) {
				SKSE::log::info("look: wild {:08X} or pet {:08X} is gone, nothing copied", a_wild, a_pet);
				return;
			}
			const auto* petRace = pet->GetRace();
			int         copied = 0;
			const auto  inv = wild->GetInventory([](RE::TESBoundObject& a_obj) { return a_obj.IsArmor(); });
			for (const auto& [obj, data] : inv) {
				const auto* armor = obj ? obj->As<RE::TESObjectARMO>() : nullptr;
				if (armor) {
					const auto* file = armor->GetFile(0);
					SKSE::log::info("look: the wild animal {:08X} carries {} ({:08X}, from {}), worn {}, a variant skin {}", a_wild, armor->GetName() ? armor->GetName() : "?", armor->GetFormID(),
						file ? file->GetFilename() : "?", data.second && data.second->IsWorn(), FromVariants(armor));
				}
				if (!armor || !data.second || !data.second->IsWorn()) {
					continue;
				}
				if (!FromVariants(armor)) {
					continue;  // only the variant skins; the wild animal's ordinary skin stays behind
				}
				if (!petRace || !ShowsOn(armor, petRace)) {
					SKSE::log::warn("look: {} ({:08X}) cannot be drawn on pet race {}, skipped", armor->GetName(), armor->GetFormID(), petRace ? petRace->GetFormEditorID() : "?");
					continue;
				}
				pet->AddObjectToContainer(const_cast<RE::TESObjectARMO*>(armor), nullptr, 1, nullptr);
				equip->EquipObject(pet, const_cast<RE::TESObjectARMO*>(armor), nullptr, 1, nullptr, true, true, false, true);
				++copied;
				SKSE::log::info("look: {} ({:08X}) copied from the wild animal to the pet {:08X}", armor->GetName(), armor->GetFormID(), a_pet);
			}
			if (copied == 0) {
				SKSE::log::info("look: the wild animal {:08X} wore no variant skin that fits the pet, nothing copied", a_wild);
			}
		}

		void CopyLook(RE::StaticFunctionTag*, RE::Actor* a_wild, RE::Actor* a_pet)
		{
			SKSE::log::info("look: CopyLook called from Papyrus (wild {:08X}, pet {:08X})", a_wild ? a_wild->GetFormID() : 0, a_pet ? a_pet->GetFormID() : 0);
			if (!a_wild || !a_pet) {
				return;
			}
			const auto wild = a_wild->GetFormID();
			const auto pet = a_pet->GetFormID();
			// a moment later, on the game thread: the pet has just been placed
			std::thread([wild, pet] {
				std::this_thread::sleep_for(std::chrono::milliseconds(800));
				if (const auto* tasks = SKSE::GetTaskInterface()) {
					tasks->AddTask([wild, pet] { Apply(wild, pet); });
				}
			}).detach();
		}

	}

	bool Bind(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("CopyLook", "BeastHarmony_Look", CopyLook);
		return true;
	}

	void Init()
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh || !Config::GetPets().copyLook) {
			return;
		}
		const auto* originals = dh->LookupForm<RE::BGSListForm>(0x023FD8, kInstincts);
		const auto* pets = dh->LookupForm<RE::BGSListForm>(0x06AFA4, kInstincts);
		if (!originals || !pets || !dh->LookupModByName(kVariants)) {
			SKSE::log::info("look: off (Instincts race lists {}, Dynamic Animal Variants Plus {})", (originals && pets) ? "found" : "missing", dh && dh->LookupModByName(kVariants) ? "found" : "missing");
			return;
		}
		for (std::uint32_t i = 0; i < originals->forms.size() && i < pets->forms.size(); ++i) {
			auto* wild = originals->forms[i] ? originals->forms[i]->As<RE::TESRace>() : nullptr;
			auto* pet = pets->forms[i] ? pets->forms[i]->As<RE::TESRace>() : nullptr;
			if (wild && pet) {
				petRaceOf[wild->GetFormID()] = pet;
			}
		}
		int addons = 0;
		for (auto* arma : dh->GetFormArray<RE::TESObjectARMA>()) {
			if (!arma || !FromVariants(arma)) {
				continue;
			}
			// the addon of a variant skin names the wild race (race, or one of the additional races)
			std::vector<RE::TESRace*> add;
			for (const auto& [wildId, pet] : petRaceOf) {
				auto* wild = RE::TESForm::LookupByID<RE::TESRace>(wildId);
				if (wild && Names(arma, wild) && !Names(arma, pet)) {
					add.push_back(pet);
				}
			}
			for (auto* pet : add) {
				arma->additionalRaces.push_back(pet);
			}
			if (!add.empty()) {
				++addons;
			}
		}
		SKSE::log::info("look: {} wild races mapped to pet races, {} variant armor addons now also name the pet race", petRaceOf.size(), addons);
	}
}
