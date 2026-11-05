/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "MenuPage.h"
#include "Config.h"
#include "KeyPicker.h"

namespace MenuPage
{
	namespace
	{
		void Render()
		{
			bool changed = false;
			auto& menu = Config::GetMenu();

			ImGuiMCP::SeparatorText("Pet command prompts (Do something for me)");
			if (ImGuiMCP::Checkbox("Show the commands as SkyPrompt prompts##bh_sp", &menu.useSkyPrompt)) {
				changed = true;
			}
			if (KeyPicker::Field("bh_kg", "Gather what you can find:", &menu.keyGather)) {
				changed = true;
			}
			if (KeyPicker::Field("bh_ko", "Something else (the old favor mode):", &menu.keyOther)) {
				changed = true;
			}
			if (KeyPicker::Field("bh_kc", "Never mind:", &menu.keyCancel)) {
				changed = true;
			}
			if (ImGuiMCP::SliderFloat("Prompts go away after (seconds)##bh_secs", &menu.seconds, 5.0f, 60.0f, "%.0f")) {
				changed = true;
			}
			ImGuiMCP::Text("Pick keys nothing else uses. The game's own controls are flagged next to the key;");
			ImGuiMCP::Text("keys that other mods use cannot be detected.");

			ImGuiMCP::SeparatorText("Other keys");
			auto& jobs = Config::GetJobs();
			if (KeyPicker::Field("bh_jobs", "Send every nearby gatherer / scavenger pet out:", &jobs.key)) {
				changed = true;
			}
			auto& sub = Config::Get();
			if (KeyPicker::Field("bh_taming", "Taming Mode on / off:", &sub.toggleKey)) {
				changed = true;
			}

			if (changed) {
				Config::Save();
			}
		}
	}

	void Init()
	{
		if (!SKSEMenuFramework::IsInstalled()) {
			SKSE::log::info("menu page: SKSE Menu Framework not installed, keys stay in config.json");
			return;
		}
		KeyPicker::Install();
		SKSEMenuFramework::SetSection("Beast Harmony");
		SKSEMenuFramework::AddSectionItem("Keys", Render);
		SKSE::log::info("menu page: Beast Harmony keys page registered");
	}
}
