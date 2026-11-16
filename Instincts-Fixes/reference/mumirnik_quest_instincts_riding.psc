ScriptName Mumirnik_Quest_Instincts_Riding Extends Quest
{ Allows riding a tamed animal. }

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
ReferenceAlias Property MountREF Auto
Actor Property PlayerREF Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Ride(Actor akTarget)
{ Makes the player ride the target. }
  If akTarget.GetActorValue("WaitingForPlayer") == 2 as Float ; #DEBUG_LINE_NO:9
    akTarget.SetActorValue("WaitingForPlayer", 1 as Float) ; #DEBUG_LINE_NO:10
  EndIf
  MountREF.ForceRefTo(akTarget as ObjectReference) ; #DEBUG_LINE_NO:12
  Self.RegisterForSingleUpdate(0.02) ; #DEBUG_LINE_NO:13
EndFunction

Event OnUpdate()
  Actor targetREF = MountREF.GetReference() as Actor ; #DEBUG_LINE_NO:17
  If MountREF ; #DEBUG_LINE_NO:18
    targetREF.Activate(PlayerREF as ObjectReference, True) ; #DEBUG_LINE_NO:19
    targetREF.BlockActivation(True) ; #DEBUG_LINE_NO:20
    (MountREF as mumirnik_ref_instincts_clearhorse).Ride(PlayerREF) ; #DEBUG_LINE_NO:21
  EndIf
EndEvent
