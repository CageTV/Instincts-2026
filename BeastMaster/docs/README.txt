Instincts - The BeastMaster 0.2.1

Find, tame, feed and train the animals of Skyrim. A continuation of "Instincts - Animal Taming in Skyrim" by Kheree (Mumirnik, Nexus 63224), which is no
longer maintained. This mod contains everything in ONE plugin: Kheree's original Instincts 1.1.6 content, our fixes, and Beast Harmony (a native SKSE
plugin that adds the knock-out rule, pet jobs, danger alert, look, and the race feeding bonus).

REPLACES (do not run together with this mod, remove or disable them):
  - Instincts - Animal Taming in Skyrim.esp / .bsa  (the original mod)
  - Instincts - Fixes.esp                           (our fix plugin, 0.1.0)
  - BeastHarmony.esp                                (the Beast Harmony plugin, 0.1.0)
Install this mod instead of all three.

REQUIRES
  - Skyrim Special Edition with SKSE and Address Library for SKSE Plugins. Built and tested only on 1.7.104.0.
  - Optional: SKSE Menu Framework (settings page and the knock-out HUD icon).

SAVES: the plugin has a new name, so saves made with the original Instincts, Instincts - Fixes or Beast Harmony 0.1.0 do NOT carry over. Start a new
game or a save without any of them. Other plugins that name "Instincts - Animal Taming in Skyrim.esp" as a master or in an ini line (for example a KID
or SkyPatcher patch) must be changed to "Instincts - The BeastMaster.esp"; the FormIDs are the same as in the original.

INSTALL (Mod Organizer 2): install the archive as one mod, enable "Instincts - The BeastMaster.esp". The .bsa next to it loads with the plugin.

WHAT IS IN IT
  Instincts - The BeastMaster.esp  all records of Instincts 1.1.6 (FormIDs unchanged) + the Beast Harmony records (BH_*, 000800-00080B) + our overrides
  Instincts - The BeastMaster.bsa  Kheree's scripts and skeleton meshes (renamed so it loads with the plugin; loose scripts below override it)
  Scripts/                         our changed scripts (Mumirnik_*, TIF__05014C78), with sources in Scripts/Source/, and BeastHarmony_*.pex
  SKSE/Plugins/BeastHarmony.dll    Beast Harmony (GPL-3.0-or-later)   Interface/, Sound/ : its icon and sounds
Script and record names keep Kheree's "Mumirnik_" prefix on purpose: it is the original author's code and renaming them would only break references.

FIXES (to Instincts 1.1.6)
1. Mumirnik_Effect_SummonREFs: Summon Companions never summoned the last pet. The loop ran `While i < ActorREFs.Length - 1`; ActorREFs holds the
   three pet slots, so only two pets came. It now runs to Length.
