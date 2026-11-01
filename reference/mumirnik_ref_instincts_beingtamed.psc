ScriptName Mumirnik_REF_Instincts_BeingTamed Extends ReferenceAlias

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Float Property GoldToTamingStrengthIngredientAdd Auto
Float Property GoldToTamingStrengthIngredientMult Auto
Float Property GoldToTamingStrengthPotionAdd Auto
Float Property GoldToTamingStrengthPotionMult Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnItemAdded(Form akBaseItem, Int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
  Actor thisActor = Self.GetActorReference() ; #DEBUG_LINE_NO:12
  Quest owningQuest = Self.GetOwningQuest() ; #DEBUG_LINE_NO:13
  Sound eatSound = (owningQuest as mumirnik_quest_instincts_feeding).eatSound ; #DEBUG_LINE_NO:15
  eatSound.Play(thisActor as ObjectReference) ; #DEBUG_LINE_NO:16
  Float itemValue = 0 as Float ; #DEBUG_LINE_NO:18
  If akBaseItem as Ingredient ; #DEBUG_LINE_NO:19
    itemValue = (akBaseItem.GetGoldValue() as Float * GoldToTamingStrengthIngredientMult) + GoldToTamingStrengthIngredientAdd ; #DEBUG_LINE_NO:20
  ElseIf akBaseItem as Potion ; #DEBUG_LINE_NO:21
    itemValue = (akBaseItem.GetGoldValue() as Float * GoldToTamingStrengthPotionMult) + GoldToTamingStrengthPotionAdd ; #DEBUG_LINE_NO:22
  Else
    itemValue = akBaseItem.GetGoldValue() as Float ; #DEBUG_LINE_NO:24
  EndIf
  itemValue *= aiItemCount as Float ; #DEBUG_LINE_NO:26
  (owningQuest as mumirnik_quest_instincts_petoptions).TryMakePet(thisActor, itemValue) ; #DEBUG_LINE_NO:28
EndEvent
