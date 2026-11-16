"""Feeding takes the player's race into account: the hunger a food item restores is multiplied by BeastHarmony_Feeding.Multiplier(player).

The multiplier is decided in BeastHarmony.dll (1.0 + the feeding bonus of the player's race, Bosmer 15 % by default, table in config.json), not in a script;
this patch only multiplies with it. Without the DLL (or its BeastHarmony_Feeding.pex) the call returns nothing, the result is not above 0, and feeding is unchanged.

Reads reference/mumirnik_quest_instincts_feeding.psc, writes src/Mumirnik_Quest_Instincts_Feeding.psc. Used by the menu feeding, the auto-eat from the pet's
own inventory and every other caller of ReceiveFood.
"""
import os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ref = os.path.join(HERE, "reference", "mumirnik_quest_instincts_feeding.psc")
out = os.path.join(HERE, "src", "Mumirnik_Quest_Instincts_Feeding.psc")
s = open(ref, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = "  itemValue *= aiCount as Float ; #DEBUG_LINE_NO:"
assert s.count(old) == 1, "ReceiveFood line not found exactly once"
i = s.index(old); j = s.index("\n", i)
line = s[i:j]
new = (line + "\n"
       "  Float feedMultiplier = BeastHarmony_Feeding.Multiplier(Game.GetPlayer())\n"
       "  If feedMultiplier > 0.0\n"
       "    itemValue *= feedMultiplier\n"
       "  EndIf")
s = s[:i] + new + s[j:]
open(out, "w", encoding="utf-8", newline="").write(s)
print("wrote", out)
