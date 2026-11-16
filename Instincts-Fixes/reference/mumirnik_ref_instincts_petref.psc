ScriptName Mumirnik_REF_Instincts_PetRef Extends ReferenceAlias
{ Handles this reference receiving food, bleedout and housing. }

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Message Property BleedoutMessage Auto
Message Property BleedoutMessageHELP Auto
GlobalVariable Property CurrentPetIsEatingGlobal Auto
Message Property DeathMessage Auto
Actor Property PlayerREF Auto
Float Property SendToMarkedLocationUpdateTimer Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Event OnItemAdded(Form akBaseItem, Int aiItemCount, ObjectReference akItemReference, ObjectReference akSourceContainer)
  Bool isEating = CurrentPetIsEatingGlobal.GetValue() == 1 as Float ; #DEBUG_LINE_NO:12
  If ((akBaseItem as Ingredient) as Bool || (akBaseItem as Potion) as Bool) && isEating ; #DEBUG_LINE_NO:13
    (Self.GetOwningQuest() as mumirnik_quest_instincts_feeding).ReceiveFood(Self.GetActorReference(), akBaseItem, aiItemCount) ; #DEBUG_LINE_NO:14
  ElseIf ((akBaseItem as Armor) as Bool || (akBaseItem as Weapon) as Bool) && !isEating ; #DEBUG_LINE_NO:15
    Self.GetActorReference().UnequipAll() ; #DEBUG_LINE_NO:16
  EndIf
EndEvent

Event OnEnterBleedout()
  BleedoutMessage.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:21
  BleedoutMessageHELP.ShowAsHelpMessage("PetBleedout", 8.0, 1.0, 1) ; #DEBUG_LINE_NO:22
EndEvent

Event OnDying(Actor akKiller)
  DeathMessage.Show(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) ; #DEBUG_LINE_NO:26
  (Self.GetOwningQuest() as mumirnik_quest_instincts_petoptions).ReleasePet(Self.GetActorReference()) ; #DEBUG_LINE_NO:27
EndEvent

Function SendToMarkedLocation()
{ Sends this actor to the marked location once it leaves line of sight, as long as it continues to have WaitingForPlayer == 2 during this time. }
  Actor thisActor = Self.GetReference() as Actor ; #DEBUG_LINE_NO:32
  thisActor.EvaluatePackage() ; #DEBUG_LINE_NO:33
  Self.OnUpdate() ; #DEBUG_LINE_NO:34
EndFunction

Event OnUpdate()
  Actor thisActor = Self.GetReference() as Actor ; #DEBUG_LINE_NO:38
  If thisActor.GetActorValue("WaitingForPlayer") == 2 as Float ; #DEBUG_LINE_NO:39
    If PlayerREF.HasLos(thisActor as ObjectReference) ; #DEBUG_LINE_NO:40
      Self.RegisterForSingleUpdate(SendToMarkedLocationUpdateTimer) ; #DEBUG_LINE_NO:41
    Else
      thisActor.MoveToPackageLocation() ; #DEBUG_LINE_NO:43
    EndIf
  EndIf
EndEvent
