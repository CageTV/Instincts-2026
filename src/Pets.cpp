/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Pets.h"
#include "Config.h"

#include <thread>
#include <unordered_set>

namespace Pets
{
	namespace
	{
		constexpr auto kInstincts = "Instincts - Animal Taming in Skyrim.esp"sv;

		enum class Class { Fighter, Scavenger, Gatherer, Producer };

		const char* ClassName(Class c)
		{
			switch (c) {
			case Class::Fighter: return "fighter";
			case Class::Scavenger: return "scavenger";
			case Class::Gatherer: return "gatherer";
			default: return "producer";
			}
		}

		struct Species
		{
			std::uint32_t    local;
			std::string_view file;
			Class            cls;
			const char*      name;
		};

		// The ORIGINAL (wild) race of every animal Instincts can tame -> its class. Pets are matched through the parallel race lists Instincts keeps.
		constexpr Species kSpecies[] = {
			{ 0x01B658, "Dragonborn.esm", Class::Fighter, "scrib / ash hopper" },
			{ 0x0131E8, "Skyrim.esm", Class::Fighter, "cave bear" },
			{ 0x0131E7, "Skyrim.esm", Class::Fighter, "bear" },
			{ 0x0131E9, "Skyrim.esm", Class::Fighter, "snow bear" },
			{ 0x024038, "Dragonborn.esm", Class::Fighter, "boar" },
			{ 0x0131EB, "Skyrim.esm", Class::Fighter, "chaurus" },
			{ 0x0051FB, "Dawnguard.esm", Class::Fighter, "chaurus hunter" },
			{ 0x0A5601, "Skyrim.esm", Class::Fighter, "chaurus reaper" },
			{ 0x0A919D, "Skyrim.esm", Class::Gatherer, "chicken" },
			{ 0x04E785, "Skyrim.esm", Class::Producer, "cow" },
			{ 0x0CF89B, "Skyrim.esm", Class::Gatherer, "deer" },
			{ 0x00D0B2, "Dawnguard.esm", Class::Gatherer, "glow deer" },
			{ 0x0131ED, "Skyrim.esm", Class::Gatherer, "elk" },
			{ 0x060D68, kInstincts, Class::Scavenger, "snow fox" },
			{ 0x109C7C, "Skyrim.esm", Class::Scavenger, "fox" },
			{ 0x04E507, "Skyrim.esm", Class::Fighter, "giant frostbite spider" },
			{ 0x060D4A, kInstincts, Class::Fighter, "giant snowy frostbite spider" },
			{ 0x053477, "Skyrim.esm", Class::Fighter, "large frostbite spider" },
			{ 0x060D49, kInstincts, Class::Fighter, "large snowy frostbite spider" },
			{ 0x0131F8, "Skyrim.esm", Class::Fighter, "frostbite spider" },
			{ 0x060D51, kInstincts, Class::Fighter, "snowy frostbite spider" },
			{ 0x0131FA, "Skyrim.esm", Class::Gatherer, "goat" },
			{ 0x06DC99, "Skyrim.esm", Class::Gatherer, "hare" },
			{ 0x0131FC, "Skyrim.esm", Class::Fighter, "horker" },
			{ 0x0131FF, "Skyrim.esm", Class::Fighter, "mammoth" },
			{ 0x0BA545, "Skyrim.esm", Class::Scavenger, "mudcrab" },
			{ 0x060D5A, kInstincts, Class::Scavenger, "giant mudcrab" },
			{ 0x060D59, kInstincts, Class::Scavenger, "large mudcrab" },
			{ 0x060D58, kInstincts, Class::Scavenger, "medium mudcrab" },
			{ 0x013200, "Skyrim.esm", Class::Fighter, "sabre cat" },
			{ 0x013202, "Skyrim.esm", Class::Fighter, "snowy sabre cat" },
			{ 0x00D0B6, "Dawnguard.esm", Class::Fighter, "glow sabre cat" },
			{ 0x013201, "Skyrim.esm", Class::Scavenger, "skeever" },
			{ 0x01320A, "Skyrim.esm", Class::Fighter, "wolf" },
			{ 0x019DB6, kInstincts, Class::Fighter, "ice wolf" },
		};

		struct State
		{
			RE::BGSListForm*                               originalRaces = nullptr;
			RE::BGSListForm*                               newRaces = nullptr;
			std::unordered_map<RE::FormID, const Species*> byOriginalRace;
			std::unordered_set<RE::FormID>                 seen;  // pets already logged this session
			bool                                           ok = false;
		} S;

		std::atomic<bool> stopping{ false };

		const Config::ClassAI& AIFor(Class c)
		{
			const auto& p = Config::GetPets();
			switch (c) {
			case Class::Fighter: return p.fighter;
			case Class::Scavenger: return p.scavenger;
			case Class::Gatherer: return p.gatherer;
			default: return p.producer;
			}
		}

