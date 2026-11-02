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
