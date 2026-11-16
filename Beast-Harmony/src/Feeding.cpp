/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#include "PCH.h"
#include "Feeding.h"
#include "Config.h"

namespace Feeding
{
	namespace
	{
		std::atomic<int> g_logged{ 0 };

		// the race the player is playing, looked up in the config table; 0 when the race has no bonus
		float BonusFor(const RE::TESRace* a_race)
		{
			if (!a_race) {
				return 0.0f;
			}
			auto* dh = RE::TESDataHandler::GetSingleton();
			if (!dh) {
				return 0.0f;
			}
			float best = 0.0f;
			for (const auto& r : Config::GetFeeding().races) {
				const auto* form = dh->LookupForm<RE::TESRace>(r.localId, r.file);
				if (form && form == a_race) {
					best = std::max(best, r.bonus);
				}
			}
			return best;
		}

		float Multiplier(RE::StaticFunctionTag*, RE::Actor* a_player)
		{
			const auto& cfg = Config::GetFeeding();
			float bonus = 0.0f;
			if (cfg.enabled && a_player) {
				bonus = BonusFor(a_player->GetRace());
			}
			const float result = 1.0f + bonus;
			if (g_logged.fetch_add(1) < 6) {
				const auto* race = a_player ? a_player->GetRace() : nullptr;
				SKSE::log::info("feeding: multiplier {:.3f} (race {}, enabled {})", result, race ? race->GetName() : "none", cfg.enabled);
			}
			return result;
		}
	}

	bool Bind(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("Multiplier", "BeastHarmony_Feeding", Multiplier);
		return true;
	}
}
