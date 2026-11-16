"""Builds src/Mumirnik_Quest_Instincts_PetOptions.psc from the decompiled original (reference/): slot cost, validate-before-destroy taming, self-healing pet count.

Usage: python tools/patch_petoptions.py
"""
import os
import re

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = open(os.path.join(HERE, "reference", "mumirnik_quest_instincts_petoptions.psc"), encoding="utf-8").read().replace("\r\n", "\n")


def cut(text, start_marker, end_marker):
    a = text.index(start_marker)
    b = text.index(end_marker, a + 1)
    return a, b


# ---- properties -----------------------------------------------------------------------------------------------------------------
props = '''Int Property SlotLimit = 3 Auto
{ Total pet slots (a big animal takes more than one). Was a hard-coded 3. }
Int Property BigPower = 6 Auto
{ An animal of this power or more takes 2 slots. }
Int Property HugePower = 10 Auto
{ An animal of this power or more takes 3 slots. }
Int Property LevelBig = 20 Auto
{ Player level needed to tame an animal of BigPower (6) or more. }
Int Property LevelGreat = 30 Auto
{ Player level needed to tame an animal of power 8 or more. }
Int Property LevelHuge = 40 Auto
{ Player level needed to tame an animal of HugePower (10) or more. }

;-- Functions ---------------------------------------
'''
src = src.replace(";-- Functions ---------------------------------------\n", props, 1)

# ---- CheckPowerBudget: slot cost instead of "== 3" -----------------------------------------------------------------------------------
old_start = src.index("  ElseIf PetCount.GetValue() == 3 as Float")
old_end = src.index("EndFunction", old_start)
new = '''  EndIf
  Int needLevel = Self.LevelNeededForPower(power)
  If PlayerREF.GetLevel() < needLevel
    Debug.Notification("You are not experienced enough to tame this animal (level " + needLevel + ").")
    Return False
  EndIf
  Int cost = Self.SlotCostForPower(power)
  Self.SyncPetCount()
  If (PetCount.GetValue() as Int) + cost > Self.GetSlotLimit()
    PetCountCheckFAILED.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
    If cost > 1
      Debug.Notification("This animal needs " + cost + " pet slots (" + (Self.GetSlotLimit() - (PetCount.GetValue() as Int)) + " free).")
    EndIf
    Return False
  EndIf
  Return True
'''
src = src[:old_start] + new + src[old_end:]


old = """  Race originalRace = akTarget.GetRace() ; #DEBUG_LINE_NO:111
  Int originalRaceId = Self.GetRaceIdForRace(originalRace) ; #DEBUG_LINE_NO:112
  Int power = Self.GetPowerForRaceId(originalRaceId) ; #DEBUG_LINE_NO:113
  If originalRaceId == -1 || power == -1 ; #DEBUG_LINE_NO:114
    Debug.Notification("Error: Invalid original race") ; #DEBUG_LINE_NO:115
  EndIf"""
new = """  Race originalRace = akTarget.GetRace()
  Int originalRaceId = OriginalRaceList.Find(originalRace as Form)
  If originalRaceId < 0
    Debug.Notification("This animal cannot be tamed.")
    Return False
  EndIf
  Int power = Self.GetPowerForRaceId(originalRaceId)"""
assert old in src
src = src.replace(old, new, 1)

