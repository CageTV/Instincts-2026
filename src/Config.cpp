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
		Diet   diet;
		Spike  spike;
		Jobs   jobs;
		Menu   menu;
		Widget widget;
		const auto kFile = std::filesystem::path("Data/SKSE/Plugins/BeastHarmony/config.json");
	}

	Subdue& Get() { return cfg; }
	Pets&   GetPets() { return pets; }
	Box&    GetBox() { return box; }
	Diet&   GetDiet() { return diet; }
	Spike&  GetSpike() { return spike; }
	Jobs&   GetJobs() { return jobs; }
	Menu&   GetMenu() { return menu; }
	Widget& GetWidget() { return widget; }

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
			const auto wj = j.value("widget", json::object());
			widget.enabled = wj.value("enabled", widget.enabled);
			widget.x = std::clamp(wj.value("x", widget.x), 0.0f, 1.0f);
			widget.y = std::clamp(wj.value("y", widget.y), 0.0f, 1.0f);
			widget.scale = std::clamp(wj.value("scale", widget.scale), 0.5f, 3.0f);
			const auto pj = j.value("pets", json::object());
			pets.enabled = pj.value("enabled", pets.enabled);
			pets.scanSeconds = pj.value("scanSeconds", pets.scanSeconds);
			pets.copyLook = pj.value("copyLook", pets.copyLook);
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
			diet.enabled = j.value("diet", json::object()).value("enabled", diet.enabled);
			const auto sj = j.value("spike", json::object());
			spike.enabled = sj.value("enabled", spike.enabled);
			spike.key = sj.value("key", spike.key);
			spike.modifier = sj.value("modifier", spike.modifier);
			spike.distance = std::clamp(sj.value("distance", spike.distance), 100.0f, 3000.0f);
			const auto jb = j.value("jobs", json::object());
			jobs.enabled = jb.value("enabled", jobs.enabled);
			jobs.key = jb.value("key", jobs.key);
			jobs.modifier = jb.value("modifier", jobs.modifier);
			jobs.range = std::clamp(jb.value("range", jobs.range), 300.0f, 8000.0f);
			jobs.maxItems = std::clamp(jb.value("maxItems", jobs.maxItems), 1, 12);
			jobs.gameHours = std::clamp(jb.value("gameHours", jobs.gameHours), 0.0f, 24.0f);
			jobs.arriveRadius = std::clamp(jb.value("arriveRadius", jobs.arriveRadius), 75.0f, 600.0f);
			jobs.sound = jb.value("sound", jobs.sound);
			jobs.soundVolume = std::clamp(jb.value("soundVolume", jobs.soundVolume), 0.0f, 1.0f);
			jobs.huntRange = std::clamp(jb.value("huntRange", jobs.huntRange), 500.0f, 10000.0f);
			jobs.huntKills = std::clamp(jb.value("huntKills", jobs.huntKills), 1, 5);
			jobs.huntSecondsPerPrey = std::clamp(jb.value("huntSecondsPerPrey", jobs.huntSecondsPerPrey), 15.0f, 600.0f);
			jobs.huntHealthWolf = std::clamp(jb.value("huntHealthWolf", jobs.huntHealthWolf), 1.0f, 5000.0f);
			jobs.huntHealthCat = std::clamp(jb.value("huntHealthCat", jobs.huntHealthCat), 1.0f, 5000.0f);
			jobs.huntHealthBear = std::clamp(jb.value("huntHealthBear", jobs.huntHealthBear), 1.0f, 5000.0f);
			jobs.huntLootFraction = std::clamp(jb.value("huntLootFraction", jobs.huntLootFraction), 0.0f, 1.0f);
			jobs.keyAttack = jb.value("keyAttack", jobs.keyAttack);
			jobs.attackModifier = jb.value("attackModifier", jobs.attackModifier);
			jobs.attackPetRange = std::clamp(jb.value("attackPetRange", jobs.attackPetRange), 500.0f, 10000.0f);
			jobs.attackSeconds = std::clamp(jb.value("attackSeconds", jobs.attackSeconds), 10.0f, 600.0f);
			jobs.alert = jb.value("alert", jobs.alert);
			jobs.alertRange = std::clamp(jb.value("alertRange", jobs.alertRange), 500.0f, 8000.0f);
			jobs.alertCooldown = std::clamp(jb.value("alertCooldown", jobs.alertCooldown), 5.0f, 600.0f);
			jobs.alertScanSeconds = std::clamp(jb.value("alertScanSeconds", jobs.alertScanSeconds), 0.5f, 30.0f);
			const auto mj = j.value("menu", json::object());
			menu.useSkyPrompt = mj.value("useSkyPrompt", menu.useSkyPrompt);
			menu.keyGather = mj.value("keyGather", menu.keyGather);
			menu.keyOther = mj.value("keyOther", menu.keyOther);
			menu.keyCancel = mj.value("keyCancel", menu.keyCancel);
			menu.keyHunt = mj.value("keyHunt", menu.keyHunt);
			menu.seconds = std::clamp(mj.value("seconds", menu.seconds), 3.0f, 120.0f);
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
		j["pets"] = { { "enabled", pets.enabled }, { "scanSeconds", pets.scanSeconds }, { "copyLook", pets.copyLook }, { "fighter", wr(pets.fighter) }, { "scavenger", wr(pets.scavenger) }, { "gatherer", wr(pets.gatherer) }, { "producer", wr(pets.producer) } };
		j["activationBox"] = { { "skip", box.skip }, { "delayFrames", box.delayFrames }, { "label", box.label } };
		j["widget"] = { { "enabled", widget.enabled }, { "x", widget.x }, { "y", widget.y }, { "scale", widget.scale } };
		j["diet"] = { { "enabled", diet.enabled } };
		j["jobs"] = { { "enabled", jobs.enabled }, { "key", jobs.key }, { "modifier", jobs.modifier }, { "range", jobs.range }, { "maxItems", jobs.maxItems }, { "gameHours", jobs.gameHours }, { "arriveRadius", jobs.arriveRadius }, { "sound", jobs.sound }, { "soundVolume", jobs.soundVolume },
			{ "huntRange", jobs.huntRange }, { "huntKills", jobs.huntKills }, { "huntSecondsPerPrey", jobs.huntSecondsPerPrey }, { "huntHealthWolf", jobs.huntHealthWolf },
			{ "huntHealthCat", jobs.huntHealthCat }, { "huntHealthBear", jobs.huntHealthBear }, { "huntLootFraction", jobs.huntLootFraction },
				{ "keyAttack", jobs.keyAttack }, { "attackModifier", jobs.attackModifier }, { "attackPetRange", jobs.attackPetRange }, { "attackSeconds", jobs.attackSeconds },
				{ "alert", jobs.alert }, { "alertRange", jobs.alertRange }, { "alertCooldown", jobs.alertCooldown }, { "alertScanSeconds", jobs.alertScanSeconds } };
		j["menu"] = { { "useSkyPrompt", menu.useSkyPrompt }, { "keyGather", menu.keyGather }, { "keyOther", menu.keyOther }, { "keyCancel", menu.keyCancel }, { "keyHunt", menu.keyHunt }, { "seconds", menu.seconds } };
		j["spike"] = { { "enabled", spike.enabled }, { "key", spike.key }, { "modifier", spike.modifier }, { "distance", spike.distance } };
		std::ofstream(kFile) << j.dump(2);
	}
}
