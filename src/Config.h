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

	// combat behaviour of a pet class (engine values): aggression 0 unaggressive, 1 aggressive, 2 very aggressive, 3 frenzied;
	// confidence 0 cowardly, 1 cautious, 2 average, 3 brave, 4 foolhardy; assistance 0 helps nobody, 1 helps allies, 2 helps friends and allies
	struct ClassAI
	{
		int aggression;
		int confidence;
		int assistance;
	};

	struct Pets
	{
		bool    enabled = true;
		float   scanSeconds = 2.0f;
		ClassAI fighter{ 1, 3, 2 };
		ClassAI scavenger{ 1, 2, 1 };
		ClassAI gatherer{ 0, 0, 0 };
		ClassAI producer{ 0, 0, 0 };
	};

	// the "What would you like to do? Attempt to Tame / Activate" box of Instincts' Taming perk
	struct Box
	{
		bool skip = true;     // hide the box and run "Attempt to Tame" directly
		int  delayFrames = 3; // frames to wait between hiding the box and opening the feeding menu
		bool label = true;    // show "Attempt to Tame" under the crosshair on wild animals that can be tamed
	};

	// what the tamed animals accept as food (Instincts' food lists are trimmed at game load)
	struct Diet
	{
		bool enabled = true;
	};

	Subdue& Get();
	Pets&   GetPets();
	Box&    GetBox();
	Diet&   GetDiet();
	void    Load();
	void    Save();
}
