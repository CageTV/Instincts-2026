ScriptName PRKF_Mumirnik_Instincts_Perk_050332F9 Extends Perk hidden

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Message Property ChanceToTameMessageHELP Auto
Quest Property PetOptionsQuest Auto
ReferenceAlias Property TemporaryREF Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Fragment_0(ObjectReference akTargetRef, Actor akActor)
  Actor thisActor = akTargetRef as Actor ; #DEBUG_LINE_NO:10
  Bool meetsPowerBudget = (PetOptionsQuest as mumirnik_quest_instincts_petoptions).CheckPowerBudget(thisActor) ; #DEBUG_LINE_NO:11
  If meetsPowerBudget ; #DEBUG_LINE_NO:12
    Utility.Wait(0.02) ; #DEBUG_LINE_NO:13
    FormList foodListForRace = (PetOptionsQuest as mumirnik_quest_instincts_feeding).GetFoodListForOriginalRace(thisActor) ; #DEBUG_LINE_NO:14
    If foodListForRace ; #DEBUG_LINE_NO:15
      TemporaryREF.ForceRefTo(thisActor as ObjectReference) ; #DEBUG_LINE_NO:16
      thisActor.AddInventoryEventFilter(foodListForRace as Form) ; #DEBUG_LINE_NO:17
      thisActor.ShowGiftMenu(True, foodListForRace, True, False) ; #DEBUG_LINE_NO:18
      Utility.Wait(0.01) ; #DEBUG_LINE_NO:19
      TemporaryREF.Clear() ; #DEBUG_LINE_NO:20
    EndIf
  EndIf
EndFunction

Function Fragment_19(ObjectReference akTargetRef, Actor akActor)
  Actor thisActor = akTargetRef as Actor ; #DEBUG_LINE_NO:32
  Bool meetsPowerBudget = (PetOptionsQuest as mumirnik_quest_instincts_petoptions).CheckPowerBudget(thisActor) ; #DEBUG_LINE_NO:33
  If meetsPowerBudget ; #DEBUG_LINE_NO:34
    Utility.Wait(0.02) ; #DEBUG_LINE_NO:35
    FormList foodListForRace = (PetOptionsQuest as mumirnik_quest_instincts_feeding).GetFoodListForOriginalRace(thisActor) ; #DEBUG_LINE_NO:36
    If foodListForRace ; #DEBUG_LINE_NO:37
      TemporaryREF.ForceRefTo(thisActor as ObjectReference) ; #DEBUG_LINE_NO:38
      thisActor.AddInventoryEventFilter(foodListForRace as Form) ; #DEBUG_LINE_NO:39
      thisActor.ShowGiftMenu(True, foodListForRace, True, False) ; #DEBUG_LINE_NO:40
      Utility.Wait(0.01) ; #DEBUG_LINE_NO:41
      TemporaryREF.Clear() ; #DEBUG_LINE_NO:42
    EndIf
  EndIf
EndFunction
