ScriptName Mumirnik_REF_Instincts_ClearHorse Extends ReferenceAlias
{ When this REF is no longer ridden, clear it. }

;-- Variables ---------------------------------------
Actor RiderREF

;-- Properties --------------------------------------
Float Property StayMountedUpdateTimer Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function Ride(Actor akRider)
  RiderREF = akRider ; #DEBUG_LINE_NO:9
  Self.RegisterForSingleUpdate(StayMountedUpdateTimer) ; #DEBUG_LINE_NO:10
EndFunction

Event OnUpdate()
  If !RiderREF.IsOnMount() ; #DEBUG_LINE_NO:14
    Self.StopRiding() ; #DEBUG_LINE_NO:15
  Else
    Self.RegisterForSingleUpdate(StayMountedUpdateTimer) ; #DEBUG_LINE_NO:17
  EndIf
EndEvent

Event OnActivate(ObjectReference akActivator)
  If akActivator == RiderREF as ObjectReference ; #DEBUG_LINE_NO:22
    RiderREF.Dismount() ; #DEBUG_LINE_NO:23
  EndIf
EndEvent

Function StopRiding()
  Actor selfREF = Self.GetActorReference() ; #DEBUG_LINE_NO:28
  selfREF.BlockActivation(False) ; #DEBUG_LINE_NO:29
  Self.Clear() ; #DEBUG_LINE_NO:30
  selfREF.EvaluatePackage() ; #DEBUG_LINE_NO:31
EndFunction
