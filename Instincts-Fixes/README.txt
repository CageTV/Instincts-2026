Instincts - Fixes (for Instincts - Animal Taming in Skyrim 1.1.6, Nexus 63224, by Kheree; the mod's permissions allow modification)

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
Needs only Instincts itself. Remove the folder and everything is as Kheree made it (pets made meanwhile keep working: the count is recomputed).
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
4. Kyne's Peace (VoiceKynesPeaceFactionEffect, Skyrim.esm record 06E0C1, overridden in Instincts - Fixes.esp): the shout's Faction effect only affects animals (ActorTypeAnimal), so your tamed animals were hit by it as well. One condition is added to the effect: GetPlayerTeammate == 0, so it skips your tamed animals (they are player teammates). Nothing else of the record changes (checked against the vanilla record). Idea taken from the description of Fury - Animal Taming of Skyrim ("Kyne's Peace now skips tamed animals"); the condition is our own. Untested in game.
