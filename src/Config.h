/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

#include <string>

namespace Config
{
	struct Subdue
	{
		bool        enabled = true;
		std::string mode = "hotkey";  // "hotkey": only while Taming Mode is switched on with the key; "always"; "never"
		int         toggleKey = 0x2B; // DirectInput scan code of the Taming Mode key (0x2B = backslash)
		float       threshold = 0.25f;     // health fraction at which the animal is knocked out or killed
		float       seconds = 60.0f;       // how long a knocked-out animal stays down
		float       chanceFists = 1.0f;    // chance to knock out (instead of kill) with bare hands
		float       chanceBlunt = 0.8f;    // mace, warhammer, staff, shield / bow bash
		float       chanceBladed = 0.5f;   // swords, axes, daggers, arrows
		int         levelBig = 20;         // player level for animals of power 6-7
		int         levelGreat = 30;       // ... 8-9
		int         levelHuge = 40;        // ... 10 and more
		int         slotLimit = 3;
	};

	Subdue& Get();
	void    Load();
	void    Save();
}
