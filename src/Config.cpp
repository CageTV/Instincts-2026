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
		Pets   pets;
		Box    box;
		const auto kFile = std::filesystem::path("Data/SKSE/Plugins/BeastHarmony/config.json");
	}

	Subdue& Get() { return cfg; }
	Pets&   GetPets() { return pets; }
	Box&    GetBox() { return box; }

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
			const auto pj = j.value("pets", json::object());
			pets.enabled = pj.value("enabled", pets.enabled);
			pets.scanSeconds = pj.value("scanSeconds", pets.scanSeconds);
			const auto rd = [&](const char* name, ClassAI& ai) {
				const auto o = pj.value(name, json::object());
				ai.aggression = std::clamp(o.value("aggression", ai.aggression), 0, 3);
				ai.confidence = std::clamp(o.value("confidence", ai.confidence), 0, 4);
				ai.assistance = std::clamp(o.value("assistance", ai.assistance), 0, 2);
			};
			rd("fighter", pets.fighter);
			rd("scavenger", pets.scavenger);
			rd("gatherer", pets.gatherer);
			rd("producer", pets.producer);
			const auto bj = j.value("activationBox", json::object());
			box.skip = bj.value("skip", box.skip);
			box.delayFrames = std::clamp(bj.value("delayFrames", box.delayFrames), 1, 30);
			box.label = bj.value("label", box.label);
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
		const auto wr = [](const ClassAI& ai) { return json{ { "aggression", ai.aggression }, { "confidence", ai.confidence }, { "assistance", ai.assistance } }; };
		j["pets"] = { { "enabled", pets.enabled }, { "scanSeconds", pets.scanSeconds }, { "fighter", wr(pets.fighter) }, { "scavenger", wr(pets.scavenger) }, { "gatherer", wr(pets.gatherer) }, { "producer", wr(pets.producer) } };
		j["activationBox"] = { { "skip", box.skip }, { "delayFrames", box.delayFrames }, { "label", box.label } };
		std::ofstream(kFile) << j.dump(2);
	}
}
