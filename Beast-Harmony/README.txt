Beast Harmony 0.1.0 (SKSE plugin + BeastHarmony.esp, ESL) - companion of Instincts - Animal Taming in Skyrim (Nexus 63224, Kheree).

KNOCK-OUT RULE. A dangerous animal that Instincts can tame is only tameable below 25 % health. With Taming Mode on (key, default backslash), the first hit by
the player that takes such an animal below the threshold decides:
  fists                          100 % knocked out
  blunt (mace, warhammer, staff, bash)   80 % knocked out, 20 % killed
  bladed (sword, axe, dagger, arrow)     50 % knocked out, 50 % killed
A knocked-out animal is paralysed and calm for 60 s and cannot be finished off by stray hits; tame it while it is down. Spells and traps never knock out.
Gates: the player level for strong animals (Instincts power 6-7: 20, 8-9: 30, 10: 40) and a free pet slot (Instincts - Fixes: strong animals take 2-3 of 3 slots).
Taming Mode off (the default after loading) leaves hunting untouched.

Config: Data/SKSE/Plugins/BeastHarmony/config.json ("mode": hotkey / always / never, "toggleKey", chances, threshold, seconds, levels, slotLimit).
Log: BeastHarmony.log (one line per roll).
Needs: Instincts - Animal Taming in Skyrim; Instincts - Fixes (this project) for the slot cost.

Credits:
Knock-out indicator icon: "classes beastmaster" by Maxicons from Noun Project (https://thenounproject.com/icon/classes-beastmaster-2360013/), used under the Noun Project free licence (CC BY 3.0).

FEEDING BONUS BY RACE (BeastHarmony_Feeding.Multiplier, src/Feeding.cpp). The hunger a food item restores for a pet is multiplied by 1 + the bonus of the player's race. Bosmer get 15 % more per item by default (the Wood Elf race and its vampire variant); the table is config.json "feeding" {"enabled", "races": [{"name", "file", "id", "bonus"}]}. The rule lives in the DLL; the patched Instincts feeding script (Instincts - Fixes) only multiplies with the result, and without this DLL feeding is unchanged. It applies to feeding in the gift menu and to the pet eating from its own inventory. Log: "feeding: multiplier ..." in BeastHarmony.log.

LICENCE: GPL-3.0-or-later (LICENSE). The source headers say so; third-party headers keep their own licences (SKSEMenuFramework.h, APMF_API.h).