# ---- MakePet ------------------------------------------------------------------------------------------------------------------------
a, b = cut(src, "Function MakePet(Actor akTarget)", "Function ReleasePet(Actor akTarget)")
make_pet = '''Function MakePet(Actor akTarget)
{ Makes the target your pet. The wild animal is only removed AFTER the pet exists in a free slot: an animal that cannot be a pet is left alone. }
  If !akTarget || akTarget.IsDead()
    Return
  EndIf
  Race originalRace = akTarget.GetRace()
  Int originalRaceId = OriginalRaceList.Find(originalRace as Form)
  If originalRaceId < 0
    Debug.Notification("This animal cannot be tamed.")
    Debug.Trace("[Instincts+] MakePet refused: race not on the tamable list: " + originalRace)
    Return
  EndIf
  Bool isForcedRandomGenderMaleVisuals = ForcedRandomGenderMaleVisualsRaceList.find(originalRace, 0) != -1
  Int gender = 0
  If ForcedFemaleRaceList.find(originalRace, 0) != -1
    gender = 2
  ElseIf ForcedMaleRaceList.find(originalRace, 0) != -1
    gender = 1
  ElseIf ForcedRandomGenderRaceList.find(originalRace, 0) != -1 || isForcedRandomGenderMaleVisuals
    gender = Utility.RandomInt(1, 2)
  Else
    gender = akTarget.GetLeveledActorBase().GetSex() + 1
  EndIf
  ActorBase newActorBase = None
  If gender == 1 || isForcedRandomGenderMaleVisuals
    newActorBase = PetActorListM.GetAt(originalRaceId) as ActorBase
  ElseIf gender == 2
    newActorBase = PetActorListF.GetAt(originalRaceId) as ActorBase
  EndIf
  ActorBase targetActorBase = akTarget.GetLeveledActorBase()
  Int raceSubstituteActorBaseId = RaceSubstituteOriginalActorBaseList.find(targetActorBase, 0)
  If raceSubstituteActorBaseId == -1
    targetActorBase = akTarget.GetBaseObject() as ActorBase
    raceSubstituteActorBaseId = RaceSubstituteOriginalActorBaseList.find(targetActorBase, 0)
  EndIf
  Int targetActorBaseId = targetActorBase.GetFormID()
  If targetActorBaseId == 67348929 && originalRace.GetFormID() == 78346
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 271029 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 271024
    Int originalRaceFormID = originalRace.GetFormID()
    If originalRaceFormID == 78328
      raceSubstituteActorBaseId = 5
    ElseIf originalRaceFormID == 341111
      raceSubstituteActorBaseId = 3
    EndIf
  ElseIf targetActorBaseId == 124835 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 416315 && originalRace.GetFormID() == 78346 && akTarget.GetLevel() >= 6
    raceSubstituteActorBaseId = 9
  ElseIf targetActorBaseId == 969066 && originalRace.GetFormID() == 1088636
    raceSubstituteActorBaseId = 0
  EndIf
  If raceSubstituteActorBaseId != -1
    If gender == 1 || isForcedRandomGenderMaleVisuals
      newActorBase = RaceSubstituteActorListM[raceSubstituteActorBaseId]
      originalRaceId = PetActorListM.Find(newActorBase as Form)
    ElseIf gender == 2
      newActorBase = RaceSubstituteActorListF[raceSubstituteActorBaseId]
      originalRaceId = PetActorListF.Find(newActorBase as Form)
    EndIf
  EndIf
  If !newActorBase || originalRaceId < 0
    Debug.Notification("This animal cannot be tamed.")
    Debug.Trace("[Instincts+] MakePet refused: no pet actor for " + targetActorBase + " (race " + originalRace + ", id " + originalRaceId + ")")
    Return
  EndIf
  Int slot = -1
  Int s = 0
  While s < PetREF.Length && slot == -1
    If PetREF[s].GetReference() == None
      slot = s
    EndIf
    s += 1
  EndWhile
  If slot == -1
    Debug.Notification("No free pet slot.")
    Debug.Trace("[Instincts+] MakePet refused: no free slot")
    Return
  EndIf
  Actor petActor = akTarget.PlaceActorAtMe(newActorBase, 4, None)
  If !petActor
    Debug.Trace("[Instincts+] MakePet failed: could not place " + newActorBase)
    Return
  EndIf
  petActor.MoveTo(akTarget as ObjectReference, 0.0, 0.0, 0.0, True)
  PetREF[slot].ForceRefTo(petActor as ObjectReference)
  ; the pet exists and holds a slot: now the wild animal can go
  akTarget.SetAlpha(0.0, True)
  akTarget.SetCriticalStage(akTarget.CritStage_DisintegrateEnd)
  TamedSound.Play(petActor as ObjectReference)
  akTarget.Kill(PlayerREF)
  petActor.StopCombat()
  petActor.SetPlayerTeammate(True, False)
  petActor.EvaluatePackage()
  TamedMessage[slot].Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
  petActor.RestoreActorValue("Health", 10000 as Float)
  ((Self as Quest) as mumirnik_quest_instincts_petstats).SetHunger(petActor, 50 as Float)
  ((Self as Quest) as mumirnik_quest_instincts_pettraining).InitTrainingProgression(petActor, originalRaceId)
  Self.SetBaseNameDisplay(petActor, gender)
  Int power = Self.GetPowerForRaceId(originalRaceId)
  PetPower.Mod(power as Float)
  petActor.SetActorValue(PowerAVName, power as Float)
  Self.SyncPetCount()
  Debug.Trace("[Instincts+] tamed " + newActorBase + " in slot " + slot + " (power " + power + ", slots in use " + (PetCount.GetValue() as Int) + "/" + Self.GetSlotLimit() + ")")
  If PetPower.GetValue() > PetPowerMax.GetValue()
    Debug.MessageBox("Error: Pet power overflow, this should not happen at this point! Releasing new pet")
    Self.ReleasePet(petActor)
    Return
  EndIf
  Game.AdvanceSkill("Speechcraft", SpeechcraftExperienceGainTaming.GetValue())
  If !TimerIsRunning
    TimerIsRunning = True
    Self.RegisterForSingleUpdateGameTime(1.0)
  EndIf
EndFunction

'''
src = src[:a] + make_pet + src[b:]

