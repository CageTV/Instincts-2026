ScriptName Mumirnik_Effect_GenerateIngredients Extends activemagiceffect

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Ingredient Property IngredientToGenerate Auto
Int Property MaxIngredients Auto
Float Property UpdateTimerGameTime Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnEffectStart(Actor akTarget, Actor akCaster)
  Self.RegisterForSingleUpdateGameTime(UpdateTimerGameTime) ; #DEBUG_LINE_NO:8
EndEvent

Event OnUpdateGameTime()
  Actor targetActor = Self.GetTargetActor() ; #DEBUG_LINE_NO:12
  If targetActor.GetItemCount(IngredientToGenerate as Form) < MaxIngredients ; #DEBUG_LINE_NO:13
    targetActor.AddItem(IngredientToGenerate as Form, 1, False) ; #DEBUG_LINE_NO:14
  EndIf
  Self.RegisterForSingleUpdateGameTime(UpdateTimerGameTime) ; #DEBUG_LINE_NO:16
EndEvent
