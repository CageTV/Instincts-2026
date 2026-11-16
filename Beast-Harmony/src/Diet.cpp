/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#include "PCH.h"
#include "Diet.h"
#include "Config.h"

#include <unordered_set>
#include <vector>

namespace Diet
{
	namespace
	{
		constexpr auto kInstincts = "Instincts - Animal Taming in Skyrim.esp"sv;

		struct Item
		{
			std::uint32_t    local;
			std::string_view file;
		};

		// not meat: honey and sweets, bee products, wings, dartwings, torchbug, jellies (eggs, hearts, tails, fat and other animal parts stay)
		constexpr Item kNotMeat[] = {
			{ 0x10394D, "Skyrim.esm"sv },   // Honey
			{ 0x064B38, "Skyrim.esm"sv },   // Honey Nut Treat
			{ 0x064B30, "Skyrim.esm"sv },   // Boiled Creme Treat
			{ 0x0B08C5, "Skyrim.esm"sv },   // Honeycomb
			{ 0x0A9191, "Skyrim.esm"sv },   // Beehive Husk
			{ 0x0A9195, "Skyrim.esm"sv },   // Bee
			{ 0x04DA73, "Skyrim.esm"sv },   // Torchbug Thorax
			{ 0x0727DF, "Skyrim.esm"sv },   // Luna Moth Wing
			{ 0x0727E0, "Skyrim.esm"sv },   // Butterfly Wing
			{ 0x0727DE, "Skyrim.esm"sv },   // Blue Butterfly Wing
			{ 0x0E4F0C, "Skyrim.esm"sv },   // Blue Dartwing
			{ 0x0BB956, "Skyrim.esm"sv },   // Orange Dartwing
			{ 0x01CD72, "Dragonborn.esm"sv }, // Netch Jelly
			{ 0x01CD71, "Dragonborn.esm"sv }, // Ash Hopper Jelly
		};

		// not a herbivore's food: poisons, Nirnroot, Spriggan sap, sweets and pie
		constexpr Item kNotPlant[] = {
			{ 0x0516C8, "Skyrim.esm"sv },   // Deathbell
			{ 0x02F44C, "Skyrim.esm"sv },   // Nightshade
			{ 0x0185FB, "Dawnguard.esm"sv }, // Poison Bloom
			{ 0x059B86, "Skyrim.esm"sv },   // Nirnroot
			{ 0x0B701A, "Skyrim.esm"sv },   // Crimson Nirnroot
			{ 0x063B5F, "Skyrim.esm"sv },   // Spriggan Sap
			{ 0x064B39, "Skyrim.esm"sv },   // Long Taffy Treat
			{ 0x064B43, "Skyrim.esm"sv },   // Apple Pie
		};

		template <std::size_t N>
		std::unordered_set<const RE::TESForm*> Resolve(RE::TESDataHandler* a_dh, const Item (&a_items)[N])
		{
			std::unordered_set<const RE::TESForm*> out;
			for (const auto& i : a_items) {
				if (const auto* f = a_dh->LookupForm(i.local, i.file)) {
					out.insert(f);
				} else {
					SKSE::log::warn("diet: {:06X} in {} not found", i.local, i.file);
				}
			}
			return out;
		}

		// remove every form in a_blocked from the list; returns how many were removed
		std::size_t Trim(RE::BGSListForm* a_list, const std::unordered_set<const RE::TESForm*>& a_blocked)
		{
			if (!a_list) {
				return 0;
			}
			std::vector<RE::TESForm*> keep;
			keep.reserve(a_list->forms.size());
			for (auto* f : a_list->forms) {
				if (!f || !a_blocked.contains(f)) {
					keep.push_back(f);
				}
			}
			const auto removed = a_list->forms.size() - keep.size();
			if (removed) {
				a_list->forms.clear();
				for (auto* f : keep) {
					a_list->forms.push_back(f);
				}
			}
			return removed;
		}
	}

	void Init()
	{
		if (!Config::GetDiet().enabled) {
			SKSE::log::info("diet: off in config");
			return;
		}
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh || !dh->LookupModByName(kInstincts)) {
			SKSE::log::info("diet: Instincts not loaded");
			return;
		}
		const auto notMeat = Resolve(dh, kNotMeat);
		const auto notPlant = Resolve(dh, kNotPlant);
		struct Target
		{
			std::uint32_t local;
			const char*   name;
			bool          meat;
			bool          plant;
		};
		// FoodMeat, FoodMeatFish, FoodVegetables, FoodVegetablesMeatFish (Everything and Fish are left as they are)
		for (const auto& t : { Target{ 0x07A2A7, "Meat", true, false }, Target{ 0x07A2A6, "MeatFish", true, false }, Target{ 0x07A2A8, "Vegetables", false, true },
				 Target{ 0x07A2A9, "VegetablesMeatFish", true, true } }) {
			auto* list = dh->LookupForm<RE::BGSListForm>(t.local, kInstincts);
			if (!list) {
				SKSE::log::warn("diet: list {} not found", t.name);
				continue;
			}
			const auto before = list->forms.size();
			std::size_t removed = 0;
			if (t.meat) {
				removed += Trim(list, notMeat);
			}
			if (t.plant) {
				removed += Trim(list, notPlant);
			}
			SKSE::log::info("diet: {} list {} -> {} items ({} removed)", t.name, before, list->forms.size(), removed);
		}
	}
}