# ---- ReleasePet: recount instead of "-1" ------------------------------------------------------------------------------------------------
old = "  PetCount.Mod(-1 as Float) ; #DEBUG_LINE_NO:306\n"
assert old in src
src = src.replace(old, "  Self.SyncPetCount()\n  Debug.Trace(\"[Instincts+] released a pet; slots in use \" + (PetCount.GetValue() as Int) + \"/\" + SlotLimit)\n", 1)

# ---- helper functions, before the game-time event -----------------------------------------------------------------------------------------
helpers = '''Int Function GetSlotLimit()
  If SlotLimit < 1
    Return 3
  EndIf
  Return SlotLimit
EndFunction

Int Function GetBigPower()
  If BigPower < 1
    Return 6
  EndIf
  Return BigPower
EndFunction

Int Function GetHugePower()
  If HugePower < 1
    Return 10
  EndIf
  Return HugePower
EndFunction

Int Function LevelNeededForPower(Int aiPower)
{ Player level needed: nothing for ordinary animals, more and more for the strong ones. }
  Int big = LevelBig
  Int great = LevelGreat
  Int huge = LevelHuge
  If big < 1
    big = 20
  EndIf
  If great < 1
    great = 30
  EndIf
  If huge < 1
    huge = 40
  EndIf
  If aiPower >= Self.GetHugePower()
    Return huge
  ElseIf aiPower >= 8
    Return great
  ElseIf aiPower >= Self.GetBigPower()
    Return big
  EndIf
  Return 0
EndFunction

Int Function SlotCostForPower(Int aiPower)
{ Slots an animal takes: 1 normally, 2 for strong ones (bears, big cats), 3 for the biggest (mammoth). }
  If aiPower >= Self.GetHugePower()
    Return 3
  ElseIf aiPower >= Self.GetBigPower()
    Return 2
  EndIf
  Return 1
EndFunction

Int Function SlotsInUse()
{ Slots held by the pets that exist now (read from the pets themselves, so it can never drift). }
  Int used = 0
  Int i = 0
  While i < PetREF.Length
    Actor pet = PetREF[i].GetReference() as Actor
    If pet
      used += Self.SlotCostForPower(pet.GetActorValue(PowerAVName) as Int)
    EndIf
    i += 1
  EndWhile
  Return used
EndFunction

Function SyncPetCount()
  PetCount.SetValue(Self.SlotsInUse() as Float)
EndFunction

'''
marker = "Event OnUpdateGameTime()"
assert marker in src
src = src.replace(marker, helpers + marker, 1)

out = os.path.join(HERE, "src", "Mumirnik_Quest_Instincts_PetOptions.psc")
open(out, "w", encoding="utf-8", newline="\n").write(src)
print("wrote", out)
