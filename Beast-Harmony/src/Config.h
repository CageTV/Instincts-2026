/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
		bool    copyLook = true;  // the pet wears the wild animal's Dynamic Animal Variants skin
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

	// what a pet gets out of food: the hunger a food item restores is multiplied by 1 + the bonus of the player's race (Feeding.h)
	struct RaceBonus
	{
		std::string   name;     // only for the config file and the log
		std::string   file;     // plugin that defines the race
		std::uint32_t localId;  // form id inside that plugin
		float         bonus;    // 0.15 = 15 % more hunger per item
	};

	struct Feeding
	{
		bool                  enabled = true;
		std::vector<RaceBonus> races{
			{ "Bosmer", "Skyrim.esm", 0x013749, 0.15f },
			{ "Bosmer (vampire)", "Skyrim.esm", 0x088884, 0.15f },
		};
	};

	// Harbinger test (see Spike.h): off by default
	struct Spike
	{
		bool  enabled = false;
		int   key = 0x42;       // DirectInput scan code (0x42 = F8)
		int   modifier = 0;     // 0 = none; 42 left shift, 29 left ctrl, 56 left alt
		float distance = 600.0f;
	};

	// Pet jobs (Gather ...). Off by default; the key starts a gather trip for the pet under the crosshair.
	struct Jobs
	{
		bool  enabled = false;
		int   key = 0x0B;          // DirectInput scan code (0x0B = the 0 key; F2 = 60, F3 = 61; NOT F5/F9, the game uses them)
		int   modifier = 0;        // 0 = none; 42 left shift, 29 left ctrl, 56 left alt
		float range = 2800.0f;     // game units from the pet it may roam to find plants (40 m)
		int   maxItems = 3;        // plants picked per trip
		float gameHours = 1.0f;    // the pet stays away at least this many game hours
		float arriveRadius = 150.0f;
		bool  sound = true;        // the animal makes its own sound when it is back
		float soundVolume = 1.0f;  // 0..1
		// Hunt: a fighter pet kills wild game (unaggressive animals nobody owns), takes what the carcass holds, then walks back to the player.
		float huntRange = 3500.0f;         // how far from the pet it looks for prey
		int   huntKills = 1;               // kills per trip
		float huntSecondsPerPrey = 90.0f;  // real seconds it chases one animal before giving up on it
		float huntHealthWolf = 45.0f;      // largest prey (health) per species: wolves small game,
		float huntHealthCat = 130.0f;      //   big cats small to medium game,
		float huntHealthBear = 300.0f;     //   bears any size
		float huntLootFraction = 0.25f;    // the pet takes at most this share of its carry weight from a carcass
		// Attack my target: the key sends every nearby fighter pet at the creature under the crosshair
		int   keyAttack = 0x22;            // 0x22 = G
		int   attackModifier = 0;
		float attackPetRange = 4000.0f;    // only pets this close to the player answer
		float attackSeconds = 60.0f;       // it stops chasing after this many real seconds
		// Danger alert: a tamed wolf near the player warns (its sound + a HUD line with the direction) when a hostile creature or person is near it
		bool  alert = true;
		float alertRange = 3000.0f;        // how far from the wolf it notices (about 43 m)
		float alertCooldown = 40.0f;       // real seconds before the same one is announced again
		float alertScanSeconds = 2.0f;
		float alertMinHealth = 40.0f;      // ignores creatures with less health than this (rats, mudcrabs, skeevers); 0 = warn of everything
	};

	// the pet command list ("Do something for me"): SkyPrompt prompts when SkyPrompt is installed, else a message box
	// On-screen indicator that the knock-out (Taming Mode) is active; x / y are the top-left corner as fractions of the screen.
	struct Widget
	{
		bool  enabled = true;
		float x = 0.02f;
		float y = 0.30f;
		float scale = 1.0f;
	};

	struct Menu
	{
		bool  useSkyPrompt = true;
		int   keyGather = 0x2C;   // DirectInput scan codes: 0x2C = Z, 0x2D = X, 0x2E = C
		int   keyOther = 0x2D;
		int   keyCancel = 0x2E;
		int   keyHunt = 0x2F;     // 0x2F = V
		float seconds = 20.0f;    // the prompts go away by themselves after this long
	};

	Subdue& Get();
	Pets&   GetPets();
	Box&    GetBox();
	Diet&   GetDiet();
	Feeding& GetFeeding();
	Spike&  GetSpike();
	Jobs&   GetJobs();
	Menu&   GetMenu();
	Widget& GetWidget();
	void    Load();
	void    Save();
}
