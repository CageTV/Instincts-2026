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
		std::vector<RE::FormID> Apply(RE::FormID a_wild, RE::FormID a_pet)
		{
			std::vector<RE::FormID> put;
			auto* wild = RE::TESForm::LookupByID<RE::Actor>(a_wild);
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			auto* equip = RE::ActorEquipManager::GetSingleton();
			if (!wild || !pet || !equip) {
				SKSE::log::info("look: wild {:08X} or pet {:08X} is gone, nothing copied", a_wild, a_pet);
				return put;
			}
			const auto* petRace = pet->GetRace();
			int         copied = 0;
			{
				const auto* wb = wild->GetActorBase();
				const auto* ws = wb ? wb->skin : nullptr;
				const auto* pb = pet->GetActorBase();
				const auto* ps = pb ? pb->skin : nullptr;
				const auto* wf = wb ? wb->GetFile(0) : nullptr;
				const auto* wsf = ws ? ws->GetFile(0) : nullptr;
				SKSE::log::info("look: wild {:08X} race {} base {:08X} ({}, from {}) skin {:08X} (from {}); pet base {:08X} skin {:08X}, pet race {}", a_wild,
					wild->GetRace() ? wild->GetRace()->GetFormEditorID() : "?", wb ? wb->GetFormID() : 0, wb && wb->GetName() ? wb->GetName() : "?", wf ? wf->GetFilename() : "?",
					ws ? ws->GetFormID() : 0, wsf ? wsf->GetFilename() : "?", pb ? pb->GetFormID() : 0, ps ? ps->GetFormID() : 0, petRace ? petRace->GetFormEditorID() : "?");
			}
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
				put.push_back(armor->GetFormID());
				SKSE::log::info("look: {} ({:08X}) copied from the wild animal to the pet {:08X}", armor->GetName(), armor->GetFormID(), a_pet);
			}
			if (copied == 0) {
				// SkyTEST / Animallica: the variant is its own actor base on a vanilla race, and the look is that base's skin (WNAM)
				const auto* wildBase = wild->GetActorBase();
				const auto* petBase = pet->GetActorBase();
				auto*       skin = wildBase ? wildBase->skin : nullptr;
				const auto* petSkin = petBase ? petBase->skin : nullptr;
				if (skin && skin != petSkin && !FromVariants(skin)) {
					const auto* file = skin->GetFile(0);
					SKSE::log::info("look: the wild base {:08X} ({}) has the skin {} ({:08X}, from {}); the pet's own skin is {:08X}", wildBase->GetFormID(), wildBase->GetName() ? wildBase->GetName() : "?",
						skin->GetName() ? skin->GetName() : "?", skin->GetFormID(), file ? file->GetFilename() : "?", petSkin ? petSkin->GetFormID() : 0);
					if (petRace && ShowsOn(skin, petRace)) {
						pet->AddObjectToContainer(skin, nullptr, 1, nullptr);
						equip->EquipObject(pet, skin, nullptr, 1, nullptr, true, true, false, true);
						++copied;
						put.push_back(skin->GetFormID());
						SKSE::log::info("look: skin {:08X} of the wild animal put on the pet {:08X}", skin->GetFormID(), a_pet);
					} else {
						SKSE::log::warn("look: skin {:08X} cannot be drawn on pet race {}, skipped", skin->GetFormID(), petRace ? petRace->GetFormEditorID() : "?");
					}
				}
			}
			if (copied == 0) {
				SKSE::log::info("look: the wild animal {:08X} has no different look to copy to the pet, nothing copied", a_wild);
			}
			return put;
		}

		// the equip can be dropped while the pet is still being set up: check and repeat it
		void Ensure(RE::FormID a_pet, const std::vector<RE::FormID>& a_armors)
		{
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			auto* equip = RE::ActorEquipManager::GetSingleton();
			if (!pet || !equip) {
				return;
			}
			for (const auto id : a_armors) {
				auto* armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(id);
				if (!armor || pet->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody) == armor) {
					continue;
				}
				SKSE::log::warn("look: pet {:08X} does not wear {:08X}, equipping again", a_pet, id);
				pet->AddObjectToContainer(armor, nullptr, 1, nullptr);
				equip->EquipObject(pet, armor, nullptr, 1, nullptr, true, true, false, true);
			}
		}

		// what the pet really wears a moment after the copy (body slot and the whole worn inventory)
		void LogWorn(RE::FormID a_pet)
		{
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			if (!pet) {
				return;
			}
			const auto  inv = pet->GetInventory([](RE::TESBoundObject& a_obj) { return a_obj.IsArmor(); });
			std::string worn;
			for (const auto& [obj, data] : inv) {
				if (obj && data.second && data.second->IsWorn()) {
					const auto* file = obj->GetFile(0);
					worn += std::format(" {:08X}({})", obj->GetFormID(), file ? file->GetFilename() : "?");
				}
			}
			const auto* body = pet->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
			SKSE::log::info("look: pet {:08X} 3D {}, body slot wears {:08X}, worn items:{}", a_pet, pet->Is3DLoaded(), body ? body->GetFormID() : 0, worn.empty() ? " none" : worn);
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
				auto put = std::make_shared<std::vector<RE::FormID>>();
				if (const auto* tasks = SKSE::GetTaskInterface()) {
					tasks->AddTask([wild, pet, put] { *put = Apply(wild, pet); });
				}
				for (int i = 0; i < 3; ++i) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1500));
					if (const auto* tasks = SKSE::GetTaskInterface()) {
						tasks->AddTask([pet, put] { Ensure(pet, *put); LogWorn(pet); });
					}
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
		if (!originals || !pets) {
			SKSE::log::info("look: off (Instincts race lists missing)");
			return;
		}
		SKSE::log::info("look: Dynamic Animal Variants Plus {}", dh->LookupModByName(kVariants) ? "found" : "not installed (its skins are skipped)");
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
		// variants that are their own actor base on a vanilla race: the skin of every base whose race is an Instincts original race
		int skins = 0;
		for (auto* npc : dh->GetFormArray<RE::TESNPC>()) {
			auto* race = npc ? npc->GetRace() : nullptr;
			auto* skin = npc ? npc->skin : nullptr;
			if (!race || !skin) {
				continue;
			}
			const auto it = petRaceOf.find(race->GetFormID());
			if (it == petRaceOf.end()) {
				continue;
			}
			bool changed = false;
			for (auto* arma : skin->armorAddons) {
				if (arma && Names(arma, race) && !Names(arma, it->second)) {
					arma->additionalRaces.push_back(it->second);
					changed = true;
				}
			}
			skins += changed ? 1 : 0;
		}
		SKSE::log::info("look: {} wild races mapped to pet races, {} variant armor addons and {} actor-base skins now also name the pet race", petRaceOf.size(), addons, skins);
	}
}
