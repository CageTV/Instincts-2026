ScriptName Mumirnik_Quest_Instincts_HouseMarker Extends Quest

;-- Variables ---------------------------------------

;-- Properties --------------------------------------
Activator Property EmptyActivator Auto
ReferenceAlias Property HouseMarkerREF Auto
Actor Property PlayerREF Auto
Float Property WaitDuration Auto

;-- Functions ---------------------------------------

; Skipped compiler generated GetState

; Skipped compiler generated GotoState

Function MarkLocation(Actor akTarget)
{ Mark the spot as a house, flag the target as housed and send ALL housed pets to the house location. }
  ObjectReference houseMarker = HouseMarkerREF.GetReference() ; #DEBUG_LINE_NO:12
  If houseMarker ; #DEBUG_LINE_NO:13
    houseMarker.MoveTo(akTarget as ObjectReference, 0.0, 0.0, 0.0, True) ; #DEBUG_LINE_NO:14
  Else
    houseMarker = akTarget.PlaceAtMe(EmptyActivator as Form, 1, False, False) ; #DEBUG_LINE_NO:16
    HouseMarkerREF.ForceRefTo(houseMarker) ; #DEBUG_LINE_NO:17
  EndIf
  akTarget.SetActorValue("WaitingForPlayer", 2 as Float) ; #DEBUG_LINE_NO:19
  ReferenceAlias[] petREF = ((Self as Quest) as mumirnik_quest_instincts_petoptions).petREF ; #DEBUG_LINE_NO:20
  Int i = 0 ; #DEBUG_LINE_NO:21
  While i < petREF.Length ; #DEBUG_LINE_NO:22
    (petREF[i] as mumirnik_ref_instincts_petref).SendToMarkedLocation() ; #DEBUG_LINE_NO:23
    i += 1 ; #DEBUG_LINE_NO:24
  EndWhile
EndFunction

Function SendToMarkedLocation(Actor akTarget)
{ Flag the target as housed and send it to the house location. }
  akTarget.SetActorValue("WaitingForPlayer", 2 as Float) ; #DEBUG_LINE_NO:30
  ReferenceAlias targetREF = ((Self as Quest) as mumirnik_quest_instincts_petoptions).GetRefForActor(akTarget) ; #DEBUG_LINE_NO:31
  (targetREF as mumirnik_ref_instincts_petref).SendToMarkedLocation() ; #DEBUG_LINE_NO:32
EndFunction