2. Mumirnik_Quest_Instincts_PetOptions (rebuilt by tools/patch_petoptions.py from the decompiled original in reference/):
   - Taming is checked BEFORE anything is destroyed. The wild animal used to be made invisible and disintegrated first; an animal whose race or pet actor
     was missing then vanished with no pet. Now the pet is created and holds a free slot first; otherwise the animal is left alone with a message
     ("This animal cannot be tamed." / "No free pet slot.").
   - Slots instead of a bare pet count: SlotLimit 3 (property). An animal takes 1 slot, 2 if its Instincts power is 6 or more (bears, sabre cats,
     chaurus, big spiders; Animallica's lion / tiger / rhino get the same), 3 if 10 or more (mammoth). The count (PetCount) is recomputed from the pets
     themselves each time, so it cannot drift. The Taming perk's "PetCount < 3" and the Summon spell's "PetCount > 0" conditions still read right.
   - A line in Papyrus.0.log ("[Instincts+] tamed ... / released ...") for every taming and release.
(Needs nothing else: it is all in this mod.)
   - Level gate: the player needs level 20 for power 6-7 animals (bears, sabre cats, chaurus), 30 for power 8-9, 40 for power 10 (mammoth) -
     LevelBig / LevelGreat / LevelHuge properties. Ordinary animals (wolf, fox, deer, ...) have no gate.
   - Taming progress is remembered: a failed attempt keeps its accumulated chance for the same animal for 120 real seconds (RememberSeconds property),
     counted from the last feeding. A different animal or a longer pause starts again from 0; a success resets it. (tools/patch_taming_memory.py)
   - Pets eat food from their own inventory: once per game hour, right after the hunger decay, a pet at or below Hunger 50 (AutoEatBelow) eats until Satisfied, Hunger 70 (AutoEatUntil):
     food its species accepts (the feeding menu's list), one item at a time via Instincts' own ReceiveFood, up to 3 per hour (AutoEatMaxMeals), not in combat.
     AutoEatBelow = 0 switches it off. (Mumirnik_Quest_Instincts_PetStats, tools/patch_autoeat.py)
3. Mumirnik_Quest_UseFurniture (rebuilt from the decompiled original, src/Mumirnik_Quest_UseFurniture.psc): choosing "Train" on a pet only opened the training
   menu when the player had sat down on the Ferry Seat the script places. On some lists the sit never completes (sit state 2, "waiting for the sit
   animation", furniture camera stuck, no control) and the original script then deleted the seat under the player, so training was impossible. The seat is
   cosmetic: the training menu (Mumirnik_Quest_Instincts_PetTraining.Train) is only message boxes. New property SkipSitting (default True) trains without the
   seat; with SkipSitting = False the seat is kept while the engine still has the player in a sit state, the camera is put back, and the log shows
   "[InstinctsSitFix] ..." lines. Found 2026-11-10; needs only Instincts itself.
4. Kyne's Peace (VoiceKynesPeaceFactionEffect, Skyrim.esm record 06E0C1, overridden in this plugin): the shout's Faction effect only affects animals (ActorTypeAnimal), so your tamed animals were hit by it as well. One condition is added to the effect: GetPlayerTeammate == 0, so it skips your tamed animals (they are player teammates). Nothing else of the record changes (checked against the vanilla record). Idea taken from the description of Fury - Animal Taming of Skyrim ("Kyne's Peace now skips tamed animals"); the condition is our own. Untested in game.
5. Spriggan "Call of the Wild" (SprigganCallEffect, Skyrim.esm record 0CB12C, overridden in this plugin): the vanilla script sprigganCallAnimalsAOE runs on every creature the area effect touches, puts it in the Spriggan faction, sets it on the Spriggan's combat target and raises its aggression, so a tamed animal in range turned on its owner. The effect's conditions (not a dragon, an animal, in the Creature faction) get one more: GetPlayerTeammate == 0. This covers the area effect only; the other Spriggan script (sprigganCallAnimalsSCRIPT) picks up to three random creatures within 9600 units itself and is not changed. Same idea as Fury's "Spriggans can no longer turn your tamed animals against you"; the condition is ours. Untested in game.

6. Mumirnik_Quest_Instincts_Feeding.ReceiveFood (rebuilt by tools/patch_feeding_multiplier.py): the hunger a food item restores is multiplied by BeastHarmony_Feeding.Multiplier(player), which Beast Harmony's DLL answers with 1 + the feeding bonus of the player's race (Bosmer 15 % by default, config.json). The rule lives in the DLL; without it the call returns nothing and feeding is unchanged. Applies to the feeding menu and to the pet's auto-eat. Untested in game.

Checked and NOT changed: the pet feeding scripts are the same in Fury as in Instincts (ReceiveFood and ModHunger are identical, apart from Fury's Bosmer bonus), so the "excessive satiation when force feeding" fix of Fury is not in its scripts and was not ported.


BEAST HARMONY (native plugin)

KNOCK-OUT RULE. A dangerous animal that Instincts can tame is only tameable below 25 % health. With Taming Mode on (key, default backslash), the first hit by
the player that takes such an animal below the threshold decides:
  fists                          100 % knocked out
  blunt (mace, warhammer, staff, bash)   80 % knocked out, 20 % killed
  bladed (sword, axe, dagger, arrow)     50 % knocked out, 50 % killed
A knocked-out animal is paralysed and calm for 60 s and cannot be finished off by stray hits; tame it while it is down. Spells and traps never knock out.
Gates: the player level for strong animals (Instincts power 6-7: 20, 8-9: 30, 10: 40) and a free pet slot (strong animals take 2-3 of 3 pet slots).
Taming Mode off (the default after loading) leaves hunting untouched.

Config: Data/SKSE/Plugins/BeastHarmony/config.json ("mode": hotkey / always / never, "toggleKey", chances, threshold, seconds, levels, slotLimit).
Log: BeastHarmony.log (one line per roll).

Credits:
Knock-out indicator icon: "classes beastmaster" by Maxicons from Noun Project (https://thenounproject.com/icon/classes-beastmaster-2360013/), used under the Noun Project free licence (CC BY 3.0).

FEEDING BONUS BY RACE (BeastHarmony_Feeding.Multiplier, src/Feeding.cpp). The hunger a food item restores for a pet is multiplied by 1 + the bonus of the player's race. Bosmer get 15 % more per item by default (the Wood Elf race and its vampire variant); the table is config.json "feeding" {"enabled", "races": [{"name", "file", "id", "bonus"}]}. The rule lives in the DLL; the patched Instincts feeding script only multiplies with the result, and without this DLL feeding is unchanged. It applies to feeding in the gift menu and to the pet eating from its own inventory. Log: "feeding: multiplier ..." in BeastHarmony.log.


LICENCE: BeastHarmony.dll and the Beast Harmony source are GPL-3.0-or-later (SKSE/Plugins/BeastHarmony-LICENSE.txt). Kheree's original scripts and
assets are used under the permissions of the original mod. See NOTICE.txt. Source: https://github.com/CageTV/Instincts-2026

CREDITS: Instincts by Kheree (Mumirnik); Fixes, Beast Harmony and this release by CageTV; knock-out icon "classes beastmaster" by Maxicons (Noun Project, CC BY 3.0);
SKSE Menu Framework header by marth (MIT); CommonLibSSE-NG by alandtse.
