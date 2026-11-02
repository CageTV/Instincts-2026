"""Pets eat food from their own inventory when they get hungry.

Instincts decays each pet's Hunger (0..100, higher = better fed) once per game hour in Mumirnik_Quest_Instincts_PetStats.UpdatePetStatsGameTime. Food only
counted when the player opened the feeding menu. Now, right after that hourly decay, a pet whose Hunger is at or below AutoEatBelow (default 50) eats
food from its own inventory: items its species accepts (the same food list the feeding menu uses), one at a time through Instincts' own ReceiveFood
(eat sound, hunger gain, message, item removed), up to AutoEatMaxMeals per hour or until it reaches AutoEatUntil (70 = Satisfied). A pet in combat does not eat.
Set AutoEatBelow to 0 to switch it off.

Reads reference/mumirnik_quest_instincts_petstats.psc, writes src/Mumirnik_Quest_Instincts_PetStats.psc.
"""
import os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ref = os.path.join(HERE, "reference", "mumirnik_quest_instincts_petstats.psc")
out = os.path.join(HERE, "src", "Mumirnik_Quest_Instincts_PetStats.psc")
s = open(ref, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def swap(old, new):
    global s
    assert s.count(old) == 1, ("not found exactly once", old[:70])
    s = s.replace(old, new, 1)


swap("Message[] Property PetStatisticsMessageM Auto\n",
     "Message[] Property PetStatisticsMessageM Auto\n"
     "Float Property AutoEatBelow = 50.0 Auto\n"
     "{ A pet at or below this Hunger (0-100) eats food from its own inventory once per game hour. 0 = never. }\n"
     "Float Property AutoEatUntil = 70.0 Auto\n"
     "{ Once it starts, a pet keeps eating until its Hunger reaches this (70 = Satisfied, 90 = Well Fed). }\n"
     "Int Property AutoEatMaxMeals = 5 Auto\n"
     "{ Most food items a pet eats in one go. }\n")

swap("  Self.ModHunger(akTarget, -hungerDecay) ; #DEBUG_LINE_NO:61\n",
     "  Self.ModHunger(akTarget, -hungerDecay) ; #DEBUG_LINE_NO:61\n"
     "  Self.AutoEat(akTarget)\n")

swap("Function SetHunger(Actor akTarget, Float aiValue)\n",
     "Function AutoEat(Actor akTarget)\n"
     "{ A hungry pet eats food it carries. Uses the same species food list as the feeding menu. }\n"
     "  If AutoEatBelow <= 0.0 || akTarget == None || akTarget.IsDead() || akTarget.IsInCombat()\n"
     "    Return\n"
     "  EndIf\n"
     "  Quest q = Self as Quest\n"
     "  Int raceId = (q as mumirnik_quest_instincts_petoptions).NewRaceList.Find(akTarget.GetRace() as Form)\n"
     "  If raceId < 0\n"
     "    Return\n"
     "  EndIf\n"
     "  GlobalVariable foodType = (q as mumirnik_quest_instincts_feeding).FoodTypes.GetAt(raceId) as GlobalVariable\n"
     "  If foodType == None\n"
     "    Return\n"
     "  EndIf\n"
     "  FormList foodList = (q as mumirnik_quest_instincts_feeding).FoodSublists.GetAt(foodType.GetValue() as Int) as FormList\n"
     "  If foodList == None\n"
     "    Return\n"
     "  EndIf\n"
     "  Int meals = 0\n"
     "  If akTarget.GetActorValue(HungerAVName) > AutoEatBelow\n"
     "    Return\n"
     "  EndIf\n"
     "  While meals < AutoEatMaxMeals && akTarget.GetActorValue(HungerAVName) < AutoEatUntil\n"
     "    Form food = None\n"
     "    Int i = akTarget.GetNumItems()\n"
     "    While i > 0 && food == None\n"
     "      i -= 1\n"
     "      Form item = akTarget.GetNthForm(i)\n"
     "      If item != None && foodList.HasForm(item)\n"
     "        food = item\n"
     "      EndIf\n"
     "    EndWhile\n"
     "    If food == None\n"
     "      Return\n"
     "    EndIf\n"
     "    (q as mumirnik_quest_instincts_feeding).ReceiveFood(akTarget, food, 1)\n"
     "    meals += 1\n"
     "  EndWhile\n"
     "EndFunction\n"
     "\n"
     "Function SetHunger(Actor akTarget, Float aiValue)\n")

open(out, "w", encoding="utf-8", newline="").write(s)
print("wrote", out)
