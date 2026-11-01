ScriptName Mumirnik_Effect_GeneratePotions Extends activemagiceffect

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Int Property MaxPotions Auto
Potion Property PotionToGenerate Auto
Float Property UpdateTimerGameTime Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnEffectStart(Actor akTarget, Actor akCaster)
  Self.RegisterForSingleUpdateGameTime(UpdateTimerGameTime) ; #DEBUG_LINE_NO:8
EndEvent

Event OnUpdateGameTime()
  Actor targetActor = Self.GetTargetActor() ; #DEBUG_LINE_NO:12
  If targetActor.GetItemCount(PotionToGenerate as Form) < MaxPotions ; #DEBUG_LINE_NO:13
    targetActor.AddItem(PotionToGenerate as Form, 1, False) ; #DEBUG_LINE_NO:14
  EndIf
  Self.RegisterForSingleUpdateGameTime(UpdateTimerGameTime) ; #DEBUG_LINE_NO:16
EndEvent
