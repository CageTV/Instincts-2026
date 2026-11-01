/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Config.h"

#include <nlohmann/json.hpp>

namespace Config
{
	namespace
	{
		using json = nlohmann::json;
		Subdue cfg;
		const auto kFile = std::filesystem::path("Data/SKSE/Plugins/BeastHarmony/config.json");
	}

	Subdue& Get() { return cfg; }

	void Load()
	{
		std::ifstream f(kFile);
		if (!f) {
			Save();
			return;
		}
		try {
			const auto j = json::parse(f, nullptr, true, true);
			const auto s = j.value("subdue", json::object());
			cfg.enabled = s.value("enabled", cfg.enabled);
			cfg.mode = s.value("mode", cfg.mode);
			cfg.toggleKey = s.value("toggleKey", cfg.toggleKey);
			cfg.threshold = std::clamp(s.value("threshold", cfg.threshold), 0.05f, 0.9f);
			cfg.seconds = std::clamp(s.value("seconds", cfg.seconds), 5.0f, 600.0f);
			const auto c = s.value("chance", json::object());
			cfg.chanceFists = std::clamp(c.value("fists", cfg.chanceFists), 0.0f, 1.0f);
			cfg.chanceBlunt = std::clamp(c.value("blunt", cfg.chanceBlunt), 0.0f, 1.0f);
			cfg.chanceBladed = std::clamp(c.value("bladed", cfg.chanceBladed), 0.0f, 1.0f);
			const auto l = s.value("levels", json::object());
			cfg.levelBig = l.value("big", cfg.levelBig);
			cfg.levelGreat = l.value("great", cfg.levelGreat);
			cfg.levelHuge = l.value("huge", cfg.levelHuge);
			cfg.slotLimit = std::clamp(s.value("slotLimit", cfg.slotLimit), 1, 10);
		} catch (const std::exception& e) {
			SKSE::log::warn("config.json unreadable ({}), using defaults", e.what());
		}
	}

	void Save()
	{
		std::error_code ec;
		std::filesystem::create_directories(kFile.parent_path(), ec);
		json j = { { "_help", { { "mode", "hotkey: a knocked-out roll only while Taming Mode is on (toggleKey); always; never" }, { "toggleKey", "DirectInput scan code, 0x2B = 43 = backslash" }, { "chance", "chance an animal is knocked out instead of killed when a hit takes it below the threshold" } } },
			{ "subdue", { { "enabled", cfg.enabled }, { "mode", cfg.mode }, { "toggleKey", cfg.toggleKey }, { "threshold", cfg.threshold }, { "seconds", cfg.seconds },
				{ "chance", { { "fists", cfg.chanceFists }, { "blunt", cfg.chanceBlunt }, { "bladed", cfg.chanceBladed } } },
				{ "levels", { { "big", cfg.levelBig }, { "great", cfg.levelGreat }, { "huge", cfg.levelHuge } } }, { "slotLimit", cfg.slotLimit } } } };
		std::ofstream(kFile) << j.dump(2);
	}
}
