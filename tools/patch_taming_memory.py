"""Taming progress is remembered for a couple of minutes.

Instincts adds every food item's value to TameChance, rolls once, and on a failed roll sets TameChance back to 0. So feeding an animal to 50% and
walking away meant starting again from 0. Now a failed roll keeps the total for the SAME animal for RememberSeconds of real time (counted from the
last feeding); a different animal, or a longer pause, starts from 0 again. A success still resets it.

Run after patch_petoptions.py (it edits src/Mumirnik_Quest_Instincts_PetOptions.psc in place).
"""
import os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
path = os.path.join(HERE, "src", "Mumirnik_Quest_Instincts_PetOptions.psc")
s = open(path, encoding="utf-8", newline="").read()
crlf = "\r\n" in s
s = s.replace("\r\n", "\n")


def swap(old, new):
    global s
    assert s.count(old) == 1, ("not found exactly once", old[:60])
    s = s.replace(old, new, 1)


if "RememberedAnimal" in s:
    raise SystemExit("already applied")

swap("Actor AnimalToTame\nFloat TameChance = 0.0\n",
     "Actor AnimalToTame\nFloat TameChance = 0.0\nActor RememberedAnimal\nFloat RememberedAt = 0.0\n")

swap("Int Property SlotLimit = 3 Auto\n",
     "Float Property RememberSeconds = 120.0 Auto\n"
     "{ A failed taming attempt keeps its accumulated chance for this many real seconds (counted from the last feeding) for the same animal. }\n"
     "Int Property SlotLimit = 3 Auto\n")

swap("  AnimalToTame = akTarget ; #DEBUG_LINE_NO:135\n  TameChance += aiValue ; #DEBUG_LINE_NO:136\n",
     "  Float now = Utility.GetCurrentRealTime()\n"
     "  If akTarget != RememberedAnimal || now < RememberedAt || now - RememberedAt > RememberSeconds\n"
     "    TameChance = 0.0 ; another animal, a long pause, or a new session: start from nothing\n"
     "  EndIf\n"
     "  RememberedAnimal = akTarget\n"
     "  RememberedAt = now\n"
     "  AnimalToTame = akTarget ; #DEBUG_LINE_NO:135\n"
     "  TameChance += aiValue ; #DEBUG_LINE_NO:136\n")

swap("    If Utility.RandomInt(0, 99) as Float < TameChance ; #DEBUG_LINE_NO:143\n      Self.MakePet(AnimalToTame) ; #DEBUG_LINE_NO:144\n",
     "    If Utility.RandomInt(0, 99) as Float < TameChance ; #DEBUG_LINE_NO:143\n      Self.MakePet(AnimalToTame) ; #DEBUG_LINE_NO:144\n"
     "      TameChance = 0.0\n      RememberedAnimal = None\n")

swap("    AnimalToTame = None ; #DEBUG_LINE_NO:148\n    TameChance = 0.0 ; #DEBUG_LINE_NO:149\n",
     "    AnimalToTame = None ; #DEBUG_LINE_NO:148\n")

open(path, "w", encoding="utf-8", newline="").write(s.replace("\n", "\r\n") if crlf else s)
print("patched", path)