		const Species* SpeciesOf(RE::Actor* a_pet)
		{
			const auto* race = a_pet->GetRace();
			if (!race || !S.newRaces || !S.originalRaces) {
				return nullptr;
			}
			const auto& nr = S.newRaces->forms;
			for (std::uint32_t i = 0; i < nr.size() && i < S.originalRaces->forms.size(); ++i) {
				if (nr[i] == race) {
					const auto* orig = S.originalRaces->forms[i];
					const auto  it = orig ? S.byOriginalRace.find(orig->GetFormID()) : S.byOriginalRace.end();
					return it != S.byOriginalRace.end() ? it->second : nullptr;
				}
			}
			return nullptr;
		}

		bool IsInstinctsPet(RE::Actor* a_actor)
		{
			const auto* base = a_actor->GetActorBase();
			if (!base) {
				return false;
			}
			const auto* file = base->GetFile(0);
			return file && file->GetFilename() == kInstincts;
		}

		void Apply(RE::Actor* a_pet, const Species& sp)
		{
			const auto& ai = AIFor(sp.cls);
			auto*       av = a_pet->AsActorValueOwner();
			const float wasA = av->GetActorValue(RE::ActorValue::kAggression);
			const float wasC = av->GetActorValue(RE::ActorValue::kConfidence);
			const float wasH = av->GetActorValue(RE::ActorValue::kAssistance);
			const bool  same = static_cast<int>(wasA) == ai.aggression && static_cast<int>(wasC) == ai.confidence && static_cast<int>(wasH) == ai.assistance;
			if (!same) {
				av->SetActorValue(RE::ActorValue::kAggression, static_cast<float>(ai.aggression));
				av->SetActorValue(RE::ActorValue::kConfidence, static_cast<float>(ai.confidence));
				av->SetActorValue(RE::ActorValue::kAssistance, static_cast<float>(ai.assistance));
			}
			if (!same || S.seen.insert(a_pet->GetFormID()).second) {
				const int nowA = static_cast<int>(av->GetActorValue(RE::ActorValue::kAggression));
				const int nowC = static_cast<int>(av->GetActorValue(RE::ActorValue::kConfidence));
				const int nowH = static_cast<int>(av->GetActorValue(RE::ActorValue::kAssistance));
				SKSE::log::info("pet {} [{:08X}] = {} ({}): aggression/confidence/assistance was {}/{}/{}, wanted {}/{}/{}, now {}/{}/{}", a_pet->GetName() ? a_pet->GetName() : "?", a_pet->GetFormID(), sp.name,
					ClassName(sp.cls), static_cast<int>(wasA), static_cast<int>(wasC), static_cast<int>(wasH), ai.aggression, ai.confidence, ai.assistance, nowA, nowC, nowH);
			}
		}

		// runs on the game thread
		void Scan()
		{
			if (!S.ok || !Config::GetPets().enabled) {
				return;
			}
			auto* player = RE::PlayerCharacter::GetSingleton();
			auto* lists = RE::ProcessLists::GetSingleton();
			if (!player || !player->Is3DLoaded() || !lists) {
				return;
			}
			for (auto& handle : lists->highActorHandles) {
				const auto actor = handle.get();
				if (!actor || actor.get() == player || actor->IsDead() || !IsInstinctsPet(actor.get())) {
					continue;
				}
				if (actor->IsInCombat()) {
					continue;  // the engine raises aggression / drops confidence mid-fight (a hurt animal flees): leave that alone, put the class values back afterwards
				}
				if (const auto* sp = SpeciesOf(actor.get())) {
					Apply(actor.get(), *sp);
				}
			}
		}
	}

	void Init()
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return;
		}
		S.originalRaces = dh->LookupForm<RE::BGSListForm>(0x023FD8, kInstincts);
		S.newRaces = dh->LookupForm<RE::BGSListForm>(0x06AFA4, kInstincts);
		std::size_t found = 0;
		for (const auto& sp : kSpecies) {
			if (const auto* race = dh->LookupForm<RE::TESRace>(sp.local, sp.file)) {
				S.byOriginalRace[race->GetFormID()] = &sp;
				++found;
			} else {
				SKSE::log::warn("pet classes: race {:06X} ({}) from {} not found", sp.local, sp.name, sp.file);
			}
		}
		S.ok = S.originalRaces && S.newRaces && found > 0;
		SKSE::log::info("pet classes {}: {} of {} species known, race lists {}", S.ok ? "on" : "OFF", found, std::size(kSpecies), (S.originalRaces && S.newRaces) ? "found" : "NOT FOUND");
		if (!S.ok) {
			return;
		}
		// a light poll: every few seconds ask the game thread to look over the loaded animals
		std::thread([] {
			while (!stopping.load()) {
				const auto ms = static_cast<int>(std::clamp(Config::GetPets().scanSeconds, 0.5f, 30.0f) * 1000.0f);
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				if (!stopping.load()) {
					SKSE::GetTaskInterface()->AddTask([] { Scan(); });
				}
			}
		}).detach();
	}

	std::string ClassOf(RE::Actor* a_actor)
	{
		if (!S.ok || !a_actor || !IsInstinctsPet(a_actor)) {
			return {};
		}
		const auto* sp = SpeciesOf(a_actor);
		return sp ? ClassName(sp->cls) : std::string{};
	}
}
